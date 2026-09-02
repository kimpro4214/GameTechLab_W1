cbuffer constants : register(b0)
{
	float2 Offset;
	float2 Scale;
    
	float RotationAngle;
	float3 Color;
    
	float Alpha;
	float3 Padding;
}

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

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
	worldPosition.y *= 0.8f;
	output.position = float4(worldPosition, 0.0f, 1.0f);
    
	output.uv = input.uv;
    
	return output;
}

float4 mainPS(PS_INPUT input) : SV_Target
{
	float4 textureColor = Texture.Sample(Sampler, input.uv);
    
	return float4(textureColor.rgb * Color, textureColor.a * Alpha);
}
