cbuffer constants : register(b0)
{
    float2 Offset;
    float Scale;
    float RotationAngle;

    float3 Color;
    float LevelRatio;

    float WorldToClipYScale;
    float WorldToClipYOffset;
    float2 Padding;
}

struct VS_INPUT
{
    float2 position : POSITION;
    float2 uv : TEXCOORD0;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;
    float2 worldPosition = input.position * Scale + Offset;
    worldPosition.y = worldPosition.y * WorldToClipYScale + WorldToClipYOffset;
    output.position = float4(worldPosition, 0.0f, 1.0f);
    output.uv = input.uv;
    return output;
}

float4 mainPS(PS_INPUT input) : SV_Target
{
    float2 p = input.uv * 2.0f - 1.0f;
    float radius = length(p);
    float glow = 1.0f - smoothstep(0.55f, 1.0f, radius);

    float rarity = pow(saturate(LevelRatio), 1.6f);
    float intensity = lerp(0.35f, 0.75f, rarity);
    float3 glowColor = lerp(Color, float3(0.45f, 0.85f, 1.0f), 0.2f);

    return float4(glowColor, glow * intensity);
}
