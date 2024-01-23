#define PLAY_IMPLEMENTATION
//#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"
#include "particle.h"
#include "boundary.h"
#include "Simulation.h"

const int DISPLAY_WIDTH = 1920;	//School
const int DISPLAY_HEIGHT = 1080;	//School

//const int DISPLAY_WIDTH = 1400;		//Laptop
//const int DISPLAY_HEIGHT = 750;		//Laptop

const int DISPLAY_SCALE = 1;

double dt = 0.016667;

double Max = 0.0;
double Min = 100.0;

std::vector<uint32_t> circles;
int size = 0;

const int ParticleAmmount = 100;
const int RowSize = 20;
const short gap = 30;

bool bPaused = true;

// The entry point for a PlayBuffer program
void MainGameEntry( PLAY_IGNORE_COMMAND_LINE )
{
	//if (ammountPerXY > 1)
	//{
	//	for (int i = -ammountPerXY/2; i < ammountPerXY / 2; i++)
	//	{
	//		int x = (DISPLAY_WIDTH / 2.0f) + (gap * i);
	//		for (int j = -ammountPerXY / 2; j < ammountPerXY / 2; j++)
	//		{
	//			int y = (DISPLAY_HEIGHT / 2.0f) + (gap * j);
	//			uint32_t id = Render::CreateParticle({ x, y });
	//			circles.push_back(id);
	//			Fluid::Simulation::getInstance().AddCircle(id);
	//			size++;
	//		}
	//	}
	//}
	//else
	//{
	//	int x = (DISPLAY_WIDTH / 2.0f);
	//	int y = (DISPLAY_HEIGHT / 2.0f);
	//	uint32_t id = Render::CreateParticle({ x, y });
	//	circles.push_back(id);
	//	Fluid::Simulation::getInstance().AddCircle(id);
	//	size++;
	//}
	int totalWidth = ParticleAmmount % RowSize;
	int TotalOffsetFromCenterWidth = totalWidth == 0 ? RowSize * gap : totalWidth * gap;

	int totalHeight = ParticleAmmount / RowSize;
	int TotalOffsetFromCenterHeight = totalHeight * gap;

	for (int i = 0; i < ParticleAmmount; i++)
	{
		int localX = i % RowSize;
		int localY = i / RowSize;

		int XOffset = localX * gap;
		int YOffset = localY * gap;

		int worldOffsetX = (DISPLAY_WIDTH / 2) - (TotalOffsetFromCenterWidth / 2);
		int worldOffsetY = (DISPLAY_HEIGHT / 2) - (TotalOffsetFromCenterHeight / 2);
		int x = worldOffsetX + XOffset;
		int y = worldOffsetY + YOffset;

		uint32_t id = Render::CreateParticle({ x, y });
		circles.push_back(id);
		Fluid::Simulation::getInstance().AddCircle(id);
		size++;
	}

	Render::Boundary::instance().resize(500, 500);
	Render::Boundary::instance().move({ DISPLAY_WIDTH /2, DISPLAY_HEIGHT /2 });

	Play::CreateManager( DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE );
}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate( float elapsedTime )
{
	auto timeStart = std::chrono::steady_clock::now();
	Play::ClearDrawingBuffer( Play::cBlack );

	//Simulation

	auto timeStartOld = std::chrono::steady_clock::now();

	if (bPaused && (Play::KeyPressed(0x4E) || Play::KeyDown(0x4D)))
	{
		Fluid::Simulation::getInstance().Update(dt);
	}
	else if (!bPaused)
	{
		Fluid::Simulation::getInstance().Update(dt);
	}

	//Vector2f pos = { Render::GetParticle(0).pos.x, Render::GetParticle(0).pos.y };
	//Play::DrawFilledCircle(pos, 25.0f, Play::cRed, 0.5f);

	for (int i = 0; i < circles.size(); i++)
	{
		Vector2f pos = { Render::GetParticle(i).pos.x, Render::GetParticle(i).pos.y };
		Play::FastDrawFilledCircle(pos, Play::cCyan);
	}
	Play::DrawFilledCircle({ DISPLAY_WIDTH / 2.0f, DISPLAY_HEIGHT / 2.0f }, 10.0f, Play::cWhite, 1.0f);

	auto timeEndOld = std::chrono::steady_clock::now();

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

	Play::DrawDebugText({ 10, 10 }, text.c_str(), fps < 25 ? Play::cRed : Play::cWhite, false);
	Play::DrawDebugText({ 10, 25 }, textdt.c_str(), fps < 25 ? Play::cRed : Play::cWhite, false);

	Play::DrawDebugText({ 10, DISPLAY_HEIGHT - 25 }, textNew.c_str(), fps < 25 ? Play::cRed : Play::cWhite, false);
	Play::DrawDebugText({ 225, DISPLAY_HEIGHT - 25 }, textMin.c_str(), fps < 25 ? Play::cRed : Play::cWhite, false);
	Play::DrawDebugText({ 400, DISPLAY_HEIGHT - 25 }, textMax.c_str(), fps < 25 ? Play::cRed : Play::cWhite, false);
	Play::DrawDebugText({ DISPLAY_WIDTH - 300, 10 }, textballs.c_str(), Play::cWhite, false);

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

