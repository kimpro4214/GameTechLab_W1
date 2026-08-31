#pragma once

// Structure for a 3D vector
struct FVector
{
	float x, y, z;
	FVector(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z) {}

	FVector operator+(const FVector& Other) const
	{
		return FVector(x + Other.x, y + Other.y, z + Other.z);
	}

	FVector operator-(const FVector& Other) const
	{
		return FVector(x - Other.x, y - Other.y, z - Other.z);
	}

	FVector operator*(float Scalar) const
	{
		return FVector(x * Scalar, y * Scalar, z * Scalar);
	}

	FVector operator/(float Scalar) const
	{
		return FVector(x / Scalar, y / Scalar, z / Scalar);
	}

	FVector& operator+=(const FVector& Other)
	{
		x += Other.x;
		y += Other.y;
		z += Other.z;
		return *this;
	}

	FVector& operator-=(const FVector& Other)
	{
		x -= Other.x;
		y -= Other.y;
		z -= Other.z;
		return *this;
	}

	FVector& operator*=(float Scalar)
	{
		x *= Scalar;
		y *= Scalar;
		z *= Scalar;
		return *this;
	}

	static float DotProduct(const FVector& A, const FVector& B)
	{
		return A.x * B.x + A.y * B.y + A.z * B.z;
	}

	static float CrossProduct2D(const FVector& A, const FVector& B)
	{
		return A.x * B.y - A.y * B.x;
	}

	static FVector CrossProduct2D(float AngularSpeed, const FVector& Vector)
	{
		return FVector(-AngularSpeed * Vector.y, AngularSpeed * Vector.x, 0.0f);
	}
};
