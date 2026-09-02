#pragma once

#include <d3d11.h>

#include <cstdint>

struct SpriteVertex
{
	float x, y; // POSITION
	float u, v; // TEXCOORD0
};

inline constexpr D3D11_INPUT_ELEMENT_DESC SpriteInputLayout[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0,
		D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8,
		D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

inline constexpr UINT SpriteInputElementCount = 2;

inline constexpr SpriteVertex SpriteQuadVertices[] =
{
	{ -1.0f,  1.0f,  0.0f,  0.0f },
	{  1.0f,  1.0f,  1.0f,  0.0f },
	{ -1.0f, -1.0f,  0.0f,  1.0f },
	{  1.0f, -1.0f,  1.0f,  1.0f },
};

inline constexpr std::uint32_t SpriteQuadIndices[] =
{
	0, 1, 2,
	2, 1, 3,
};
