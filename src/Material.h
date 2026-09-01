#pragma once

#include <wrl/client.h>
#include <d3d11.h>

#include "RenderPipeline.h"

#include <memory>
#include <cassert>

class Material final
{
public:
	explicit Material(std::shared_ptr<const RenderPipeline> Pipeline)
		: RenderPipeline(std::move(Pipeline))
	{
		assert(RenderPipeline);
	}

	const RenderPipeline& GetRendererPipeline() const { return *RenderPipeline; };

	void SetTextureSRV(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> TextureSRV) { this->TextureSRV = TextureSRV; };
	void SetSamplerState(Microsoft::WRL::ComPtr<ID3D11SamplerState> SamplerState) { this->SamplerState = SamplerState; };

private:
	friend class URenderer;

	void BindResources(ID3D11DeviceContext* Context) const;

	std::shared_ptr<const RenderPipeline> RenderPipeline = nullptr;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> TextureSRV;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> SamplerState;
};
