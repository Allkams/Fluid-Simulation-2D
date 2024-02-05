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
		if (gravity)
		{
			for (int i = 0; i < circleIDs.size(); i++)
			{
				//Render::particle& particle = Render::GetParticle(i);
				velocity[i].y += 40.0f * 0.01667f;
			}
		}

		for (int i = 0; i < circleIDs.size(); i++)
		{
			prevPositons.push_back(positions[i]);
			positions[i] += 0.01667f * velocity[i];
		}

		//Collision towards boundaries
		for (int i = 0; i < circleIDs.size(); i++)
		{
			//Render::particle& particle = Render::GetParticle(i);

			velocity[i] = (positions[i] - prevPositons[i]) / 0.01667f;
			velocity[i] *= 0.99f;

			// Collision resolver, simple edition
			Point2D& topLeft = Render::Boundary::instance().getTopLeft();
			Point2D& bottomRight = Render::Boundary::instance().getBottomRight();
			if (positions[i].x - 4 < topLeft.x)
			{
				positions[i].x = topLeft.x + 4;
				velocity[i].x = -velocity[i].x * dampFactor;
			}
			else if (positions[i].x + 4 >= bottomRight.x)
			{
				positions[i].x = bottomRight.x - 4;
				velocity[i].x = -velocity[i].x * dampFactor;
			}

			if (positions[i].y - 4 < topLeft.y)
			{
				positions[i].y = topLeft.y + 4;
				velocity[i].y = -velocity[i].y * dampFactor;
			}
			else if (positions[i].y + 4 >= bottomRight.y)
			{
				positions[i].y = bottomRight.y - 4;
				velocity[i].y = -velocity[i].y * dampFactor;
			}
		}
	}

	void Simulation::AddCircle(uint32_t cID, const Vector2f& pos)
	{
		circleIDs.push_back(cID);
		positions.push_back(pos);
		velocity.push_back({ 0,0 });
	}

	void Simulation::ClearData()
	{
		circleIDs.clear();
		springPairs.clear();
		neighbourList.clear();
	}

	void Simulation::toggleGravity()
	{
		gravity = !gravity;
	}

	Vector2f& Simulation::getPosition(int id)
	{
		// TODO: insert return statement here
		return positions[id];
	}

	float Simulation::CalculateDensity(Vector2f& point)
	{
		float density = 0;
		const float mass = 1;

		for(Vector2f& pos : positions)
		{
			float dist = distance(pos, point);
			float influence = math::SmoothingKernel(interactionRadius, dist);
			if (influence < 1.0f)
			{
				density += mass * influence;
			}
		}
		return density;
	}

	double distance(const Render::particle& p1, const Render::particle& p2) {
		return std::sqrt((p1.pos.x - p2.pos.x) * (p1.pos.x - p2.pos.x) + (p1.pos.y - p2.pos.y) * (p1.pos.y - p2.pos.y));
	}
	float distance(const Vector2f& particle1, const Vector2f& particle2) {
		return std::sqrt((particle1.x - particle2.x) * (particle1.x - particle2.x) + (particle1.y - particle2.y) * (particle1.y - particle2.y));
	}
}