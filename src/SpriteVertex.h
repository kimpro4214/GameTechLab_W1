#pragma once

#include <cstdint>

struct SpriteVertex
{
	float x, y; // POSITION
	float u, v; // TEXCOORD0
};

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
