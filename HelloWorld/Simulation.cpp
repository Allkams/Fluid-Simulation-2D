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
		//updateNeighbours();

		const float dampFactor = 0.95f;
		std::vector<Vector2f> prevPositons;
		/*for (int i = 0; i < circleIDs.size(); i++)
		{
			Render::particle& particle = Render::GetParticle(i);
			particle.vel.y += 40.0f * 0.01667f;
		}*/

		//applyViscosity(0.01667f);

		for (int i = 0; i < circleIDs.size(); i++)
		{
			Render::particle& particle = Render::GetParticle(i);
			prevPositons.push_back(particle.pos);
			particle.pos += 0.01667f * particle.vel;

		}

		springAdjustment(0.01667f);
		springDisplacement(0.01667f);
		//doubleDensityRelaxation(0.01667f);

		//Collision towards boundaries
		for (int i = 0; i < circleIDs.size(); i++)
		{
			Render::particle& particle = Render::GetParticle(i);

			particle.vel = (particle.pos - prevPositons[i]) / 0.01667f;
			particle.vel *= 0.99f;

			// Collision resolver, simple edition
			Point2D& topLeft = Render::Boundary::instance().getTopLeft();
			Point2D& bottomRight = Render::Boundary::instance().getBottomRight();
			if (particle.pos.x - 4 < topLeft.x)
			{
				particle.pos.x = topLeft.x + 4;
				particle.vel.x = -particle.vel.x * dampFactor;
			}
			else if (particle.pos.x + 4 >= bottomRight.x)
			{
				particle.pos.x = bottomRight.x - 4;
				particle.vel.x = -particle.vel.x * dampFactor;
			}

			if (particle.pos.y - 4 < topLeft.y)
			{
				particle.pos.y = topLeft.y + 4;
				particle.vel.y = -particle.vel.y * dampFactor;
			}
			else if (particle.pos.y + 4 >= bottomRight.y)
			{
				particle.pos.y = bottomRight.y - 4;
				particle.vel.y = -particle.vel.y * dampFactor;
			}
		}
	}

	void Simulation::AddCircle(uint32_t cID)
	{
		circleIDs.push_back(cID);
	}

	void Simulation::ClearData()
	{
		circleIDs.clear();
		springPairs.clear();
		neighbourList.clear();
	}

	double distance(const Render::particle& p1, const Render::particle& p2) {
		return std::sqrt((p1.pos.x - p2.pos.x) * (p1.pos.x - p2.pos.x) + (p1.pos.y - p2.pos.y) * (p1.pos.y - p2.pos.y));
	}

	void Simulation::applyViscosity(float dt)
	{
		const float alfa = 10.0f;
		const float beta = 0.0f;

		for (int i = 0; i < circleIDs.size(); i++)
		{
			//auto it = neighbourList.find(i);
			Render::particle& particle = Render::GetParticle(i);
			for (int j = i + 1; j < circleIDs.size(); j++)
			{
				if (i == j)
				{
					continue;
				}
				Render::particle& neighbour = Render::GetParticle(j);

				//const float distance = distance;
				const float influense = distance(particle, neighbour) / interactionRadius;

				if (influense <= 1)
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

	//bool pairExists(const std::vector<SpringPair>& springPairs, SpringPair& insertPair)
	//{
	//	//return springPairs.find({ id1, id2, interactionRadius }) != springPairs.end();
	//	for (auto& pair : springPairs)
	//	{
	//		if (pair == insertPair)
	//		{
	//			return true;
	//		}
	//	}
	//	return false;
	//}

	int getPairID(const std::vector<SpringPair>& springPairs, const SpringPair& findPair)
	{
		for (int i = 0; i < springPairs.size(); i++)
		{
			if (springPairs[i] == findPair)
			{
				return i;
			}
		}
		return -1;
	}

	void Simulation::springAdjustment(float dt)
	{
		const float yieldRatio = 0.2f;
		const float Stretch = 0.3f;
		const float Compress = 0.3f;
		const Vector2f L = {16.0f,16.0f};

		for (uint32_t i = 0; i < circleIDs.size(); i++)
		{
			Render::particle& particle = Render::GetParticle(i);

			for (uint32_t j = 0; j < circleIDs.size(); j++)
			{
				if (i == j)
				{
					continue;
				}

				Render::particle& neighbour = Render::GetParticle(j);
				

				float dist = distance(particle, neighbour);
				const float influense = dist / interactionRadius;

				if (influense <= 1)
				{
					Vector2f particleDist = Vector2f(abs(neighbour.pos.x - particle.pos.x), abs(neighbour.pos.y - particle.pos.y));

					SpringPair pairToInsert = SpringPair(i, j, Vector2f(interactionRadius, interactionRadius));
					int id = getPairID(springPairs, pairToInsert);
  					if (id == -1) {
						springPairs.push_back(pairToInsert);
						//id = springPairs.size() - 1;
						continue;
					}

					SpringPair& pair = springPairs[id];

					Vector2f Deform = yieldRatio * pair.restSpring;
					
					// I do not understand what I even do...
					if (particleDist.Length() > (L + Deform).Length())	// Stretch
					{
  						pair.restSpring +=  dt * Stretch * (particleDist - L - Deform);
					}
					else if (particleDist.Length() < (L - Deform).Length())	// Compress
					{
						pair.restSpring -=  dt * Compress * (L - Deform - particleDist);
					}

					//if (pair.restSpring < 0.0f) // If negative
					//{
					//	pair.restSpring = 0.0f;
					//}

				}
			}
		}

		for (auto it = springPairs.begin(); it != springPairs.end();)
		{
			if (it->restSpring.x > interactionRadius && it->restSpring.y > interactionRadius)
			{
				it = springPairs.erase(it);
				continue;
			}
			
			++it;
		}

	}

	void Simulation::doubleDensityRelaxation(float dt)
	{
		const float pressureMultiplier = 4.0f; // Adjust as needed
		const float pressureNearMultiplier = 16.0f; // Adjust as needed

		for (int i = 0; i < circleIDs.size(); i++)
		{
			Render::particle& particle = Render::GetParticle(i);

			float d = 0.0f;
			float dNear = 0.0f;

			for (int j = 0; j < circleIDs.size(); j++)
			{
				if (j == i)
					continue;

				Render::particle& neighbour = Render::GetParticle(j);
				float dist = distance(particle, neighbour);
				if (dist > interactionRadius)
					continue;

				const float influense = dist / interactionRadius;

				if (influense <= 1.0f)
				{
					d += powf(1 - influense, 2);
					dNear += powf(1 - influense, 3);
				}
			}

			const float P = pressureMultiplier * (d - 50.0f);
			const float pNear = pressureNearMultiplier * dNear;

			Vector2f dx = { 0, 0 };

			for (int j = 0; j < circleIDs.size(); j++)
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
				const float influense = q.Length();

				if (influense <= 1.0f)
				{
					const Vector2f D = (dt * dt) * (P * (Vector2f(1, 1) - q) + pNear * ((Vector2f(1, 1) - q) * (Vector2f(1, 1) - q))) * qN;
					neighbour.pos -= D / 2.0f;
					dx += D / 2.0f;
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

			const Vector2f D = dt * dt * springConstant * (Vector2f(1,1) - (springPair.restSpring / interactionRadius)) * (springPair.restSpring - q) * qN;
			particle.pos -= D / 2.0f;
			neighbour.pos += D / 2.0f;
		}

	}

	void Simulation::updateNeighbours()
	{
		// Something incorrect here...
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