#pragma once
#include <vector>
#include "Play.h"

namespace Fluid
{
	class Simulation
	{
	public:
		static Simulation& getInstance();
		void Update(float deltaTime);
		void HandleInput();

		void AddCircle();
		void initCircleAmount(int Ammount);

	private:
		void pauseSimulation(bool bPause);
		void incrementSimulationStep(int amount = 1);
		void deincrementSimulationStep(int amount = 1);

		void draw();

		float global_DT;

		std::vector<int> circleIDs;
		std::vector<Vector2D> velocities;
		std::vector<float> densities;
	private:
		Simulation() {};
		Simulation(const Simulation& other) = delete;
		~Simulation() {};
	};
}