#include "pch.h"

#include "BackgroundRenderer.h"

#include "Mesh.h"
#include "SpriteVertex.h"
#include "URenderer.h"
#include "Game/GameConfig.h"

namespace
{
	struct ObjectConstants
	{
		float ResolutionX;
		float ResolutionY;
		float Time;
		float GuideVisible;

		float GuideX;
		float GuideStartY;
		float GuideEndY;
		float GameOverY;

		float WorldToClipYScale;
		float WorldToClipYOffset;
		float Padding1;
		float Padding2;
	};
	static_assert(sizeof(ObjectConstants) % 16 == 0);
}

BackgroundRenderer::BackgroundRenderer() = default;
BackgroundRenderer::~BackgroundRenderer() = default;

bool BackgroundRenderer::Initialize(URenderer& Renderer)
{
	RenderPipelineDesc PipelineDesc{};
	PipelineDesc.ShaderFileName = L"shaders/BackgroundShader.hlsl";
	PipelineDesc.VertexEntryPoint = "mainVS";
	PipelineDesc.PixelEntryPoint = "mainPS";
	PipelineDesc.InputElements = SpriteInputLayout;
	PipelineDesc.InputElementCount = SpriteInputElementCount;

	D3D11_RENDER_TARGET_BLEND_DESC& Target = PipelineDesc.BlendDesc.RenderTarget[0];
	Target.BlendEnable = FALSE;
	Target.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	const std::shared_ptr<RenderPipeline> Pipeline =
		Renderer.CreateRenderPipeline(PipelineDesc);
	if (!Pipeline)
	{
		Release();
		return false;
	}

	BackgroundMaterial.emplace(Pipeline);

	MeshDesc MeshDescription{};
	MeshDescription.VertexData = SpriteQuadVertices;
	MeshDescription.VertexDataSize = sizeof(SpriteQuadVertices);
	MeshDescription.VertexStride = sizeof(SpriteVertex);
	MeshDescription.VertexCount = sizeof(SpriteQuadVertices) / sizeof(SpriteVertex);
	MeshDescription.IndexData = SpriteQuadIndices;
	MeshDescription.IndexDataSize = sizeof(SpriteQuadIndices);
	MeshDescription.IndexCount = sizeof(SpriteQuadIndices) / sizeof(std::uint32_t);
	BackgroundMesh = Renderer.CreateMesh(MeshDescription);
	ConstantBuffer =
		Renderer.CreateDynamicConstantBuffer(sizeof(ObjectConstants));

	if (BackgroundMesh == nullptr || ConstantBuffer == nullptr)
	{
		Release();
		return false;
	}

	return true;
}

void BackgroundRenderer::Release()
{
	BackgroundMaterial.reset();
	BackgroundMesh.reset();
	ConstantBuffer.Reset();
	ElapsedTime = 0.0f;
}

void BackgroundRenderer::Draw(URenderer& Renderer, float DeltaTime, const FDropGuideState& GuideState)
{
	if (!BackgroundMaterial.has_value() || !BackgroundMesh || !ConstantBuffer)
	{
		return;
	}

	ElapsedTime += DeltaTime;

	const ObjectConstants Constants{
		Renderer.ViewportInfo.Width,
		Renderer.ViewportInfo.Height,
		ElapsedTime,
		GuideState.bVisible ? 1.0f : 0.0f,

		GuideState.X,
		GuideState.StartY,
		GuideState.EndY,
		GameConfig::GameOverLineY,

		GameConfig::WorldToClipYScale,
		GameConfig::WorldToClipYOffset
	};

	Renderer.UpdateDynamicConstantBuffer(ConstantBuffer, Constants);

	Renderer.Draw(BackgroundMaterial.value(), *BackgroundMesh, ConstantBuffer.Get());
}
