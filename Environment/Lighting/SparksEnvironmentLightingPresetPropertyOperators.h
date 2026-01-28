// © 2024 Jar of Sparks, All Rights Reserved

#pragma once

namespace SparksEnvironmentLightingPresetPropertyOperators
{
	static FColor MultiplyByScalar(const FColor& Color, const float Scalar)
	{
		return FColor(Color.R * Scalar, Color.G * Scalar, Color.B * Scalar);
	}

	static FLinearColor LinearColorAllZeroes(0.0, 0.0, 0.0, 0.0);
};
