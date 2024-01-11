#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"
#include "Simulation.h"

int DISPLAY_WIDTH = 640;
int DISPLAY_HEIGHT = 360;
int DISPLAY_SCALE = 2;

float dt = 0.016667f;

// The entry point for a PlayBuffer program
void MainGameEntry( PLAY_IGNORE_COMMAND_LINE )
{
	Play::CreateManager( DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE );
}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate( float elapsedTime )
{
	auto timeStart = std::chrono::steady_clock::now();
	Play::ClearDrawingBuffer( Play::cBlack );

	//Simulation
	Fluid::Simulation::getInstance().Update(dt);
	Play::DrawCircle();

	int fps = 1 / dt;
	std::string text = "fps: " + std::to_string(fps);
	std::string textdt = "DT: " + std::to_string(dt);

	Play::DrawDebugText({ 10, 10 }, text.c_str(), fps < 25 ? Play::cRed : Play::cWhite, false);
	Play::DrawDebugText( { 10, 25 }, textdt.c_str(), fps < 25 ? Play::cRed : Play::cWhite, false);

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

