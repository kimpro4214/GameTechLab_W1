cbuffer BackgroundConstants : register(b0)
{
    float2 Resolution;
    float Time;
    float Padding;
};

static const float BoardRatio = 0.75f;

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
    
    output.position = float4(input.position, 0.0f, 1.0f);
    output.uv = input.uv;
    
    return output;
}

float4 mainPS(PS_INPUT input) : SV_Target
{
    float2 uv = input.uv;

    float3 topColor = float3(0.094f, 0.141f, 0.227f);
    float3 bottomColor = float3(0.035f, 0.063f, 0.118f);
    float3 color = lerp(topColor, bottomColor, uv.y);

    float2 glowCenterA =
        float2(0.22f, 0.35f) +
        float2(
            sin(Time * 0.18f) * 0.07f,
            cos(Time * 0.14f) * 0.05f);

    float glowA = 1.0f - smoothstep(0.0f, 0.55f, distance(uv, glowCenterA));

    float2 glowCenterB =
        float2(0.62f, 0.75f) +
        float2(
            cos(Time * 0.11f) * 0.06f,
            sin(Time * 0.16f) * 0.04f);

    float glowB = 1.0f - smoothstep(0.0f, 0.45f, distance(uv, glowCenterB));

    color += float3(0.025f, 0.055f, 0.085f) * glowA;
    color += float3(0.035f, 0.025f, 0.070f) * glowB;

    float boardMask = 1.0f - step(BoardRatio, uv.x);
    color += float3(0.025f, 0.045f, 0.065f) * boardMask;

    float2 centered = uv * 2.0f - 1.0f;
    float vignette = smoothstep(0.45f, 1.25f, dot(centered, centered));

    color *= 1.0f - vignette * 0.25f;

    return float4(color, 1.0f);
}
