#include "pch.h"
#include "ParticleRenderer.h"

#include "Particle/FMergeParticle.h"
#include "RenderPipeline.h"
#include "SpriteVertex.h"
#include "URenderer.h"

namespace
{
	struct ObjectConstants
	{
		float OffsetX;
		float OffsetY;
		float ScaleX;
		float ScaleY;
		float RotationAngle;
		
		FVector Color;
		float Alpha;

		float Padding1;
		float Padding2;
		float Padding3;
	};
	static_assert(sizeof(ObjectConstants) % 16 == 0);
}

ParticleRenderer::ParticleRenderer() = default;
ParticleRenderer::~ParticleRenderer() = default;

bool ParticleRenderer::Initialize(URenderer& Renderer)
{
	RenderPipelineDesc PipelineDesc{};
	PipelineDesc.ShaderFileName = L"shaders/ParticleShader.hlsl";
	PipelineDesc.VertexEntryPoint = "mainVS";
	PipelineDesc.PixelEntryPoint = "mainPS";
	PipelineDesc.InputElements = SpriteInputLayout;
	PipelineDesc.InputElementCount = SpriteInputElementCount;

	D3D11_RENDER_TARGET_BLEND_DESC& Target = PipelineDesc.BlendDesc.RenderTarget[0];
	Target.BlendEnable = TRUE;
	Target.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	Target.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	Target.BlendOp = D3D11_BLEND_OP_ADD;
	Target.SrcBlendAlpha = D3D11_BLEND_ONE;
	Target.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	Target.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	Target.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	const std::shared_ptr<RenderPipeline> Pipeline =
		Renderer.CreateRenderPipeline(PipelineDesc);
	if (!Pipeline)
	{
		return false;
	}

	ParticleMaterial.emplace(Pipeline);

	MeshDesc MeshDescription{};
	MeshDescription.VertexData = SpriteQuadVertices;
	MeshDescription.VertexDataSize = sizeof(SpriteQuadVertices);
	MeshDescription.VertexStride = sizeof(SpriteVertex);
	MeshDescription.VertexCount = sizeof(SpriteQuadVertices) / sizeof(SpriteVertex);
	MeshDescription.IndexData = SpriteQuadIndices;
	MeshDescription.IndexDataSize = sizeof(SpriteQuadIndices);
	MeshDescription.IndexCount = sizeof(SpriteQuadIndices) / sizeof(std::uint32_t);
	ParticleMesh = Renderer.CreateMesh(MeshDescription);
	ConstantBuffer =
		Renderer.CreateDynamicConstantBuffer(sizeof(ObjectConstants));

	if (!ParticleMesh || !ConstantBuffer)
	{
		Release();
		return false;
	}

	return true;
}

void ParticleRenderer::Release()
{
	ParticleMaterial.reset();
	ParticleMesh.reset();
	ConstantBuffer.Reset();
}

void ParticleRenderer::Draw(URenderer& Renderer, const std::vector<FMergeParticle>& Particles) const
{
	if (!ParticleMaterial.has_value() || !ParticleMesh || !ConstantBuffer)
	{
		return;
	}

	for (const auto& Particle : Particles)
	{
		const ObjectConstants Constants{
			Particle.Position.x,
			Particle.Position.y,
			Particle.ScaleX,
			Particle.ScaleY,
			Particle.Rotation,
			Particle.Color,
			1.0f - Particle.Age / Particle.Lifetime,
		};

		Renderer.UpdateDynamicConstantBuffer(ConstantBuffer, Constants);
		Renderer.Draw(ParticleMaterial.value(), *ParticleMesh, ConstantBuffer.Get());
	}
}
