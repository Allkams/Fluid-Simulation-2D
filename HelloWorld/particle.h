#pragma once
#include "Play.h"
#include <vector>

namespace Render
{
	struct particle
	{
		float x, y;
		float vx = 0.0f, vy = 0.0f;
		float d = 1.0f;
	};

	uint32_t CreateParticle(const Point2f& pos);
	particle& GetParticle(int id);
}