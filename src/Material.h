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

private:
	friend class URenderer;

	void BindResources(ID3D11DeviceContext* Context) const;

	std::shared_ptr<const RenderPipeline> RenderPipeline = nullptr;
};
