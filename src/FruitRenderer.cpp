#include "FruitRenderer.h"

#include "URenderer.h"
#include "RenderPipeline.h"
#include "Material.h"
#include "Mesh.h"
#include "SpriteVertex.h"
#include "FVector.h"

#include <array>

namespace
{
	constexpr D3D11_INPUT_ELEMENT_DESC FruitInputLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	constexpr std::array<LPCWSTR, 11> FruitTexturePaths =
	{
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

void FruitRenderer::Initialize()
{
	RenderPipelineDesc PipelineDesc{};
	PipelineDesc.ShaderFileName = L"shaders/FruitShader.hlsl";
	PipelineDesc.VertexEntryPoint = "mainVS";
	PipelineDesc.PixelEntryPoint = "mainPS";
	PipelineDesc.InputElements = FruitInputLayout;
	PipelineDesc.InputElementCount = ARRAYSIZE(FruitInputLayout);

	auto& BlendDesc = PipelineDesc.BlendDesc;
	BlendDesc.AlphaToCoverageEnable = FALSE;
	BlendDesc.IndependentBlendEnable = FALSE;

	auto& Target = BlendDesc.RenderTarget[0];
	Target.BlendEnable = TRUE;
	Target.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	Target.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	Target.BlendOp = D3D11_BLEND_OP_ADD;
	Target.SrcBlendAlpha = D3D11_BLEND_ONE;
	Target.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	Target.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	Target.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	auto FruitPipeline = Renderer.CreateRenderPipeline(PipelineDesc);

	D3D11_SAMPLER_DESC SamplerDesc{};
	SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	auto Sampler = Renderer.CreateSamplerState(SamplerDesc);

	Materials.reserve(11);
	for (const auto TexturePath : FruitTexturePaths)
	{
		Materials.emplace_back(FruitPipeline);
		Materials.back().SetTextureSRV(Renderer.LoadTexture(TexturePath));
		Materials.back().SetSamplerState(Sampler);
	}

	MeshDesc MeshDesc{};
	MeshDesc.VertexData = SpriteQuadVertices;
	MeshDesc.VertexDataSize = sizeof(SpriteQuadVertices);
	MeshDesc.VertexStride = sizeof(SpriteVertex);
	MeshDesc.VertexCount = sizeof(SpriteQuadVertices) / sizeof(SpriteVertex);
	MeshDesc.IndexData = SpriteQuadIndices;
	MeshDesc.IndexDataSize = sizeof(SpriteQuadIndices);
	MeshDesc.IndexCount = sizeof(SpriteQuadIndices) / sizeof(std::uint32_t);

	FruitMesh = Renderer.CreateMesh(MeshDesc);

	ObjectConstantBuffer = Renderer.CreateDynamicConstantBuffer(sizeof(FruitObjectConstants) + 0xf & 0xfffffff0);
}

void FruitRenderer::Release()
{
	Materials.clear();
	FruitMesh.reset();
	ObjectConstantBuffer.Reset();
}

void FruitRenderer::Draw(int Level, const FVector& Location, float PhysicsRadius, float RotationAngle)
{
	FruitObjectConstants Constants{ Location.x, Location.y, PhysicsRadius / FruitSpriteRatio, RotationAngle };
	Renderer.UpdateDynamicConstantBuffer(ObjectConstantBuffer, Constants);
	Renderer.Draw(Materials[Level], *FruitMesh, ObjectConstantBuffer.Get());
}
