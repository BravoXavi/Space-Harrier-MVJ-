#include "Globals.h"
#include "Application.h"
#include "ModuleTime.h"
#include "SDL/include/SDL.h"

using namespace std;

ModuleTime::ModuleTime(bool start_enabled) : Module(start_enabled)
{}

// Destructor
ModuleTime::~ModuleTime()
{}

bool ModuleTime::Init()
{
	deltaTime = 0.0f;
	lastDeltaTime = 0.0f;

	return true;
}

update_status ModuleTime::PreUpdate()
{
	Uint32 tick = SDL_GetTicks();
	deltaTime = ((float)tick - lastDeltaTime) / 1000.0f;
	lastDeltaTime = (float)tick;

	return UPDATE_CONTINUE;
}

//Return the delta value to apply to speed values
const float ModuleTime::getDeltaTime() const
{
	return deltaTime;
}