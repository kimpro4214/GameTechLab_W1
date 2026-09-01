// ShaderW0.hlsl 소스 파일 맨위에 아래 상수버퍼 선언을 추가 하세요.
cbuffer constants : register(b0)
{
    float3 Offset;
    float Scale;
    float RotationAngle;
    float3 Padding;
}

struct VS_INPUT
{
    float4 position : POSITION; // Input position from vertex buffer
    float4 color : COLOR; // Input color from vertex buffer
};

struct PS_INPUT
{
    float4 position : SV_POSITION; // Transformed position to pass to the pixel shader
    float4 color : COLOR; // Color to pass to the pixel shader
};

static const float ScaleStandard = 16.5 / 0.05;

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;
    
    float3 localPosition = input.position.xyz;
    float c = cos(RotationAngle);
    float s = sin(RotationAngle);
    float2 rotatedPosition;
    rotatedPosition.x = localPosition.x * c - localPosition.y * s;
    rotatedPosition.y = localPosition.x * s + localPosition.y * c;

    float3 worldPosition = float3(rotatedPosition, localPosition.z) * Scale + Offset;
    output.position = float4(worldPosition, input.position.w);
    
    float StandardizedScale = Scale * ScaleStandard;
    
    float3 fruitColor = input.color.xyz;
    
    if (StandardizedScale <= 16.5)
    {
        fruitColor = float3(0.95, 0.05, 0.05);
    }
    else if (StandardizedScale <= 24.0)
    {
        fruitColor = float3(0.99, 0.41, 0.3);
    }
    else if (StandardizedScale <= 30.5)
    {
        fruitColor = float3(0.63, 0.42, 1.0);
    }
    else if (StandardizedScale <= 36.5) // 34.5
    {
        fruitColor = float3(1.0, 0.72, 0.0);
    }
    else if (StandardizedScale <= 44.5)
    {
        fruitColor = float3(0.99, 0.55, 0.17);
    }
    else if (StandardizedScale <= 57.0)
    {
        fruitColor = float3(0.95, 0.05, 0.05);
    }
    else if (StandardizedScale <= 64.5)
    {
        fruitColor = float3(0.98, 0.94, 0.62);
    }
    else if (StandardizedScale <= 78.0)
    {
        fruitColor = float3(1.0, 0.71, 0.68);
    }
    else if (StandardizedScale <= 88.5)
    {
        fruitColor = float3(0.97, 0.92, 0.04);
    }
    else if (StandardizedScale <= 110.0)
    {
        fruitColor = float3(0.62, 0.87, 0.07);
    }
    else if (StandardizedScale <= 129.5)
    {
        fruitColor = float3(0.08, 0.61, 0.04);
    }
    
    // [-1, 1] -> [0, 1]
    float tint = (localPosition.y + 1.0) * 0.5;
    // 그라데이션
    fruitColor = lerp(float3(1.0, 1.0, 1.0), fruitColor, tint + 0.2);
    
    output.color = float4(fruitColor, input.color.w);
    
    return output;
}

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    // Output the color directly
    return input.color;
}
