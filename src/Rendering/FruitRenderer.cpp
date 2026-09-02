#include "pch.h"
#include "Rendering/FruitRenderer.h"

#include "Game/FruitCatalog.h"
#include "Mesh.h"
#include "Physics/UBall.h"
#include "RenderPipeline.h"
#include "SpriteVertex.h"
#include "URenderer.h"

#include <array>

namespace
{
	struct ObjectConstants
	{
		float OffsetX;
		float OffsetY;
		float Scale;
		float RotationAngle;

		FVector Color;
		float LevelRatio;
	};
	static_assert(sizeof(ObjectConstants) % 16 == 0);
}

FruitRenderer::FruitRenderer() = default;
FruitRenderer::~FruitRenderer() = default;

bool FruitRenderer::Initialize(URenderer& Renderer)
{
	RenderPipelineDesc PipelineDesc{};
	PipelineDesc.ShaderFileName = L"shaders/FruitShader.hlsl";
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

	D3D11_SAMPLER_DESC SamplerDesc{};
	SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	const Microsoft::WRL::ComPtr<ID3D11SamplerState> Sampler =
		Renderer.CreateSamplerState(SamplerDesc);
	if (!Sampler)
	{
		return false;
	}

	auto Texture = Renderer.LoadTexture(L"assets/jelly.png");
	if (!Texture)
	{
		Release();
		return false;
	}

	FruitMaterial.emplace(Pipeline);
	FruitMaterial->SetTextureSRV(Texture);
	FruitMaterial->SetSamplerState(Sampler);

	MeshDesc MeshDescription{};
	MeshDescription.VertexData = SpriteQuadVertices;
	MeshDescription.VertexDataSize = sizeof(SpriteQuadVertices);
	MeshDescription.VertexStride = sizeof(SpriteVertex);
	MeshDescription.VertexCount = sizeof(SpriteQuadVertices) / sizeof(SpriteVertex);
	MeshDescription.IndexData = SpriteQuadIndices;
	MeshDescription.IndexDataSize = sizeof(SpriteQuadIndices);
	MeshDescription.IndexCount = sizeof(SpriteQuadIndices) / sizeof(std::uint32_t);
	FruitMesh = Renderer.CreateMesh(MeshDescription);
	ConstantBuffer =
		Renderer.CreateDynamicConstantBuffer(sizeof(ObjectConstants));

	return FruitMesh != nullptr && ConstantBuffer != nullptr;
}

void FruitRenderer::Release()
{
	FruitMaterial.reset();
	FruitMesh.reset();
	ConstantBuffer.Reset();
}

void FruitRenderer::Draw(
	URenderer& Renderer,
	const std::vector<std::unique_ptr<UBall>>& Balls) const
{
	if (!FruitMaterial.has_value() || !FruitMesh || !ConstantBuffer)
	{
		return;
	}

	for (const std::unique_ptr<UBall>& Ball : Balls)
	{
		if (!FruitCatalog::IsValidLevel(Ball->Level))
		{
			continue;
		}

		const ObjectConstants Constants{
			Ball->Location.x,
			Ball->Location.y,
			Ball->Radius,
			Ball->RotationAngle,

			FruitCatalog::GetColor(Ball->Level),
			static_cast<float>(Ball->Level) / static_cast<float>(FruitCatalog::LevelCount - 1),
		};
		Renderer.UpdateDynamicConstantBuffer(ConstantBuffer, Constants);
		Renderer.Draw(
			FruitMaterial.value(),
			*FruitMesh,
			ConstantBuffer.Get());
	}
}
