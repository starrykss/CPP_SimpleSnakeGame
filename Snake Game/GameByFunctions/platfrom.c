#include "platform.h"

PlatformState* createPlatform(const char* windowTitle, int windowWidth, int windowHeight, Uint32 windowFlags)
{
	PlatformState* result = malloc(sizeof(PlatformState));
	SDL_memset(result, 0, sizeof(PlatformState));
	result->window = SDL_CreateWindow(windowTitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowWidth, windowHeight, windowFlags);
	if (result->window == NULL)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", SDL_GetError(), NULL);
	}
	else
	{
		result->windowSurface = SDL_GetWindowSurface(result->window);
	}

	result->currentRenderer = createRenderer(rDebug);
	return result;
}
void freePlatform(PlatformState* platform)
{
	if (platform != NULL)
	{
		if (platform->window != NULL)
		{
			SDL_DestroyWindow(platform->window);
		}
		if (platform->currentRenderer != NULL)
		{
			freeRenderer(platform->currentRenderer);
		}
		free(platform);
	}
}

static void toggleRenderer(PlatformState* platform)
{
	int rType = platform->currentRenderer->type;
	freeRenderer(platform->currentRenderer);
	platform->currentRenderer = createRenderer((rType + 1) % RENDERERTYPE_COUNT);
}

void handleKeyDown(GameState* game, PlatformState* platform)
{
	switch (platform->eventState.key.keysym.sym)
	{
		case SDLK_ESCAPE:
			game->gameMode = modeClose;
			return;
			break;

		case SDLK_c:
			toggleRenderer(platform);
			return;
			break;

		case SDLK_g:
			platform->currentRenderer->drawGrid = !platform->currentRenderer->drawGrid;
			return;
			break;
	}


	switch (game->gameMode)
	{
		case modeGameOver:
			restartGame(game);
			break;
		case modeRun:
			switch (platform->eventState.key.keysym.sym)
			{
				case SDLK_UP:
					inputDirection(game, DIR_UP);
					break;
				case SDLK_DOWN:
					inputDirection(game, DIR_DOWN);
					break;
				case SDLK_LEFT:
					inputDirection(game, DIR_LEFT);
					break;
				case SDLK_RIGHT:
					inputDirection(game, DIR_RIGHT);
					break;
			}
			break;
	}
}

void handleEvent(GameState* game, PlatformState* platform)
{

	switch (platform->eventState.type)
	{
		case SDL_QUIT:
			game->gameMode = modeClose;
			break;
		case SDL_WINDOWEVENT:
			if (platform->eventState.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
			{
				SDL_FreeSurface(platform->windowSurface);
				platform->windowSurface = SDL_GetWindowSurface(platform->window);
			}
			break;
		case SDL_KEYDOWN:
			handleKeyDown(game, platform);
			break;
	}
}

void runMainLoop(GameState* game, PlatformState* platform)
{
	while (game->gameMode != modeClose)
	{
		while (SDL_PollEvent(&platform->eventState))
		{
			handleEvent(game, platform);
		}

		update(game);
		render(platform->windowSurface, game, platform->currentRenderer);

		SDL_UpdateWindowSurface(platform->window);
	}
}