#pragma once

#include "pch.h"

#include "Material.h"

class Mesh;
class UBall;
class URenderer;

class FruitRenderer
{
public:
	FruitRenderer();
	~FruitRenderer();

	FruitRenderer(const FruitRenderer&) = delete;
	FruitRenderer& operator=(const FruitRenderer&) = delete;

	bool Initialize(URenderer& Renderer);
	void Release();
	void Draw(
		URenderer& Renderer,
		const std::vector<std::unique_ptr<UBall>>& Balls) const;

private:
	std::optional <Material> FruitMaterial;
	std::shared_ptr<Mesh> FruitMesh;
	Microsoft::WRL::ComPtr<ID3D11Buffer> ConstantBuffer;
};
