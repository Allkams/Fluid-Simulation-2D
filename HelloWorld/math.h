#pragma once
#include "Play.h"


namespace math
{
	bool rectangleCollision(const Point2f& pos, Point2f& vel , const Point2f& topLeft, const Point2f& bottomRight)
	{
		bool hit = false;
		const float dampFactor = 0.95f;
		if (pos.x - 6 < topLeft.x || pos.x + 6 > bottomRight.x)
		{
			vel.x = -vel.x * dampFactor;
			hit |= true;
		}

		if (pos.y - 6 < topLeft.y || pos.y + 6 > bottomRight.y)
		{
			vel.y = -vel.y * dampFactor;;
			hit |= true;
		}

		return hit;
	}

	static float SmoothingKernel(float radius, float dist);
	{

	}
}