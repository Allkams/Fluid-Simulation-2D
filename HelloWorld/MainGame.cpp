#define PLAY_IMPLEMENTATION
//#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"
#include "particle.h"
#include "boundary.h"
#include "Simulation.h"
#include <cmath>

//const int DISPLAY_WIDTH = 1920;	//School
//const int DISPLAY_HEIGHT = 1080;	//School

const int DISPLAY_WIDTH = 1920 * 0.92f;		//Laptop
const int DISPLAY_HEIGHT = 1080 * 0.92f;		//Laptop

const int DISPLAY_SCALE = 1;

double dt = 0.016667;

double Max = 0.0;
double Min = 100.0;

std::vector<uint32_t> circles;
int size = 0;

int ParticleAmmount = 100;
int RowSize = 20;
const short gap = 10;

bool bPaused = true;

/* 
 * TODOS:
 *  - Achive pure density (Grid stacking correctly)
 *  - Make it work with 9.82 F gravity (or 98.2)
 *  - Spatial Neighborhood search with hash functions.
 *  - Correct stacking.
 *  - Particle Blending through alpha channel.
 *  - Data oriented design for everything.
 *		- Create one read and one write buffer for Position, Velocity, Density etc.
 *  - Multithread system through the data oriented design pattern.
 *  - Display more information on the screen from the simulation.
*/


void GenerateGrid()
{
	if (circles.size() > 0)
	{
		circles.clear();
		Render::ClearParticles();
		Fluid::Simulation::getInstance().ClearData();
		size = 0;
	}

	int totalWidth = ParticleAmmount > RowSize ? 0 : ParticleAmmount% RowSize;
	int TotalOffsetFromCenterWidth = totalWidth == 0 ? RowSize * gap : totalWidth * gap;

	int totalHeight = ceil((float)ParticleAmmount / (float)RowSize);
	int TotalOffsetFromCenterHeight = totalHeight * gap;

	for (int i = 0; i < ParticleAmmount; i++)
	{
		int localX = i % RowSize;
		int localY = i / RowSize;

		int XOffset = localX * gap;
		int YOffset = localY * gap;

		int worldOffsetX = (DISPLAY_WIDTH / 2) - ((TotalOffsetFromCenterWidth - gap) / 2.0f);
		int worldOffsetY = (DISPLAY_HEIGHT / 2) - ((TotalOffsetFromCenterHeight - gap) / 2.0f);
		int x = worldOffsetX + XOffset;
		int y = worldOffsetY + YOffset;

		uint32_t id = Render::CreateParticle({ x, y });
		circles.push_back(id);
		Fluid::Simulation::getInstance().AddCircle(id);
		size++;
	}
}

// The entry point for a PlayBuffer program
void MainGameEntry( PLAY_IGNORE_COMMAND_LINE )
{
	GenerateGrid();

	Render::Boundary::instance().resize(400, 400);
	Render::Boundary::instance().move({ DISPLAY_WIDTH /2, DISPLAY_HEIGHT /2 });

	Play::CreateManager( DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE );
}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate( float elapsedTime )
{
	auto timeStart = std::chrono::steady_clock::now();
	Play::ClearDrawingBuffer( Play::cBlack );

	//Simulation


	if (Play::KeyPressed(VK_SPACE))
	{
		bPaused = !bPaused;
	}

	auto timeStartOld = std::chrono::steady_clock::now();
	if (bPaused && (Play::KeyPressed(0x4E) || Play::KeyDown(0x4D)))
	{
		Fluid::Simulation::getInstance().Update(dt);
	}
	else if (!bPaused)
	{
		Fluid::Simulation::getInstance().Update(dt);
	}
	auto timeEndOld = std::chrono::steady_clock::now();

	if (Play::KeyPressed(VK_RIGHT))
	{
		ParticleAmmount++;
		GenerateGrid();
	}
	if (Play::KeyPressed(VK_LEFT))
	{
		ParticleAmmount--;
		GenerateGrid();
	}
	if (Play::KeyPressed(VK_UP))
	{
		RowSize++;
		GenerateGrid();
	}
	if (Play::KeyPressed(VK_DOWN))
	{
		RowSize--;
		GenerateGrid();
	}

	if (Play::KeyPressed(0x52))
	{
		dt = 0.016667;
		GenerateGrid();
	}


	//Vector2f pos = { Render::GetParticle(22).pos.x, Render::GetParticle(22).pos.y };
	//Play::DrawFilledCircle(pos, 16.0f, Play::cRed, 0.5f);
	for (int i = 0; i < circles.size(); i++)
	{
		Render::particle& p = Render::GetParticle(i);
		Vector2f pos = { p.pos.x, p.pos.y };
		Play::FastDrawFilledCircle(pos, Play::cCyan);
	}
	//Play::DrawFilledCircle({ DISPLAY_WIDTH / 2.0f, DISPLAY_HEIGHT / 2.0f }, 10.0f, Play::cWhite, 1.0f);


	double elapseOld = std::chrono::duration<double>(timeEndOld - timeStartOld).count();

	Render::Boundary::instance().draw();
	if (elapseOld < Min)
	{
		Min = elapseOld;
	}
	if (elapseOld > Max)
	{
		Max = elapseOld;
	}

	int fps = 1 / dt;
	std::string text = "fps: " + std::to_string(fps);
	std::string textdt = "DT: " + std::to_string(dt);
	std::string textNew = "Elapsed function: " + std::to_string(elapseOld);
	std::string textMin = "Min time: " + std::to_string(Min);
	std::string textMax = "Max time: " + std::to_string(Max);
	std::string textballs = "Particle Amount: " + std::to_string(ParticleAmmount);
	std::string textPaused = (bPaused == true) ? "Paused" : "Running";

	Play::DrawDebugText({ 10, 10 }, text.c_str(), fps < 25 ? Play::cRed : Play::cWhite, false);
	Play::DrawDebugText({ 10, 25 }, textdt.c_str(), fps < 25 ? Play::cRed : Play::cWhite, false);

	Play::DrawDebugText({ 10, DISPLAY_HEIGHT - 25 }, textNew.c_str(), fps < 25 ? Play::cRed : Play::cWhite, false);
	Play::DrawDebugText({ 225, DISPLAY_HEIGHT - 25 }, textMin.c_str(), fps < 25 ? Play::cRed : Play::cWhite, false);
	Play::DrawDebugText({ 400, DISPLAY_HEIGHT - 25 }, textMax.c_str(), fps < 25 ? Play::cRed : Play::cWhite, false);
	Play::DrawDebugText({ DISPLAY_WIDTH - 300, 10 }, textballs.c_str(), Play::cWhite, false);
	Play::DrawDebugText({ DISPLAY_WIDTH - 300, 35 }, textPaused.c_str(), bPaused ? Play::cRed : Play::cGreen, false);

	Play::DrawDebugText({ DISPLAY_WIDTH / 2, 10 }, "Fluid Simulation By Alexander Marklund (Allkams)!");
	Play::DrawDebugText({ DISPLAY_WIDTH / 2, 25 }, "Created with Playbuffer");

	Play::PresentDrawingBuffer();
	auto timeEnd = std::chrono::steady_clock::now();
	dt = std::chrono::duration<double>(timeEnd - timeStart).count();

	return Play::KeyDown( VK_ESCAPE );
}

// Gets called once when the player quits the game 
int MainGameExit( void )
{
	Play::DestroyManager();
	return PLAY_OK;
}

