#pragma once

#include <wrl/client.h>
#include <d3d11.h>

#include "Material.h"

#include <vector>
#include <memory>

class URenderer;
class Mesh;

struct FVector;

class FruitRenderer
{
public:
	explicit FruitRenderer(URenderer& InRenderer)
		: Renderer(InRenderer)
	{
	}

	void Initialize();
	void Release();

	void Draw(int Level, const FVector& Location, float PhysicsRadius, float RotationAngle);


private:
	URenderer& Renderer;

	std::vector<Material> Materials;
	std::shared_ptr<Mesh> FruitMesh;
	Microsoft::WRL::ComPtr<ID3D11Buffer> ObjectConstantBuffer;
};
