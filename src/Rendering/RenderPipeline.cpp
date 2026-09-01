#include "RenderPipeline.h"

#include <d3d11.h>

void RenderPipeline::Bind(ID3D11DeviceContext* Context) const
{
	if (!Context)
	{
		return;
	}

	Context->VSSetShader(VertexShader.Get(), nullptr, 0);
	Context->PSSetShader(PixelShader.Get(), nullptr, 0);
	Context->IASetInputLayout(InputLayout.Get());
	Context->RSSetState(RasterizerState.Get());
	Context->OMSetBlendState(BlendState.Get(), nullptr, 0xffffffff);
}
