// ShaderW0.hlsl 소스 파일 맨위에 아래 상수버퍼 선언을 추가 하세요.
cbuffer constants : register(b0)
{
    float3 Offset;
    float Scale;
    float RotationAngle;
    float3 FruitColor;
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
    
    float3 fruitColor = FruitColor;
    
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
