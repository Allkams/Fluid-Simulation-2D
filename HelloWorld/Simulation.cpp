#include "Simulation.h"

namespace Fluid
{
	Simulation& Simulation::getInstance()
	{
		static Simulation instance;

		return instance;
	}

	void Simulation::Update(float deltatime)
	{
		//Perfom updates here
	}
}