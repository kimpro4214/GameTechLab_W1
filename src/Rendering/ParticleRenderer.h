#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include "Material.h"
#include "Mesh.h"

#include <memory>
#include <optional>
#include <vector>

struct FMergeParticle;
class URenderer;

class ParticleRenderer final
{
public:
	ParticleRenderer();
	~ParticleRenderer();

	ParticleRenderer(const ParticleRenderer&) = delete;
	ParticleRenderer& operator=(const ParticleRenderer&) = delete;

	bool Initialize(URenderer& Renderer);
	void Release();
	void Draw(URenderer& Renderer, const std::vector<FMergeParticle>& Particles) const;

private:
	bool CreateDropletMaterial(URenderer& Renderer);
	bool CreateSplashMaterial(URenderer& Renderer);

private:
	std::optional<Material> DropletMaterial;
	std::optional<Material> SplashMaterial;
	std::shared_ptr<Mesh> ParticleMesh;
	Microsoft::WRL::ComPtr<ID3D11Buffer> ConstantBuffer;
};
