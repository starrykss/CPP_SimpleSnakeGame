#ifndef PLATFORM_H
#define PLATFORM_H

#include "all.h"

struct PlatformState
{
	Renderer* currentRenderer;
	SDL_Event eventState;
	SDL_Window* window;
    SDL_Surface* windowSurface;
};

PlatformState* createPlatform(const char* windowTitle, int width, int height, Uint32 flags);
void freePlatform(PlatformState* platform);

void runMainLoop(GameState* game, PlatformState* platform);

#endif 