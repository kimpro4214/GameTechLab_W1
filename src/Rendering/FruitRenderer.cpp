#include "Rendering/FruitRenderer.h"

#include "FVertexSimple.h"
#include "Material.h"
#include "Mesh.h"
#include "Physics/UBall.h"
#include "RenderPipeline.h"
#include "Rendering/FruitVisuals.h"
#include "Sphere.h"
#include "URenderer.h"

namespace
{
	struct FruitObjectConstants
	{
		FVector Offset;
		float Scale;
		float RotationAngle;
		FVector FruitColor;
	};

	static_assert(sizeof(FruitObjectConstants) % 16 == 0);
}

FruitRenderer::FruitRenderer() = default;
FruitRenderer::~FruitRenderer() = default;

bool FruitRenderer::Initialize(URenderer& Renderer)
{
	D3D11_INPUT_ELEMENT_DESC InputLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
			D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12,
			D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	RenderPipelineDesc PipelineDesc{};
	PipelineDesc.ShaderFileName = L"shaders/FruitShader.hlsl";
	PipelineDesc.VertexEntryPoint = "mainVS";
	PipelineDesc.PixelEntryPoint = "mainPS";
	PipelineDesc.InputElements = InputLayout;
	PipelineDesc.InputElementCount = 2;
	const std::shared_ptr<RenderPipeline> Pipeline =
		Renderer.CreateRenderPipeline(PipelineDesc);
	if (!Pipeline)
	{
		return false;
	}
	FruitMaterial = std::make_unique<Material>(Pipeline);

	MeshDesc MeshDescription{};
	MeshDescription.VertexData = sphere_vertices;
	MeshDescription.VertexDataSize = sizeof(sphere_vertices);
	MeshDescription.VertexStride = sizeof(FVertexSimple);
	MeshDescription.VertexCount = sizeof(sphere_vertices) / sizeof(FVertexSimple);
	FruitMesh = Renderer.CreateMesh(MeshDescription);
	FruitConstantBuffer =
		Renderer.CreateDynamicConstantBuffer(sizeof(FruitObjectConstants));

	return FruitMesh != nullptr && FruitConstantBuffer != nullptr;
}

void FruitRenderer::Draw(
	URenderer& Renderer,
	const std::vector<std::unique_ptr<UBall>>& Balls) const
{
	if (!FruitMaterial || !FruitMesh || !FruitConstantBuffer)
	{
		return;
	}

	for (const std::unique_ptr<UBall>& Ball : Balls)
	{
		const FruitObjectConstants Constants{
			Ball->Location,
			Ball->Radius,
			Ball->RotationAngle,
			GetFruitColor(Ball->Radius)
		};
		Renderer.UpdateDynamicConstantBuffer(FruitConstantBuffer, Constants);
		Renderer.Draw(*FruitMaterial, *FruitMesh, FruitConstantBuffer.Get());
	}
}
