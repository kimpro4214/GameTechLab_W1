cbuffer constants : register(b0)
{
    float2 Offset;
    float2 Scale;

    float RotationAngle;
    float3 Color;

    float Alpha;
    float3 Padding;
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
    output.position = float4(input.position * Scale + Offset, 0.0f, 1.0f);
    output.uv = input.uv;
    return output;
}

float4 mainPS(PS_INPUT input) : SV_Target
{
    float2 p = input.uv * 2.0f - 1.0f;
    float radius = length(p);
    float edge = 1.0f - smoothstep(0.65f, 1.0f, radius);
    float core = 1.0f - smoothstep(0.0f, 0.65f, radius);
    float intensity = saturate(edge * 0.65f + core * 0.55f);

    return float4(Color, Alpha * intensity);
}
