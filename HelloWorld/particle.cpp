#include "particle.h"

namespace Render
{
	static std::vector<particle> particlesAlloc;

	uint32_t CreateParticle(const Point2f& pos)
	{
		particle particle;
		particle.pos = pos;

		const uint32_t particleID = (uint32_t)particlesAlloc.size();
		particlesAlloc.push_back(particle);

		return particleID;
	}

	particle& GetParticle(int id)
	{
		return particlesAlloc[id];
	}

	void RemoveParticle(int32_t id)
	{
		if (id < particlesAlloc.size())
		{
			particlesAlloc.erase(particlesAlloc.begin() + id);
		}
		else
		{
			// Handle invalid particleID (out of range)
			printf("Error: Invalid particle ID, %i", id);
		}
	}
	void ClearParticles()
	{
		particlesAlloc.clear();
	}
}