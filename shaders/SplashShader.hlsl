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
    float2 p = input.uv * 2.0f - 1.0f;
    float radius = length(p);
    float angle = atan2(p.y, p.x);
	
    float wobble =
	sin(angle * 5.0f + RotationAngle * 3.1f) * 0.08f +
	sin(angle * 9.0f - RotationAngle * 1.7f) * 0.035f;
	
    float outerRadius = 0.72f + wobble;
    float edgeWidth = fwidth(radius) * 1.5f;
	
    float splash = 1.0f - smoothstep(outerRadius - edgeWidth, outerRadius + edgeWidth, radius);
	
	float progress = 1.0f - Alpha;
    float innerRadius = progress * 0.55f;
	
    float innerMask = smoothstep(innerRadius - edgeWidth, innerRadius + edgeWidth, radius);
    splash *= innerMask;
	
    float centerGlow = 1.0f - smoothstep(0.0f, outerRadius, radius);
    float alpha = splash * lerp(0.45f, 1.0f, centerGlow);
	
    float3 highlight = lerp(Color, float3(1.0f, 1.0f, 1.0f), 0.35f);
    float3 color = lerp(Color, highlight, centerGlow);
    
	return float4(color, alpha * Alpha);
}
