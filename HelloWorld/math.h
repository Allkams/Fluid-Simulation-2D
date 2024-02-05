#pragma once
#include "Play.h"


namespace math
{
	static float SmoothingKernel(float dist, float radius)
	{
		if (dist >= radius) return 0;

		float volume = 3.14 * pow(radius, 4) / 6.0f;
		return (radius - dist) * (radius - dist) / volume;
	}

	static float SmoothingKernelDerivative(float dist, float radius)
	{
		if (dist >= radius) return 0;
		float scale = 12 / (pow(radius, 4) * 3.14);
		return (dist - radius) * scale;
	}
}