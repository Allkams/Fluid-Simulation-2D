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
		updateNeighbours();

		const float dampFactor = 0.95f;
		std::vector<Vector2f> prevPositons;
		for (int i = 0; i < circleIDs.size(); i++)
		{
			Render::particle& particle = Render::GetParticle(i);
			particle.vel.y += 9.82f * 0.01667f;
		}

		applyViscosity(0.01667f);

		for (int i = 0; i < circleIDs.size(); i++)
		{
			Render::particle& particle = Render::GetParticle(i);
			prevPositons.push_back(particle.pos);
			particle.pos += 0.01667f * particle.vel;

		}

		springAdjustment(0.01667f);
		springDisplacement(0.01667f);
		doubleDensityRelaxation(0.01667f);

		//Collision towards boundaries
		for (int i = 0; i < circleIDs.size(); i++)
		{
			Render::particle& particle = Render::GetParticle(i);

			particle.vel = (particle.pos - prevPositons[i]) / 0.01667f;

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

	double distance(const Render::particle& p1, const Render::particle& p2) {
		return std::sqrt((p1.pos.x - p2.pos.x) * (p1.pos.x - p2.pos.x) + (p1.pos.y - p2.pos.y) * (p1.pos.y - p2.pos.y));
	}

	void Simulation::applyViscosity(float dt)
	{
		const float alfa = 1.0f;
		const float beta = 0.10f;

		for (int i = 0; i < circleIDs.size(); i++)
		{
			auto it = neighbourList.find(i);
			Render::particle& particle = Render::GetParticle(i);
			for (int j = 0; j < it->second.size(); j++)
			{
				if (i == j || i > j)
				{
					continue;
				}
				Render::particle& neighbour = Render::GetParticle(j);

				//const float distance = distance;
				const float influense = distance(particle, neighbour) / interactionRadius;

				if (influense < 1)
				{
					const Vector2f q = (neighbour.pos - particle.pos) / interactionRadius;
					Vector2f qN = q;
					qN.Normalize();

					float u = dot((particle.vel - neighbour.vel), qN);
					if (u > 0)
					{
						Vector2f I = dt * (1 - influense) * ((alfa * u) + (beta * (u * u))) * qN;
						particle.vel -= I / 2.0f;
						neighbour.vel += I / 2.0f;
					}
				}

			}
		}
	}

	bool pairExists(const std::unordered_set<SpringPair>& springPairs, uint32_t id1, uint32_t id2, float interactionRadius)
	{
		return springPairs.find({ id1, id2, interactionRadius }) != springPairs.end();
	}

	size_t getTuple(const std::unordered_set<SpringPair>& springPairs, int id1, int id2)
	{
		int id = 0;
		for (const auto& pair : springPairs) 
		{
			if ((pair.index1 == id1 && pair.index2 == id2) || (pair.index1 == id2 && pair.index2 == id1)) 
			{
				return id;
			}
			id++;
		}
		return -1;
	}

	void Simulation::springAdjustment(float dt)
	{
		const float yieldRatio = 0.2f;
		const float yieldStretch = 0.2f;
		const float yieldCompress = 0.2f;

		for (int i = 0; i < circleIDs.size(); i++)
		{
			Render::particle& particle = Render::GetParticle(i);

			for (int j = 0; j < circleIDs.size(); j++)
			{
				if (i == j)
				{
					continue;
				}

				Render::particle& neighbour = Render::GetParticle(i);
				

				float dist = distance(particle, neighbour);
				const float influense = dist / interactionRadius;

				if (influense < 1)
				{
					if (!pairExists(springPairs, i, j, interactionRadius)) {
						springPairs.insert({ (uint32_t)i, (uint32_t)j, interactionRadius });
						continue;
					}

					auto it = springPairs.find({ (uint32_t)i, (uint32_t)j, interactionRadius });

					if (it != springPairs.end())
					{
						float& spring = const_cast<float&>(it->restSpring);

						float Deform = yieldRatio * spring;
						if (dist > interactionRadius + Deform)	// Stretch
						{
							spring = spring + 0.01667f * yieldStretch * (dist - interactionRadius - Deform);
						}
						else if (dist < interactionRadius - Deform)	// Compress
						{
							spring = spring - 0.01667f * yieldCompress * (interactionRadius - Deform - dist);
						}
					}

				}
			}
		}

		for (auto it = springPairs.begin(); it != springPairs.end();)
		{
			if (it->restSpring > interactionRadius)
			{
				it = springPairs.erase(it);
				continue;
			}
			
			++it;
		}

	}

	void Simulation::doubleDensityRelaxation(float dt)
	{
		const float pressureMultiplier = 10.0f; // Adjust as needed

		for (int i = 0; i < circleIDs.size(); i++)
		{
			Render::particle& particle = Render::GetParticle(i);

			auto it = neighbourList.find(i);

			if (it == neighbourList.end())
			{
				continue;
			}

			float d = 0.0f;
			float dNear = 0.0f;

			for (int j = 0; j < it->second.size(); j++)
			{
				if (j == i)
					continue;

				Render::particle& neighbour = Render::GetParticle(j);
				float dist = distance(particle, neighbour);
				if (dist > interactionRadius)
					continue;

				const float influense = dist / interactionRadius;

				if (influense < 1.0f)
				{
					d += pow(1 - influense, 2);
					dNear += pow(1 - influense, 3);
				}
			}

			const float P = pressureMultiplier * (d - 0.5f);
			const float pNear = pressureMultiplier * dNear;

			Vector2f dx = { 0, 0 };

			for (int j = 0; j < it->second.size(); j++)
			{
				if (j == i)
					continue;

				Render::particle& neighbour = Render::GetParticle(j);

				const float dist = distance(particle, neighbour);

				if (dist > interactionRadius)
					continue;

				const Vector2f q = (neighbour.pos - particle.pos) / interactionRadius;
				Vector2f qN = q;
				qN.Normalize();
				const float size = q.Length();

				if (size < 1.0f)
				{
					const Vector2f D = (0.01667f * 0.01667f) * (P * (1 - size) + pNear * pow(1 - size, 2)) * qN;
					neighbour.pos += D / 2.0f;
					dx -= D / 2.0f;
				}
			}

			particle.pos += dx;
		}
	}

	void Simulation::springDisplacement(float dt)
	{
		const float springConstant = 1.0f;

		for (const auto& springPair : springPairs)
		{
			Render::particle& particle = Render::GetParticle(springPair.index1);
			Render::particle& neighbour = Render::GetParticle(springPair.index2);

			float dist = distance(particle, neighbour);

			const Vector2f q = (neighbour.pos - particle.pos) / interactionRadius;
			Vector2f qN = q;
			qN.Normalize();
			const Vector2f D = powf(0.01667f, 2) * springConstant * ((1 - springPair.restSpring) / interactionRadius) * (springPair.restSpring - dist) * qN;
			particle.pos -= D / 2.0f;
			neighbour.pos += D / 2.0f;
		}

	}

	void Simulation::updateNeighbours()
	{
		neighbourList.clear();

		for (int i = 0; i < circleIDs.size(); i++)
		{
			Render::particle& particle = Render::GetParticle(i);
			neighbourList[i] = {};
			for (int j = 0; j < circleIDs.size(); j++)
			{
				Render::particle& neighbour = Render::GetParticle(j);

				const float dist = distance(particle, neighbour);

				if (dist <= interactionRadius)
				{
					neighbourList.at(i).push_back(j);
				}
			}
		}
	}
}