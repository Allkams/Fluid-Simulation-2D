#pragma once
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include "Play.h"
#include "particle.h"
#include "springPair.h"

namespace Fluid
{

	class Simulation
	{
	public:
		static Simulation& getInstance();
		void Update(float deltaTime);

		void AddCircle(uint32_t cID, const Vector2f& pos);
		void ClearData();

		void toggleGravity();
		Vector2f& getPosition(int id);

	private:

		//void applyViscosity(float dt);
		//void springAdjustment(float dt);
		//void doubleDensityRelaxation(float dt);
		//void springDisplacement(float dt);

		//void updateNeighbours();

		void updateDensities();

		float CalculateDensity(Vector2f& point);
		float ConvertDensityToPressure(float density);
		float CalculateProperty(int particleIndex);
		float CalculateSharedPressure(float densityA, float densityB);
		Vector2f& CalculatePropertyGradient(int particleIndex);
		Vector2f& CalculatePressureForce(int particleIndex);

		const float interactionRadius = 120.0f;
		bool gravity = false;
		const float TargetDensity = 275.0f;
		const float pressureMultiplier = 0.5f * 100.0f;

		std::vector<uint32_t> circleIDs;
		std::vector<Vector2f> positions;
		std::vector<Vector2f> velocity;
		std::vector<float> densities;
		std::vector<float> particleProperties;
		
		std::vector<SpringPair> springPairs;
		std::unordered_map<uint32_t, std::vector<uint32_t>> neighbourList;
	private:
		Simulation() {};
		Simulation(const Simulation& other) = delete;
		~Simulation() {};
	};
}