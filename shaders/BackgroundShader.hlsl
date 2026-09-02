cbuffer BackgroundConstants : register(b0)
{
    float2 Resolution;
    float Time;
    float GuideVisible;

    float GuideX;
    float GuideStartY;
    float GuideEndY;
    float GameOverY;

    float WorldToClipYScale;
    float WorldToClipYOffset;
    float2 Padding;
};

static const float BoardRatio = 0.75f;
static const float GameAreaLeft = -1.0f;
static const float GameAreaRight = 0.5f;

float WrappedPulse(float Phase, float Width)
{
    float wrappedDistance = abs(frac(Phase + 0.5f) - 0.5f);
    return exp(-(wrappedDistance * wrappedDistance) / (Width * Width));
}

float3 CalculateGameOverBarrier(float2 WorldPosition, float DistancePx)
{
    float3 glowColor = float3(0.75f, 0.05f, 1.0f);
    float3 coreColor = float3(1.0f, 0.18f, 0.65f);
    float3 pulseColor = float3(0.85f, 0.92f, 1.0f);
    
    float lineRange =
        step(GameAreaLeft, WorldPosition.x) *
        step(WorldPosition.x, GameAreaRight);

    float lineU = saturate(
        (WorldPosition.x - GameAreaLeft) /
        (GameAreaRight - GameAreaLeft));
    
    float travelPhase = lineU - Time * 0.22f;
    float pulseEnvelope = WrappedPulse(travelPhase, 0.10f);
    pulseEnvelope += WrappedPulse(travelPhase + 0.5f, 0.065f) * 0.55f;
    
    float combinedWave =
        0.50f +
        sin(lineU * 42.0f - Time * 7.0f) * 0.24f +
        sin(lineU * 67.0f - Time * 10.5f + 1.7f) * 0.17f +
        sin(lineU * 23.0f + Time * 4.0f + 0.8f) * 0.09f;
    combinedWave = saturate(combinedWave);

    float pulseEnergy = pulseEnvelope * lerp(0.55f, 1.25f, combinedWave);
    float idlePulse = 0.82f + sin(Time * 2.2f) * 0.18f;

    float core = (1.0f - smoothstep(0.0f, 1.5f, DistancePx)) * lineRange;
    float nearGlow = (1.0f - smoothstep(0.0f, 5.0f, DistancePx)) * lineRange;
    float wideGlow = (1.0f - smoothstep(0.0f, 14.0f, DistancePx)) * lineRange;

    float3 barrier = glowColor * wideGlow * (0.08f + pulseEnergy * 0.18f);
    barrier += coreColor * core * (0.38f * idlePulse);
    barrier += pulseColor * nearGlow * pulseEnergy * 0.72f;
    barrier += pulseColor * core * pulseEnergy * 0.28f;
    return barrier;
}

float3 CalculateGuide(float2 WorldPosition, float DistancePx)
{
    float3 guideColor = float3(0.10f, 0.88f, 1.0f);
    float3 guideCoreColor = float3(0.80f, 1.0f, 1.0f);
    
    float guideMinY = min(GuideStartY, GuideEndY);
    float guideMaxY = max(GuideStartY, GuideEndY);

    float guideRange =
        step(guideMinY, WorldPosition.y) *
        step(WorldPosition.y, guideMaxY) *
        GuideVisible;
    
    float guideLength = max(guideMaxY - guideMinY, 0.0001f);
    float guideT = saturate((WorldPosition.y - guideMinY) / guideLength);
    
    float endpointFade = smoothstep(0.0f, 0.08f, guideT) * smoothstep(0.0f, 0.12f, 1.0f - guideT);

    float core = (1.0f - smoothstep(0.0f, 1.2f, DistancePx)) * guideRange * endpointFade;
    float glow = (1.0f - smoothstep(0.0f, 8.0f, DistancePx)) * guideRange * endpointFade;
    
    float waveA = sin(guideT * 28.0f - Time * 3.2f);
    float waveB = sin(guideT * 47.0f - Time * 5.1f + 1.4f);
    
    float caustic = saturate(0.72f + waveA * 0.17f + waveB * 0.11f);
    
    float3 guide = guideColor * glow * 0.12f;
    guide += guideColor * core * 0.28f;
    guide += guideCoreColor * core * caustic * 0.55f;
    
    return guide;
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
    
    output.position = float4(input.position, 0.0f, 1.0f);
    output.uv = input.uv;
    
    return output;
}

float4 mainPS(PS_INPUT input) : SV_Target
{
    float2 uv = input.uv;
    float2 clipPosition = float2(input.uv.x * 2.0f - 1.0f, 1.0f - input.uv.y * 2.0f);
    float2 worldPosition = float2(
        clipPosition.x,
        (clipPosition.y - WorldToClipYOffset) / WorldToClipYScale);
    float gameOverDistancePx =
        abs(worldPosition.y - GameOverY) * WorldToClipYScale * Resolution.y * 0.5f;
    float guideDistancePx = abs(worldPosition.x - GuideX) * Resolution.x * 0.5f;

    float3 topColor = float3(0.094f, 0.141f, 0.227f);
    float3 bottomColor = float3(0.035f, 0.063f, 0.118f);
    float3 color = lerp(topColor, bottomColor, uv.y);

    float2 glowCenterA =
        float2(0.22f, 0.35f) +
        float2(sin(Time * 0.18f) * 0.07f, cos(Time * 0.14f) * 0.05f);

    float glowA = 1.0f - smoothstep(0.0f, 0.55f, distance(uv, glowCenterA));

    float2 glowCenterB =
        float2(0.62f, 0.75f) +
        float2(cos(Time * 0.11f) * 0.06f, sin(Time * 0.16f) * 0.04f);

    float glowB = 1.0f - smoothstep(0.0f, 0.45f, distance(uv, glowCenterB));

    color += float3(0.025f, 0.055f, 0.085f) * glowA;
    color += float3(0.035f, 0.025f, 0.070f) * glowB;

    float boardMask = 1.0f - step(BoardRatio, uv.x);
    color += float3(0.025f, 0.045f, 0.065f) * boardMask;

    float2 centered = uv * 2.0f - 1.0f;
    float vignette = smoothstep(0.45f, 1.25f, dot(centered, centered));

    color *= 1.0f - vignette * 0.25f;

    color += CalculateGameOverBarrier(worldPosition, gameOverDistancePx);
    
    color += CalculateGuide(worldPosition, guideDistancePx);

    return float4(color, 1.0f);
}
