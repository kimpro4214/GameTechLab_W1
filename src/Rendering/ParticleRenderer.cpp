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
	if (!CreateDropletMaterial(Renderer))
	{
		Release();
		return false;
	}

	if (!CreateSplashMaterial(Renderer))
	{
		Release();
		return false;
	}

	if (!CreateFlashMaterial(Renderer))
	{
		Release();
		return false;
	}

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
	DropletMaterial.reset();
	SplashMaterial.reset();
	FlashMaterial.reset();
	ParticleMesh.reset();
	ConstantBuffer.Reset();
}

void ParticleRenderer::Draw(URenderer& Renderer, const std::vector<FMergeParticle>& Particles) const
{
	if (!DropletMaterial.has_value() ||
		!SplashMaterial.has_value() ||
		!FlashMaterial.has_value() ||
		!ParticleMesh ||
		!ConstantBuffer)
	{
		return;
	}

	for (const auto& Particle : Particles)
	{
		const float Progress = Particle.Age / Particle.Lifetime;
		const float ScaleProgress = 1.0f - (1.0f - Progress) * (1.0f - Progress);
		const float Alpha = (1.0f - Progress) * (1.0f - Progress);

		const ObjectConstants Constants{
			Particle.Position.x,
			Particle.Position.y,
			std::lerp(Particle.StartScaleX, Particle.EndScaleX, ScaleProgress),
			std::lerp(Particle.StartScaleY, Particle.EndScaleY, ScaleProgress),
			Particle.Rotation,
			Particle.Color,
			Alpha,
		};

		Renderer.UpdateDynamicConstantBuffer(ConstantBuffer, Constants);

		switch (Particle.Type)
		{
			case EMergeParticleType::Droplet:
				Renderer.Draw(DropletMaterial.value(), *ParticleMesh, ConstantBuffer.Get());
				break;
			case EMergeParticleType::Splash:
				Renderer.Draw(SplashMaterial.value(), *ParticleMesh, ConstantBuffer.Get());
				break;
			case EMergeParticleType::Flash:
				Renderer.Draw(FlashMaterial.value(), *ParticleMesh, ConstantBuffer.Get());
				break;
		}
	}
}

bool ParticleRenderer::CreateDropletMaterial(URenderer& Renderer)
{
	RenderPipelineDesc PipelineDesc{};
	PipelineDesc.ShaderFileName = L"shaders/DropletShader.hlsl";
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

	DropletMaterial.emplace(Pipeline);

	return true;
}

bool ParticleRenderer::CreateSplashMaterial(URenderer& Renderer)
{
	RenderPipelineDesc PipelineDesc{};
	PipelineDesc.ShaderFileName = L"shaders/SplashShader.hlsl";
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

	SplashMaterial.emplace(Pipeline);
	return true;
}

bool ParticleRenderer::CreateFlashMaterial(URenderer& Renderer)
{
	RenderPipelineDesc PipelineDesc{};
	PipelineDesc.ShaderFileName = L"shaders/FlashShader.hlsl";
	PipelineDesc.VertexEntryPoint = "mainVS";
	PipelineDesc.PixelEntryPoint = "mainPS";
	PipelineDesc.InputElements = SpriteInputLayout;
	PipelineDesc.InputElementCount = SpriteInputElementCount;

	D3D11_RENDER_TARGET_BLEND_DESC& Target = PipelineDesc.BlendDesc.RenderTarget[0];
	Target.BlendEnable = TRUE;
	Target.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	Target.DestBlend = D3D11_BLEND_ONE;
	Target.BlendOp = D3D11_BLEND_OP_ADD;
	Target.SrcBlendAlpha = D3D11_BLEND_ZERO;
	Target.DestBlendAlpha = D3D11_BLEND_ONE;
	Target.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	Target.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	const std::shared_ptr<RenderPipeline> Pipeline =
		Renderer.CreateRenderPipeline(PipelineDesc);
	if (!Pipeline)
	{
		return false;
	}

	FlashMaterial.emplace(Pipeline);
	return true;
}
