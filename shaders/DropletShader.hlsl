cbuffer constants : register(b0)
{
    float2 Offset;
    float2 Scale;
    
    float RotationAngle;
    float3 Color;
    
    float Alpha;
    float WorldToClipYScale;
    float WorldToClipYOffset;
    float Padding;
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
    
    float c = cos(RotationAngle);
    float s = sin(RotationAngle);
    
    float2 rotatedPosition = float2(
        input.position.x * c - input.position.y * s,
        input.position.x * s + input.position.y * c
    );

    float2 worldPosition = rotatedPosition * Scale + Offset;
    worldPosition.y = worldPosition.y * WorldToClipYScale + WorldToClipYOffset;
    output.position = float4(worldPosition, 0.0f, 1.0f);
    
    output.uv = input.uv;
    
    return output;
}

float4 mainPS(PS_INPUT input) : SV_Target
{
    float2 toCenter = input.uv - float2(0.5f, 0.5f);
    float2 stretched = float2(toCenter.x * 1.8f, toCenter.y);
    float distanceFromCenter = length(stretched);
    
    float highlight = 1.0f - smoothstep(0.0f, 0.18f, length(input.uv - float2(0.38f, 0.30f)));

    float3 color = Color + highlight * 0.1f;
    
    float alpha = 1.0f - smoothstep(0.38f, 0.5f, distanceFromCenter);
    
    return float4(saturate(color), Alpha * alpha);
}
