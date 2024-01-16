#pragma once
#include "Play.h"
#include <vector>

namespace Render
{
	struct particle
	{
		Vector2f pos;
		Vector2f vel = {0.0f, 0.0f};
		float d = 1.0f;
		float p = 0.0f;
	};

	uint32_t CreateParticle(const Point2f& pos);
	particle& GetParticle(int id);
}