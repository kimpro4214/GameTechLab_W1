#pragma once

#include <Windows.h>
#include <wrl/client.h>
#include <d3d11.h>

class URenderer;

class Mesh final
{
public:
	void Bind(ID3D11DeviceContext* Context) const;
	bool HasIndices() const { return IndexBuffer.Get() != nullptr; };
	UINT GetVertexCount() const { return VertexCount; };
	UINT GetIndexCount() const { return IndexCount; };

private:
	friend class URenderer;

	Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> IndexBuffer;

	UINT VertexStride = 0;
	UINT VertexCount = 0;
	UINT IndexCount = 0;

	DXGI_FORMAT IndexFormat = DXGI_FORMAT_R32_UINT;
	D3D11_PRIMITIVE_TOPOLOGY Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};

struct MeshDesc
{
	const void* VertexData = nullptr;
	UINT VertexDataSize = 0;
	UINT VertexStride = 0;
	UINT VertexCount = 0;

	const void* IndexData = nullptr;
	UINT IndexDataSize = 0;
	UINT IndexCount = 0;

	DXGI_FORMAT IndexFormat = DXGI_FORMAT_R32_UINT;
	D3D11_PRIMITIVE_TOPOLOGY Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};
