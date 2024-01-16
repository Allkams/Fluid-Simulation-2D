//#define PLAY_IMPLEMENTATION
//#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Simulation.h"
#include "math.h"

namespace Fluid
{
	Simulation& Simulation::getInstance()
	{
		static Simulation instance;

		return instance;
	}

	// Paper way of simulation:
	// - For each particle, apply gravity
	// - Run function/Algorithm "ApplyViscosity"
	// - For each particle, save previous position, advance to predicted position.
	// - Run function/algorithm "adjust springs"
	// - Run function/algorithm "apply spring displacement"
	// - Run double density relaxsation
	// - Resolve collisions
	// - For each particle, use prev position and curret to compute next velocity.

	void Simulation::Update(float deltatime)
	{
		const float dampFactor = 0.95f;
		for (int i = 0; i < circleIDs.size(); i++)
		{
			Render::particle& particle = Render::GetParticle(i);
			//particle.vel.y += 9.82f * deltatime;

			//math::rectangleCollision({ particle.x, particle.y }, velocities[i], { 300, 300 }, { 1000 , 1000 });
			int prevX = particle.pos.x;
			int prevY = particle.pos.y;
			particle.pos += particle.vel;

			doubleDensityRelaxation(particle, i, deltatime);

			// Collision resolver, simple edition
			if (particle.pos.x - 6 < 200)
			{
				particle.pos.x = 10 + 6;
				particle.vel.x = -particle.vel.x * dampFactor;
			}
			else if (particle.pos.x + 6 >= 1200)
			{
				particle.pos.x = 1200 - 6;
				particle.vel.x = -particle.vel.x * dampFactor;
			}

			if (particle.pos.y - 6 < 10)
			{
				particle.pos.y = 10 + 6;
				particle.vel.y = -particle.vel.y * dampFactor;
			}
			else if (particle.pos.y + 6 >= 700)
			{
				particle.pos.y = 700 - 6;
				particle.vel.y = -particle.vel.y * dampFactor;
			}

			//
		}

		draw();
	}

	void Simulation::AddCircle(uint32_t cID)
	{
		circleIDs.push_back(cID);
	}

	void Simulation::doubleDensityRelaxation(Render::particle& p, int pID, float dt)
	{
		//Something incorrect here but on the correct path at least.
		float d = 0.0f;
		float dNear = 0.0f;
		for (int i = 0; i < circleIDs.size(); i++)
		{
			if (i == pID)
				continue;

			Render::particle& neighbour = Render::GetParticle(i);

			if ((neighbour.pos - p.pos).Length() > 10.0f)
				continue;

			Vector2f q = (neighbour.pos - p.pos) / 10.0f;
			float size = q.Length();
			if (size < 1.0f)
			{
				d += pow(1 - size, 2);
				dNear += pow(1 - size, 3);
			}
		}
		float P = 2 * (d - 1.0f);
		float pNear = 2 * dNear;
		Vector2f dx = {0,0};
		for (int i = 0; i < circleIDs.size(); i++)
		{
			if (i == pID)
				continue;

			Render::particle& neighbour = Render::GetParticle(i);

			if ((neighbour.pos - p.pos).Length() > 10.0f)
				continue;

			Vector2f q = (neighbour.pos - p.pos) / 10.0f;
			Vector2f qN = q;
			qN.Normalize();
			float size = q.Length();
			if (size < 1.0f)
			{
				Vector2f D = (dt * dt) * (P * (1 - size) + pNear * pow(1 - size, 2)) * qN;
				neighbour.pos += D / 2.0f;
				dx -= D / 2.0f;
			}
		}
		p.pos += dx;
	}

	void Simulation::draw()
	{
		Play::DrawRect({ 200, 50 }, { 1200 , 700 }, Play::cWhite);
	}
}