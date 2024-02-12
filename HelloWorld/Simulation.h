#pragma once
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include "Play.h"
#include "particle.h"
#include "springPair.h"

namespace Fluid
{
	struct SpatialStruct
	{
		uint32_t key;
		uint32_t hash;
		uint32_t index;
		SpatialStruct() : key(UINT32_MAX), index(UINT32_MAX), hash(UINT32_MAX) {};
		SpatialStruct(uint32_t inKey, uint32_t inHash, uint32_t inIndex) : key(inKey), index(inIndex), hash(inHash) {};
	};

	inline bool compareByKey(const SpatialStruct& a, const SpatialStruct& b)
	{
		return a.key < b.key;
	}

	class Simulation
	{
	public:
		static Simulation& getInstance();
		void Update(float deltaTime);

		void AddCircle(uint32_t cID, const Vector2f& pos);
		void ClearData();

		void toggleGravity();
		Vector2f& getPosition(int id);
		Vector2f& getVelocity(int id);
		float getSpeed(int id);
		float getSpeedNormalized(int id);

	private:

		void updateDensities();

		float CalculateDensity(uint32_t particleIndex);
		float ConvertDensityToPressure(float density);
		float CalculateSharedPressure(float densityA, float densityB);
		Vector2f& CalculatePressureForce(int particleIndex);

		void UpdateSpatialLookup();
		void SpatialNeighbors(int particleIndex, std::vector<uint32_t>& callback);

		const float interactionRadius = 35.0f;
		bool gravity = false;
		const float TargetDensity = 10.0f;
		const float pressureMultiplier = 30.0f;

		const float DT = 1.0f / 60.0f;

		std::vector<uint32_t> circleIDs;

		//std::map<uint32_t, uint32_t> spatialLookup;
		std::vector<SpatialStruct> spatialLookup;
		std::vector<uint32_t> startIndices;

		std::vector<Vector2f> positions;
		std::vector<Vector2f> predictedPositions;
		std::vector<Vector2f> velocity;
		std::vector<float> densities;
		std::vector<float> particleProperties;

		const Vector2f offsets[9] = { {-1,-1}, {0, -1 }, {1, -1}, {-1, 0}, {0,0}, {1, 0}, {-1, 1}, {0,1}, {1,1} };
		
	private:
		Simulation() {};
		Simulation(const Simulation& other) = delete;
		~Simulation() {};
	};
}