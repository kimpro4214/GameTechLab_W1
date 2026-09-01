#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include <memory>
#include <vector>

class Material;
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
	void Draw(
		URenderer& Renderer,
		const std::vector<std::unique_ptr<UBall>>& Balls) const;

private:
	std::unique_ptr<Material> FruitMaterial;
	std::shared_ptr<Mesh> FruitMesh;
	Microsoft::WRL::ComPtr<ID3D11Buffer> FruitConstantBuffer;
};
