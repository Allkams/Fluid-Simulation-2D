#pragma once
#include "Play.h"

namespace math
{
	static float SmoothingKernelPow2(float dist, float radius)
	{
		if (dist < radius)
		{
			float volume = 6.0f / (PLAY_PI * pow(radius, 4));
			float v = radius - dist;
			return v * v * volume;
		}
		return 0;
	}

	static float SmoothingKernelPow3(float dist, float radius)
	{
		if (dist < radius)
		{
			float volume = 10.0f / (PLAY_PI * pow(radius, 5));
			float v = radius - dist;
			return v * v * v * volume;
		}
		return 0;
	}


	static float SmoothingKernelDerivativePow2(float dist, float radius)
	{
		if (dist < radius)
		{
			float scale = 12 / (pow(radius, 4) * PLAY_PI);
			float v = (dist - radius);
			return -v * scale;
		}
		return 0;
	}

	static float SmoothingKernelDerivativePow3(float dist, float radius)
	{
		if (dist < radius)
		{
			float scale = 30 / (pow(radius, 5) * PLAY_PI);
			float v = (dist - radius);
			return -v * v * scale;
		}
		return 0;
	}
}