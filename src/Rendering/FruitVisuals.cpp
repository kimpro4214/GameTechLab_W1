#include "Rendering/FruitVisuals.h"

FVector GetFruitColor(float Radius)
{
	constexpr float ScaleStandard = 16.5f / 0.05f;
	const float StandardizedScale = Radius * ScaleStandard;

	if (StandardizedScale <= 16.5f) return FVector(0.95f, 0.05f, 0.05f);
	if (StandardizedScale <= 24.0f) return FVector(0.99f, 0.41f, 0.30f);
	if (StandardizedScale <= 30.5f) return FVector(0.63f, 0.42f, 1.00f);
	if (StandardizedScale <= 36.5f) return FVector(1.00f, 0.72f, 0.00f);
	if (StandardizedScale <= 44.5f) return FVector(0.99f, 0.55f, 0.17f);
	if (StandardizedScale <= 57.0f) return FVector(0.85f, 0.35f, 0.75f);
	if (StandardizedScale <= 64.5f) return FVector(0.98f, 0.94f, 0.62f);
	if (StandardizedScale <= 78.0f) return FVector(1.00f, 0.71f, 0.68f);
	if (StandardizedScale <= 88.5f) return FVector(0.97f, 0.92f, 0.04f);
	if (StandardizedScale <= 110.0f) return FVector(0.62f, 0.87f, 0.07f);
	return FVector(0.08f, 0.61f, 0.04f);
}
