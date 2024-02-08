//#define PLAY_IMPLEMENTATION
//#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Simulation.h"
#include <thread>
#include <execution>
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
		UpdateSpatialLookup();

		const float dampFactor = 0.95f;
		std::vector<Vector2f> prevPositons;
		std::for_each(std::execution::par, circleIDs.begin(), circleIDs.end(),
			[this](uint32_t i)
			{
				if (gravity)
				{
					velocity[i].y += 9.82f * 0.016667f;
				}
				densities[i] = CalculateDensity(positions[i]);
			});

		for(int i = 0; i < circleIDs.size(); i++)
		{
			Vector2f pressureForce = CalculatePressureForce(i);
			Vector2f pressureAcceration = pressureForce / 1.0f;
			velocity[i] += pressureAcceration * 0.016667f;
		}

		std::for_each(std::execution::par, circleIDs.begin(), circleIDs.end(),
			[this](uint32_t i)
			{
				//prevPositons.push_back(positions[i]);
				positions[i] += 0.016667f * velocity[i];
				velocity[i] *= 0.99f;
			});

		//Collision towards boundaries
		std::for_each(std::execution::par, circleIDs.begin(), circleIDs.end(),
			[this, dampFactor](uint32_t i)
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
			});
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
		return positions[id];
	}

	double distance(const Render::particle& p1, const Render::particle& p2) {
		return std::sqrt((p1.pos.x - p2.pos.x) * (p1.pos.x - p2.pos.x) + (p1.pos.y - p2.pos.y) * (p1.pos.y - p2.pos.y));
	}
	float distance(const Vector2f& particle1, const Vector2f& particle2) {
		return std::sqrt((particle1.x - particle2.x) * (particle1.x - particle2.x) + (particle1.y - particle2.y) * (particle1.y - particle2.y));
	}

	float distanceSquared(const Vector2f& particle1, const Vector2f& particle2) {
		return (particle1.x - particle2.x) * (particle1.x - particle2.x) + (particle1.y - particle2.y) * (particle1.y - particle2.y);
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
			if (dist > interactionRadius) continue;

			float influence = math::SmoothingKernel(dist, interactionRadius);
			/*if (influence < 1.0f)
			{*/
				density += mass * influence;
			//}
		}
		assert(density != 0.0f);
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
		std::vector<uint32_t> neighbors;
		SpatialNeighbors(particleIndex, neighbors);
		for (int i = 0; i < circleIDs.size(); i++)
		{
			if (particleIndex == i) continue;
			uint32_t index = neighbors[i];
			float dist = distance(positions[index], positions[particleIndex]);
			float influence = math::SmoothingKernel(dist, interactionRadius);
			float density = densities[index];
			property += particleProperties[index] * 1.0f / density * influence;
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
		std::vector<uint32_t> neighbors;
		SpatialNeighbors(particleIndex, neighbors);
		for (int i = 0; i < neighbors.size(); i++)
		{
			if (particleIndex == i) continue;
			uint32_t index = neighbors[i];
			float dist = distance(positions[index], positions[particleIndex]);
			Vector2f dir = (positions[index] - positions[particleIndex]) / dist;
			float slope = math::SmoothingKernelDerivative(dist, interactionRadius);
			float density = densities[index];
			propertyGradient += -particleProperties[index] * dir * slope * 1.0f / density;
		}
		return propertyGradient;
	}

	Vector2f& Simulation::CalculatePressureForce(int particleIndex)
	{
		Vector2f pressureForce = { 0,0 };
		std::vector<uint32_t> neighbors;
		SpatialNeighbors(particleIndex, neighbors);
		for (int i = 0; i < neighbors.size(); i++)
		{
			if (particleIndex == i) continue;
			uint32_t index = neighbors[i];
			float dist = distance(positions[index], positions[particleIndex]);
			Vector2f dir = dist == 0 ? Vector2f(Play::RandomRollRange(0,1), Play::RandomRollRange(0, 1)) : (positions[index] - positions[particleIndex]) / dist;
			float slope = math::SmoothingKernelDerivative(dist, interactionRadius);
			float density = densities[index];
			float sharedPressure = CalculateSharedPressure(density, densities[particleIndex]);
			pressureForce += -sharedPressure * dir * slope * 1.0f / density;
		}

		return pressureForce;
	}

	Vector2f PositionToCellCoord(Vector2f& pos)
	{
		int GridPosX = (pos.x /*- Render::Boundary::instance().getTopLeft().x*/) / 100.0f;
		int GridPosY = (pos.y /*- Render::Boundary::instance().getTopLeft().y*/) / 100.0f;
		return { GridPosX, GridPosY };
	}

	uint32_t HashCell(int X, int Y)
	{
		uint32_t a = (uint32_t)X * 15823;
		uint32_t b = (uint32_t)Y * 9737333;
		return a + b;
	}

	uint32_t GetKeyFromHash(uint32_t hash, uint32_t spatialLength)
	{
		return hash % spatialLength;
	}
	//Might be incorrect...
	void Simulation::UpdateSpatialLookup()
	{
		/*spatialLookup.clear();
		startIndices.clear();*/
		if (spatialLookup.size() == 0)
		{
			spatialLookup.resize(circleIDs.size());
			startIndices.resize(circleIDs.size());
		}
		std::for_each(std::execution::par, circleIDs.begin(), circleIDs.end(),
			[this](uint32_t i)
			{
				Vector2f cellPos = PositionToCellCoord(positions[i]);
				uint32_t cellKey = GetKeyFromHash(HashCell(cellPos.x, cellPos.y), circleIDs.size());
				spatialLookup[i] = { cellKey, i };
				startIndices[i] = INT_MAX;
			});

		std::sort(spatialLookup.begin(), spatialLookup.end(), compareByKey);

		std::for_each(std::execution::par, circleIDs.begin(), circleIDs.end(),
			[this](uint32_t i)
			{
				uint32_t key = spatialLookup[i].key;
				uint32_t keyPrev = i == 0 ? UINT32_MAX : spatialLookup[i - 1].key;
				if (key != keyPrev)
				{
					startIndices[key] = i;
				}
			});

	}

	void Simulation::SpatialNeighbors(int particleIndex, std::vector<uint32_t>& callback)
	{
		Vector2f CellPos = PositionToCellCoord(positions[particleIndex]);
		float sqrRadius = interactionRadius * interactionRadius;
		
		Vector2f offsets[9] = { {-1,-1}, {0, -1 }, {1, -1}, {-1, 0}, {0,0}, {1, 0}, {-1, 1}, {0,1}, {1,1} };

		for (auto& offset : offsets)
		{
			uint32_t key = GetKeyFromHash(HashCell(CellPos.x + offset.x, CellPos.y + offset.y), spatialLookup.size());
			int cellStartIndex = startIndices[key];

			for (int i = cellStartIndex; i < spatialLookup.size(); i++)
			{
				if (spatialLookup[i].key != key) break;


				uint32_t index = spatialLookup[i].index;
				if (particleIndex == index) continue;
				float sqrDist = distanceSquared(positions[particleIndex], positions[index]);

				if (sqrDist <= sqrRadius)
				{
					callback.push_back(index);
				}
			}
		}
	}

}