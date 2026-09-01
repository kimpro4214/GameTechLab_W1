#pragma once

#include "FVector.h"

class UPrimitive
{
public:
	virtual ~UPrimitive() = default;

	virtual bool IsColliding(const UPrimitive* Other) const = 0;
	virtual void AddVelocity(const FVector& DeltaVelocity) = 0;
};
