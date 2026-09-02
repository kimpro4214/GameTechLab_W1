// ShaderW0.hlsl 소스 파일 맨위에 아래 상수버퍼 선언을 추가 하세요.
cbuffer constants : register(b0)
{
    float2 Offset;
    float Scale;
    float RotationAngle;
}

Texture2D FruitTexture : register(t0);
SamplerState FruitSampler : register(s0);

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

    // The game world is 2.5 units tall in a 1024x1280 window.
    // Compressing world Y by 1024 / 1280 keeps circular sprites round.
    worldPosition.y *= 0.8f;
    output.position = float4(worldPosition, 0.0, 1.0);
    
    output.uv = input.uv;
    
    return output;
}

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    return FruitTexture.Sample(FruitSampler, input.uv);
}
