#pragma once

#include "pch.h"

#include "Material.h"

class Mesh;
class URenderer;

class BackgroundRenderer final
{
public:
	BackgroundRenderer();
	~BackgroundRenderer();

	BackgroundRenderer(const BackgroundRenderer&) = delete;
	BackgroundRenderer& operator=(const BackgroundRenderer&) = delete;

	bool Initialize(URenderer& Renderer);
	void Release();
	void Draw(URenderer& Renderer, float DeltaTime);

private:
	std::optional<Material> BackgroundMaterial;
	std::shared_ptr<Mesh> BackgroundMesh;
	Microsoft::WRL::ComPtr<ID3D11Buffer> ConstantBuffer;

	float ElapsedTime = 0.0f;
};
