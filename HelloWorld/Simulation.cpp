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

		const float dampFactor = 0.90f;
		std::for_each(std::execution::par, circleIDs.begin(), circleIDs.end(),
			[this, deltatime](uint32_t i)
			{
				if (gravity)
				{
					velocity[i].y += 98.2f * deltatime;
				}
				predictedPositions[i] = positions[i] + velocity[i] * DT;
			});

		UpdateSpatialLookup();

		std::for_each(std::execution::par, circleIDs.begin(), circleIDs.end(),
			[this](uint32_t i)
			{
				densities[i] = CalculateDensity(i);
			});

		std::for_each(std::execution::par, circleIDs.begin(), circleIDs.end(),
			[this, deltatime](uint32_t i)
		{
			Vector2f pressureForce = CalculatePressureForce(i);
			Vector2f pressureAcceration = pressureForce / densities[i];
			velocity[i] += pressureAcceration * deltatime;
		});

		std::for_each(std::execution::par, circleIDs.begin(), circleIDs.end(),
			[this, deltatime, dampFactor](uint32_t i)
			{
				positions[i] += deltatime * velocity[i];
				//velocity[i] *= 0.99f;

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
		predictedPositions.push_back(pos);
		velocity.push_back({ 0,0 });
		densities.push_back(0);
		particleProperties.push_back(0);
	}

	void Simulation::ClearData()
	{
		circleIDs.clear();
	}

	void Simulation::toggleGravity()
	{
		gravity = !gravity;
	}

	Vector2f& Simulation::getPosition(int id)
	{
		return positions[id];
	}

	Vector2f& Simulation::getVelocity(int id)
	{
		return velocity[id];
	}

	float Simulation::getSpeed(int id)
	{
		return velocity[id].Length();
	}

	float Simulation::getSpeedNormalized(int id)
	{
		return std::clamp(velocity[id].Length(), 0.0f, 300.0f) / 300.0f;
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

	Vector2f PositionToCellCoord(const Vector2f& pos)
	{
		int GridPosX = (pos.x) / 100.0f;
		int GridPosY = (pos.y) / 100.0f;
		return { GridPosX, GridPosY };
	}

	uint32_t HashCell(int X, int Y)
	{
		uint32_t a = (uint32_t)X * 15823;
		uint32_t b = (uint32_t)Y * 9737333;
		return a + b;
	}

	uint32_t GetKeyFromHash(const uint32_t hash, const uint32_t spatialLength)
	{
		return hash % spatialLength;
	}

	void Simulation::updateDensities()
	{
		//TODO: Multithread

		for (int i = 0; i < circleIDs.size(); i++)
		{
			densities[i] = CalculateDensity(i);
		}
	}

	float Simulation::CalculateDensity(uint32_t particleIndex)
	{
		float density = 0;

		Vector2f pressureForce = { 0,0 };
		Vector2f pos = predictedPositions[particleIndex];
		Vector2f originCell = PositionToCellCoord(pos);
		float sqrRadius = interactionRadius * interactionRadius;

		for (int i = 0; i < 9; i++)
		{
			uint32_t hash = HashCell(originCell.x + offsets[i].x, originCell.y + offsets[i].y);
			uint32_t key = GetKeyFromHash(hash, spatialLookup.size());
			int currIndex = startIndices[key];

			while (currIndex < circleIDs.size())
			{
				SpatialStruct index = spatialLookup[currIndex];
				currIndex++;
				if (index.key != key) break;


				if (index.hash != hash) continue;

				uint32_t neighborIndex = index.index;
				//if (neighborIndex == particleIndex) continue;

				Vector2f neighbourPos = predictedPositions[neighborIndex];
				Vector2f offsetToNeighbour = neighbourPos - pos;
				float sqrDist = dot(offsetToNeighbour, offsetToNeighbour);

				if (sqrDist > sqrRadius) continue;

				float dist = distance(neighbourPos, pos);
				density += math::SmoothingKernelPow2(dist, interactionRadius);
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

	float Simulation::CalculateSharedPressure(float densityA, float densityB)
	{
		float pressureA = ConvertDensityToPressure(densityA);
		float pressureB = ConvertDensityToPressure(densityB);
		return (pressureA + pressureB) * 0.5f;
	}

	Vector2f& Simulation::CalculatePressureForce(int particleIndex)
	{
		Vector2f pressureForce = { 0,0 };
		Vector2f pos = predictedPositions[particleIndex];
		Vector2f originCell = PositionToCellCoord(pos);
		float sqrRadius = interactionRadius * interactionRadius;

		for (int i = 0; i < 9; i++)
		{
			uint32_t hash = HashCell(originCell.x + offsets[i].x, originCell.y + offsets[i].y);
			uint32_t key = GetKeyFromHash(hash, spatialLookup.size());
			int currIndex = startIndices[key];

			while (currIndex < circleIDs.size())
			{
				SpatialStruct index = spatialLookup[currIndex];
				currIndex++;
				if (index.key != key) break;


				if (index.hash != hash) continue;

				uint32_t neighborIndex = index.index;
				if (neighborIndex == particleIndex) continue;

				Vector2f neighbourPos = predictedPositions[neighborIndex];
				Vector2f offsetToNeighbour = neighbourPos - pos;
				float sqrDist = offsetToNeighbour.LengthSqr();

				if (sqrDist <= sqrRadius)
				{
					//callback.push_back(index);
					float dist = offsetToNeighbour.Length();
					Vector2f dir = dist > 0 ? offsetToNeighbour / dist : Vector2f(0, 1);
					float slope = math::SmoothingKernelDerivativePow2(dist, interactionRadius) ;
					float density = densities[neighborIndex];
					float sharedPressure = CalculateSharedPressure(density, densities[particleIndex]);
					pressureForce += dir * slope * sharedPressure / density;
				}
			}
		}

		return pressureForce;
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
				Vector2f cellPos = PositionToCellCoord(predictedPositions[i]);
				uint32_t hash = HashCell(cellPos.x, cellPos.y);
				uint32_t cellKey = GetKeyFromHash(hash, circleIDs.size());
				spatialLookup[i] = { cellKey, hash, i };
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
		
		//Vector2f offsets[9] = { {-1,-1}, {0, -1 }, {1, -1}, {-1, 0}, {0,0}, {1, 0}, {-1, 1}, {0,1}, {1,1} };

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