//#define PLAY_IMPLEMENTATION
//#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Simulation.h"
#include "math.h"
#include "particle.h"

namespace Fluid
{
	Simulation& Simulation::getInstance()
	{
		static Simulation instance;

		return instance;
	}

	void Simulation::Update(float deltatime)
	{
		const float dampFactor = 0.95f;
		for (int i = 0; i < circleIDs.size(); i++)
		{
			Render::particle& particle = Render::GetParticle(i);
			particle.vy += 9.82f * deltatime;

			//math::rectangleCollision({ particle.x, particle.y }, velocities[i], { 300, 300 }, { 1000 , 1000 });


			particle.x += particle.vx;
			particle.y += particle.vy;

			if (particle.x - 6 < 10 || particle.x + 6 > 1900)
			{
				particle.vx = -particle.vx * dampFactor;
			}

			if (particle.y - 6 < 10 || particle.y + 6 > 1000)
			{
				particle.vy = -particle.vy * dampFactor;;
			}
		}

		draw();
	}

	void Simulation::AddCircle(uint32_t cID)
	{
		circleIDs.push_back(cID);
	}

	void Simulation::draw()
	{
		Play::DrawRect({ 10, 10 }, { 1900 , 1000 }, Play::cWhite);
	}
}