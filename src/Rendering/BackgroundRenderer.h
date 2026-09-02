#pragma once

#include "pch.h"

#include "Material.h"

class Mesh;
class URenderer;

struct FDropGuideState
{
	float X = 0.0f;
	float StartY = 0.0f;
	float EndY = 0.0f;
	bool bVisible = false;
};

class BackgroundRenderer final
{
public:
	BackgroundRenderer();
	~BackgroundRenderer();

	BackgroundRenderer(const BackgroundRenderer&) = delete;
	BackgroundRenderer& operator=(const BackgroundRenderer&) = delete;

	bool Initialize(URenderer& Renderer);
	void Release();
	void Draw(URenderer& Renderer, float DeltaTime, const FDropGuideState& GuideState);

private:
	std::optional<Material> BackgroundMaterial;
	std::shared_ptr<Mesh> BackgroundMesh;
	Microsoft::WRL::ComPtr<ID3D11Buffer> ConstantBuffer;

	float ElapsedTime = 0.0f;
};
