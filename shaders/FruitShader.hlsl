// ShaderW0.hlsl 소스 파일 맨위에 아래 상수버퍼 선언을 추가 하세요.
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

    worldPosition.y = worldPosition.y * WorldToClipYScale + WorldToClipYOffset;
    output.position = float4(worldPosition, 0.0, 1.0);
    
    output.uv = input.uv;
    
    return output;
}

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    float2 p = input.uv * 2.0f - 1.0f;
    float radius = length(p);
    float aa = fwidth(radius);
    float alpha = 1.0f - smoothstep(1.0f - aa, 1.0f + aa, radius);

    if (alpha <= 0.0f)
        discard;

    float z = sqrt(saturate(1.0f - dot(p, p)));
    float3 normal = normalize(float3(p.x, -p.y, z));
    
    float c = cos(RotationAngle);
    float s = sin(RotationAngle);
    normal.xy = float2(normal.x * c - normal.y * s, normal.x * s + normal.y * c);

    float3 lightDir = normalize(float3(-0.45f, 0.55f, 0.8f));
    float3 viewDir = float3(0.0f, 0.0f, 1.0f);

    float diffuse = saturate(dot(normal, lightDir));
    float transmission = exp(-z * 1.4f);
    
    float rarity = pow(saturate(LevelRatio), 1.6f);
    
    float3 color = Color * (0.32f + diffuse * 0.28f);
    color += Color * transmission * 0.35f;
    
    float detailTiling = clamp(Scale / 3.0f, 0.18f, 1.5f);
    float2 detailUV = p * 0.5f * detailTiling + 0.5f;
    float sample = Texture.Sample(Sampler, detailUV).r;
    float detail = clamp((sample - 0.34f) * 8.0f, -0.3f, 0.3f);
    color *= 1.0f + detail * 0.3f;

    float rim = pow(1.0f - z, 1.4f);
    float3 rimColor = lerp(Color, float3(0.65f, 0.9f, 1.0f), 0.65f);
    float rimIntensity = lerp(0.28f, 0.7f, rarity);
    color += rimColor * rim * rimIntensity;
    
    float specular = pow(saturate(dot(reflect(-lightDir, normal), viewDir)), 20.0f);
    float specularIntensity = lerp(0.25f, 0.75f, rarity);
    color += float3(0.8f, 0.95f, 1.0f) * specular * specularIntensity;

    float bodyOpacity = lerp(0.6f, 0.9f, z);
    
    color *= lerp(0.92f, 1.08f, rarity);

    return float4(saturate(color), alpha * bodyOpacity);
}
