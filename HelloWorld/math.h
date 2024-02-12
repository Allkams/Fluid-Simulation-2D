#pragma once
#include "Play.h"

namespace math
{
	static float SmoothingKernel(float dist, float radius)
	{
		if (dist >= radius) return 0;

		float volume = PLAY_PI * pow(radius, 4) / 6.0f;
		return (radius - dist) * (radius - dist) / volume;
	}

	static float SmoothingKernelDerivative(float dist, float radius)
	{
		if (dist < radius)
		{
			float scale = 12 / (pow(radius, 4) * PLAY_PI);
			return (dist - radius) * scale;
		}
		return 0;
	}

	static float SmoothingKernelDerivativePow2(float dist, float radius)
	{
		if (dist < radius)
		{
			float scale = 12 / (pow(radius, 4) * PLAY_PI);
			float v = (dist - radius);
			return v * v * scale;
		}
		return 0;
	}
}