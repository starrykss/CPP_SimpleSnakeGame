#include "all.h"

int main()
{
	initEngine();
	PlatformState* platformState = createPlatform("Snake Game", CONST_SIZE_WINDOW_WIDTH, CONST_SIZE_WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
	GameState* gameState = createGame();

	runMainLoop(gameState, platformState);

	freeGame(gameState);
	freePlatform(platformState);
	quitEngine();

    return 0;
}