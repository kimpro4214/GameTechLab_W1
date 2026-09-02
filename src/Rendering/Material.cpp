#include "pch.h"
#include "Material.h"

#include <d3d11.h>

void Material::BindResources(ID3D11DeviceContext* Context) const
{
	RenderPipeline->Bind(Context);
	Context->PSSetShaderResources(0, 1, TextureSRV.GetAddressOf());
	Context->PSSetSamplers(0, 1, SamplerState.GetAddressOf());
}
