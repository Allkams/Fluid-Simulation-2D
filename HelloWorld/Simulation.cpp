//#define PLAY_IMPLEMENTATION
//#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Simulation.h"
#include "math.h"
#include "boundary.h"

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
		std::vector<Vector2f> prevPositons;
		for (int i = 0; i < circleIDs.size(); i++)
		{
			Render::particle& particle = Render::GetParticle(i);
			//particle.vel.y += 9.82f * deltatime;

			prevPositons.push_back(particle.pos);
			particle.pos += deltatime * particle.vel;

		}

		doubleDensityRelaxation(deltatime);

		//Collision towards boundaries
		for (int i = 0; i < circleIDs.size(); i++)
		{
			Render::particle& particle = Render::GetParticle(i);

			particle.vel = (particle.pos - prevPositons[i]) / deltatime;

			// Collision resolver, simple edition
			Point2D& topLeft = Render::Boundary::instance().getTopLeft();
			Point2D& bottomRight = Render::Boundary::instance().getBottomRight();
			if (particle.pos.x - 6 < topLeft.x)
			{
				particle.pos.x = topLeft.x + 6;
				particle.vel.x = -particle.vel.x * dampFactor;
			}
			else if (particle.pos.x + 6 >= bottomRight.x)
			{
				particle.pos.x = bottomRight.x - 6;
				particle.vel.x = -particle.vel.x * dampFactor;
			}

			if (particle.pos.y - 6 < topLeft.y)
			{
				particle.pos.y = topLeft.y + 6;
				particle.vel.y = -particle.vel.y * dampFactor;
			}
			else if (particle.pos.y + 6 >= bottomRight.y)
			{
				particle.pos.y = bottomRight.y - 6;
				particle.vel.y = -particle.vel.y * dampFactor;
			}


		}

		//draw();
	}

	void Simulation::AddCircle(uint32_t cID)
	{
		circleIDs.push_back(cID);
	}

	void Simulation::ClearCircles()
	{
		circleIDs.clear();
	}

	void Simulation::doubleDensityRelaxation(float dt)
	{
		const float interactionRadius = 100.0f;
		const float pressureMultiplier = 3.0f; // Adjust as needed

		for (int i = 0; i < circleIDs.size(); i++)
		{
			Render::particle& p = Render::GetParticle(i);

			const Vector2f OriginPos = p.pos;

			float d = 0.0f;
			float dNear = 0.0f;

			for (int j = 0; j < circleIDs.size(); j++)
			{
				if (j == i)
					continue;

				Render::particle& neighbour = Render::GetParticle(j);
				float dist = (neighbour.pos - OriginPos).Length();
				if (dist > interactionRadius)
					continue;

				const float influense = (neighbour.pos - OriginPos).Length() / interactionRadius;

				if (influense < 1.0f)
				{
					d += pow(1 - influense, 2);
					dNear += pow(1 - influense, 3);
				}
			}

			const float P = pressureMultiplier * (d - 1.0f);
			const float pNear = pressureMultiplier * dNear;

			Vector2f dx = { 0, 0 };

			for (int j = 0; j < circleIDs.size(); j++)
			{
				if (j == i)
					continue;

				Render::particle& neighbour = Render::GetParticle(j);

				const float distance = (neighbour.pos - OriginPos).Length();

				if (distance > interactionRadius)
					continue;

				const Vector2f q = (neighbour.pos - OriginPos) / interactionRadius;
				Vector2f qN = q;
				qN.Normalize();
				const float size = q.Length();

				if (size < 1.0f)
				{
					const Vector2f D = (dt * dt) * (P * (1 - size) + pNear * pow(1 - size, 2)) * qN;
					neighbour.pos += D / 2.0f;
					dx -= D / 2.0f;
				}
			}

			p.pos += dx;
		}
	}

	void Simulation::draw()
	{
		Play::DrawRect({ 200, 50 }, { 1200 , 700 }, Play::cWhite);
	}
}