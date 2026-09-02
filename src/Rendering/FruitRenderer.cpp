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
	constexpr D3D11_INPUT_ELEMENT_DESC FruitInputLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0,
			D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8,
			D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	constexpr std::array<LPCWSTR, FruitCatalog::LevelCount> FruitTexturePaths = {
		L"assets/fruits/fruit_00.png",
		L"assets/fruits/fruit_01.png",
		L"assets/fruits/fruit_02.png",
		L"assets/fruits/fruit_03.png",
		L"assets/fruits/fruit_04.png",
		L"assets/fruits/fruit_05.png",
		L"assets/fruits/fruit_06.png",
		L"assets/fruits/fruit_07.png",
		L"assets/fruits/fruit_08.png",
		L"assets/fruits/fruit_09.png",
		L"assets/fruits/fruit_10.png",
	};

	struct FruitObjectConstants
	{
		float OffsetX;
		float OffsetY;
		float Scale;
		float RotationAngle;
	};

	constexpr float FruitSpriteRatio = 736.0f / 1024.0f;
}

FruitRenderer::FruitRenderer() = default;
FruitRenderer::~FruitRenderer() = default;

bool FruitRenderer::Initialize(URenderer& Renderer)
{
	RenderPipelineDesc PipelineDesc{};
	PipelineDesc.ShaderFileName = L"shaders/FruitShader.hlsl";
	PipelineDesc.VertexEntryPoint = "mainVS";
	PipelineDesc.PixelEntryPoint = "mainPS";
	PipelineDesc.InputElements = FruitInputLayout;
	PipelineDesc.InputElementCount = ARRAYSIZE(FruitInputLayout);

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
	SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	const Microsoft::WRL::ComPtr<ID3D11SamplerState> Sampler =
		Renderer.CreateSamplerState(SamplerDesc);
	if (!Sampler)
	{
		return false;
	}

	FruitMaterials.clear();
	FruitMaterials.reserve(FruitTexturePaths.size());
	for (LPCWSTR TexturePath : FruitTexturePaths)
	{
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Texture =
			Renderer.LoadTexture(TexturePath);
		if (!Texture)
		{
			Release();
			return false;
		}

		FruitMaterials.emplace_back(Pipeline);
		FruitMaterials.back().SetTextureSRV(Texture);
		FruitMaterials.back().SetSamplerState(Sampler);
	}

	MeshDesc MeshDescription{};
	MeshDescription.VertexData = SpriteQuadVertices;
	MeshDescription.VertexDataSize = sizeof(SpriteQuadVertices);
	MeshDescription.VertexStride = sizeof(SpriteVertex);
	MeshDescription.VertexCount = sizeof(SpriteQuadVertices) / sizeof(SpriteVertex);
	MeshDescription.IndexData = SpriteQuadIndices;
	MeshDescription.IndexDataSize = sizeof(SpriteQuadIndices);
	MeshDescription.IndexCount = sizeof(SpriteQuadIndices) / sizeof(std::uint32_t);
	FruitMesh = Renderer.CreateMesh(MeshDescription);
	FruitConstantBuffer =
		Renderer.CreateDynamicConstantBuffer(sizeof(FruitObjectConstants));

	return FruitMesh != nullptr && FruitConstantBuffer != nullptr;
}

void FruitRenderer::Release()
{
	FruitMaterials.clear();
	FruitMesh.reset();
	FruitConstantBuffer.Reset();
}

void FruitRenderer::Draw(
	URenderer& Renderer,
	const std::vector<std::unique_ptr<UBall>>& Balls) const
{
	if (!FruitMesh || !FruitConstantBuffer ||
		FruitMaterials.size() != FruitCatalog::LevelCount)
	{
		return;
	}

	for (const std::unique_ptr<UBall>& Ball : Balls)
	{
		if (!FruitCatalog::IsValidLevel(Ball->Level))
		{
			continue;
		}

		const FruitObjectConstants Constants{
			Ball->Location.x,
			Ball->Location.y,
			Ball->Radius / FruitSpriteRatio,
			Ball->RotationAngle
		};
		Renderer.UpdateDynamicConstantBuffer(FruitConstantBuffer, Constants);
		Renderer.Draw(
			FruitMaterials[static_cast<std::size_t>(Ball->Level)],
			*FruitMesh,
			FruitConstantBuffer.Get());
	}
}

ID3D11ShaderResourceView* FruitRenderer::GetFruitTextureSRV(int Level) const
{
	if (!FruitCatalog::IsValidLevel(Level))
	{
		return nullptr;
	}

	return FruitMaterials[Level].GetTextureSRV();
}
