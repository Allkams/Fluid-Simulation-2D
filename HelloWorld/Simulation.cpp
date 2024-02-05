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
		for (int i = 0; i < circleIDs.size(); i++)
		{
			if (gravity)
			{
				velocity[i].y += 98.20f * deltatime;
			}
			densities[i] = CalculateDensity(positions[i]);
		}

		for (int i = 0; i < circleIDs.size(); i++)
		{
			Vector2f pressureForce = CalculatePressureForce(i);
			Vector2f pressureAcceration = pressureForce / 1.0f;
			velocity[i] += pressureAcceration * deltatime;
		}

		for (int i = 0; i < circleIDs.size(); i++)
		{
			//prevPositons.push_back(positions[i]);
			positions[i] += deltatime * velocity[i];
			velocity[i] *= 0.99f;
		}

		//Collision towards boundaries
		for (int i = 0; i < circleIDs.size(); i++)
		{
			//Render::particle& particle = Render::GetParticle(i);

			//velocity[i] = (positions[i] - prevPositons[i]) / 0.01667f;

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
		densities.push_back(0);
		particleProperties.push_back(0);
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

	double distance(const Render::particle& p1, const Render::particle& p2) {
		return std::sqrt((p1.pos.x - p2.pos.x) * (p1.pos.x - p2.pos.x) + (p1.pos.y - p2.pos.y) * (p1.pos.y - p2.pos.y));
	}
	float distance(const Vector2f& particle1, const Vector2f& particle2) {
		return std::sqrt((particle1.x - particle2.x) * (particle1.x - particle2.x) + (particle1.y - particle2.y) * (particle1.y - particle2.y));
	}

	void Simulation::updateDensities()
	{
		//TODO: Multithread

		for (int i = 0; i < circleIDs.size(); i++)
		{
			densities[i] = CalculateDensity(positions[i]);
		}
	}

	float Simulation::CalculateDensity(Vector2f& point)
	{
		float density = 0;
		const float mass = 1;

		for(Vector2f& pos : positions)
		{
			float dist = distance(pos, point);
			float influence = math::SmoothingKernel(dist, interactionRadius);
			if (influence < 1.0f)
			{
				density += mass * influence;
			}
		}
		return density;
	}

	float Simulation::ConvertDensityToPressure(float density)
	{
		float densityError = density - TargetDensity;
		float pressure = densityError * pressureMultiplier;
		return pressure;
	}

	float Simulation::CalculateProperty(int particleIndex)
	{
		float property = 0.0f;
		for (int i = 0; i < circleIDs.size(); i++)
		{
			if (particleIndex == i) continue;

			float dist = distance(positions[i], positions[particleIndex]);
			float influence = math::SmoothingKernel(dist, interactionRadius);
			float density = densities[i];
			property += particleProperties[i] * 1.0f / density * influence;
		}
		return property;
	}

	float Simulation::CalculateSharedPressure(float densityA, float densityB)
	{
		float pressureA = ConvertDensityToPressure(densityA);
		float pressureB = ConvertDensityToPressure(densityB);
		return (pressureA + pressureB) / 2;
	}

	Vector2f& Simulation::CalculatePropertyGradient(int particleIndex)
	{
		Vector2f propertyGradient = {0,0};
		for (int i = 0; i < circleIDs.size(); i++)
		{
			if (particleIndex == i) continue;
			float dist = distance(positions[i], positions[particleIndex]);
			Vector2f dir = (positions[i] - positions[particleIndex]) / dist;
			float slope = math::SmoothingKernelDerivative(dist, interactionRadius);
			float density = densities[i];
			propertyGradient += -particleProperties[i] * dir * slope * 1.0f / density;
		}
		return propertyGradient;
	}

	Vector2f& Simulation::CalculatePressureForce(int particleIndex)
	{
		Vector2f pressureForce = { 0,0 };
		for (int i = 0; i < circleIDs.size(); i++)
		{
			if (particleIndex == i) continue;
			float dist = distance(positions[i], positions[particleIndex]);
			Vector2f dir = (positions[i] - positions[particleIndex]) / dist;
			float slope = math::SmoothingKernelDerivative(dist, interactionRadius);
			float density = densities[i];
			float sharedPressure = CalculateSharedPressure(density, densities[particleIndex]);
			pressureForce += -sharedPressure * dir * slope * 1.0f / density;
		}
		return pressureForce;
	}

}