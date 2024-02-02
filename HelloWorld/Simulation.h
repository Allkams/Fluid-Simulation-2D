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

		void AddCircle(uint32_t cID);
		void ClearData();

	private:

		void applyViscosity(float dt);
		void springAdjustment(float dt);
		void doubleDensityRelaxation(float dt);
		void springDisplacement(float dt);

		void updateNeighbours();

		const float interactionRadius = 16.0f;

		std::vector<uint32_t> circleIDs;
		std::vector<SpringPair> springPairs;
		std::unordered_map<uint32_t, std::vector<uint32_t>> neighbourList;
	private:
		Simulation() {};
		Simulation(const Simulation& other) = delete;
		~Simulation() {};
	};
}