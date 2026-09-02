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
	RenderPipelineDesc FruitPipelineDesc{};
	FruitPipelineDesc.ShaderFileName = L"shaders/FruitShader.hlsl";
	FruitPipelineDesc.VertexEntryPoint = "mainVS";
	FruitPipelineDesc.PixelEntryPoint = "mainPS";
	FruitPipelineDesc.InputElements = SpriteInputLayout;
	FruitPipelineDesc.InputElementCount = SpriteInputElementCount;

	D3D11_RENDER_TARGET_BLEND_DESC& Target = FruitPipelineDesc.BlendDesc.RenderTarget[0];
	Target.BlendEnable = TRUE;
	Target.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	Target.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	Target.BlendOp = D3D11_BLEND_OP_ADD;
	Target.SrcBlendAlpha = D3D11_BLEND_ONE;
	Target.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	Target.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	Target.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	const std::shared_ptr<RenderPipeline> FruitPipeline =
		Renderer.CreateRenderPipeline(FruitPipelineDesc);
	if (!FruitPipeline)
	{
		return false;
	}

	RenderPipelineDesc GlowPipelineDesc{};
	GlowPipelineDesc.ShaderFileName = L"shaders/GlowShader.hlsl";
	GlowPipelineDesc.VertexEntryPoint = "mainVS";
	GlowPipelineDesc.PixelEntryPoint = "mainPS";
	GlowPipelineDesc.InputElements = SpriteInputLayout;
	GlowPipelineDesc.InputElementCount = SpriteInputElementCount;

	D3D11_RENDER_TARGET_BLEND_DESC& GlowTarget = GlowPipelineDesc.BlendDesc.RenderTarget[0];
	GlowTarget.BlendEnable = TRUE;
	GlowTarget.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	GlowTarget.DestBlend = D3D11_BLEND_ONE;
	GlowTarget.BlendOp = D3D11_BLEND_OP_ADD;
	GlowTarget.SrcBlendAlpha = D3D11_BLEND_ZERO;
	GlowTarget.DestBlendAlpha = D3D11_BLEND_ONE;
	GlowTarget.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	GlowTarget.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	const std::shared_ptr<RenderPipeline> GlowPipeline =
		Renderer.CreateRenderPipeline(GlowPipelineDesc);
	if (!GlowPipeline)
	{
		Release();
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

	FruitMaterial.emplace(FruitPipeline);
	FruitMaterial->SetTextureSRV(Texture);
	FruitMaterial->SetSamplerState(Sampler);
	GlowMaterial.emplace(GlowPipeline);

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
	GlowMaterial.reset();
	FruitMesh.reset();
	ConstantBuffer.Reset();
}

void FruitRenderer::Draw(
	URenderer& Renderer,
	const std::vector<std::unique_ptr<UBall>>& Balls) const
{
	if (!FruitMaterial.has_value() || !GlowMaterial.has_value() || !FruitMesh || !ConstantBuffer)
	{
		return;
	}

	// 모든 글로우를 먼저 그려 다른 젤리 본체 위로 번지지 않게 한다.
	for (const std::unique_ptr<UBall>& Ball : Balls)
	{
		if (!FruitCatalog::IsValidLevel(Ball->Level))
		{
			continue;
		}

		const ObjectConstants GlowConstants{
			Ball->Location.x,
			Ball->Location.y,
			Ball->Radius * 1.6f,
			0.0f,

			FruitCatalog::GetColor(Ball->Level),
			static_cast<float>(Ball->Level) / static_cast<float>(FruitCatalog::LevelCount - 1),
		};
		Renderer.UpdateDynamicConstantBuffer(ConstantBuffer, GlowConstants);
		Renderer.Draw(GlowMaterial.value(), *FruitMesh, ConstantBuffer.Get());
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
