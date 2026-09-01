#pragma once

#include <wrl/client.h>
#include <d3d11.h>

struct ID3D11DeviceContext;
class Mesh;

class RenderPipeline final
{
public:
	void Bind(ID3D11DeviceContext* Context) const;

private:
	friend class URenderer;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> VertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> PixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> InputLayout;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> RasterizerState;
	Microsoft::WRL::ComPtr<ID3D11BlendState> BlendState;
};

struct RenderPipelineDesc
{
	LPCWSTR ShaderFileName;
	LPCSTR VertexEntryPoint;
	LPCSTR PixelEntryPoint;

	const D3D11_INPUT_ELEMENT_DESC* InputElements = nullptr;
	UINT InputElementCount = 0;

	D3D11_BLEND_DESC BlendDesc{};
};
