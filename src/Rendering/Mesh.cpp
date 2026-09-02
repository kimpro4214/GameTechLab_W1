#include "pch.h"
#include "Mesh.h"

#include "Windows.h"

void Mesh::Bind(ID3D11DeviceContext* context) const
{
	if (!context)
	{
		return;
	}

	constexpr UINT offset = 0;

	context->IASetVertexBuffers(0, 1, VertexBuffer.GetAddressOf(), &VertexStride, &offset);
	context->IASetIndexBuffer(IndexBuffer.Get(), IndexFormat, 0);

	context->IASetPrimitiveTopology(Topology);
}
