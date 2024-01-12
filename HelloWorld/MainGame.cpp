#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"
#include "Simulation.h"

const int DISPLAY_WIDTH = 1920;
const int DISPLAY_HEIGHT = 1080;
const int DISPLAY_SCALE = 1;

double dt = 0.016667;

double Max = 0.0;
double Min = 100.0;

std::vector<Point2f> positions;
int size = 0;

const int ammountPerXY = 100;
const short gap = 7;

// The entry point for a PlayBuffer program
void MainGameEntry( PLAY_IGNORE_COMMAND_LINE )
{
	for (int i = -ammountPerXY/2; i < ammountPerXY / 2; i++)
	{
		int x = (DISPLAY_WIDTH / 2.0f) + (gap * i);
		for (int j = -ammountPerXY / 2; j < ammountPerXY / 2; j++)
		{
			int y = (DISPLAY_HEIGHT / 2.0f) + (gap * j);
			positions.push_back({ x , y });
			size++;
		}
	}

	Play::CreateManager( DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE );
}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate( float elapsedTime )
{
	auto timeStart = std::chrono::steady_clock::now();
	Play::ClearDrawingBuffer( Play::cBlack );

	//Simulation
	//Fluid::Simulation::getInstance().Update(dt);

	auto timeStartOld = std::chrono::steady_clock::now();

	for (int i = 0; i < positions.size(); i++)
	{
		Play::FastDrawFilledCircle(positions[i], Play::cRed);
	}

	auto timeEndOld = std::chrono::steady_clock::now();

	double elapseOld = std::chrono::duration<double>(timeEndOld - timeStartOld).count();

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

	Play::DrawDebugText({ 10, 10 }, text.c_str(), fps < 25 ? Play::cRed : Play::cWhite, false);
	Play::DrawDebugText({ 10, 25 }, textdt.c_str(), fps < 25 ? Play::cRed : Play::cWhite, false);

	Play::DrawDebugText({ 10, DISPLAY_HEIGHT - 25 }, textNew.c_str(), fps < 25 ? Play::cRed : Play::cWhite, false);
	Play::DrawDebugText({ 225, DISPLAY_HEIGHT - 25 }, textMin.c_str(), fps < 25 ? Play::cRed : Play::cWhite, false);
	Play::DrawDebugText({ 400, DISPLAY_HEIGHT - 25 }, textMax.c_str(), fps < 25 ? Play::cRed : Play::cWhite, false);

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

