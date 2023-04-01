#include "renderer.h"
#include <stdio.h>

static void FillRect(SDL_Surface* target, int x, int y, int w, int h, Uint32 color)
{
	SDL_Rect fillRect;
	fillRect.x = x;
	fillRect.y = y;
	fillRect.w = w;
	fillRect.h = h;
	SDL_FillRect(target, &fillRect, color);
}
static void FillRectTilePosition(SDL_Surface* target, v2 tilePosition, int w, int h, Uint32 color)
{
	FillRect(target, tilePosition.x * CONST_SIZE_TILE_WIDTH, tilePosition.y * CONST_SIZE_TILE_HEIGHT, w, h, color);
}
static v2 tilePositionToPixelPosition(v2 tilePosition)
{
	tilePosition.x *= CONST_SIZE_TILE_WIDTH;
	tilePosition.y *= CONST_SIZE_TILE_HEIGHT;
	return tilePosition;
}
static SDL_Rect createRect(int x, int y, int w, int h)
{
	SDL_Rect result;
	result.x = x;
	result.y = y;
	result.w = w;
	result.h = h;
	return result;
}
static TileType getTileTypeFromPosition(int tileX, int tileY, int maxX, int maxY)
{
	maxX -= 1;
	maxY -= 1;

	if (tileX == 0 && tileY == 0)
	{
		return TopLeft;
	}
	else if (tileX == 0 && tileY == maxY)
	{
		return BottomLeft;
	}

	else if (tileY == 0 && tileX == maxX)
	{
		return TopRight;
	}
	else if (tileY == maxY && tileX == maxX)
	{
		return BottomRight;
	}
	else if (tileY == maxY)
	{
		return Bottom;
	}
	else if (tileX == maxX)
	{
		return Right;
	}
	else if (tileX == 0)
	{
		return Left;
	}
	else if (tileY == 0)
	{
		return Top;
	}
	else
	{
		return Middle;
	}

	SDL_assert(SDL_FALSE);
	return 0;
}

static void drawTileFromTileMap(SDL_Surface* target, TileMap* tileMap, TileType type, int posX, int posY)
{
	SDL_assert(type < TILE_TYPE_COUNT);

	SDL_Rect destRect;
	destRect.x = posX;
	destRect.y = posY;
	destRect.w = CONST_SIZE_TILE_WIDTH;
	destRect.h = CONST_SIZE_TILE_HEIGHT;

	SDL_BlitScaled(tileMap->tileMapSurface, tileMap->sourceRects + type, target, &destRect);
}
static void drawTileMap(SDL_Surface* target, Renderer* rend)
{
	for (int tileX = 0; tileX < CONST_SIZE_TILE_COUNTX; tileX++)
	{
		for (int tileY = 0; tileY < CONST_SIZE_TILE_COUNTY; tileY++)
		{
			int pixelX = tileX * CONST_SIZE_TILE_WIDTH;
			int pixelY = tileY * CONST_SIZE_TILE_HEIGHT;

			TileType tileType = getTileTypeFromPosition(tileX, tileY, CONST_SIZE_TILE_COUNTX, CONST_SIZE_TILE_COUNTY);
			drawTileFromTileMap(target, &rend->tileMap, tileType, pixelX, pixelY);
		}
	}
}
static TileMap createColorTileMap(Uint32 color)
{
	TileMap result;
	result.tileMapSurface = SDL_CreateRGBSurface(0, CONST_SIZE_TILE_WIDTH, CONST_SIZE_TILE_HEIGHT, 32, 0, 0, 0, 0);
	SDL_Rect rect = { 0, 0, CONST_SIZE_TILE_WIDTH, CONST_SIZE_TILE_HEIGHT };
	for (int i = 0; i < TILE_TYPE_COUNT; i++)
	{
		result.sourceRects[i] = rect;
	}
	SDL_FillRect(result.tileMapSurface, NULL, color);
	return result;
}
static void freeTileMap(TileMap* tileMap)
{
	if (tileMap != NULL)
	{
		SDL_FreeSurface(tileMap->tileMapSurface);
	}
}

static void drawAnimation(SDL_Surface* target, Animation* animation, v2 targetPosition)
{
	SDL_Rect destRect;
	destRect.x = targetPosition.x;
	destRect.y = targetPosition.y;
	SDL_BlitSurface(animation->surface, animation->frames + animation->currentFrame, target, &destRect);
}
static void drawAnimationTilePosition(SDL_Surface* target, Animation* animation, v2 tilePosition)
{
	v2 targetPosition = tilePositionToPixelPosition(tilePosition);
	drawAnimation(target, animation, targetPosition);
}
static void drawAnimationPivotBottomCenter(SDL_Surface* target, Animation* animation, v2 targetPosition)
{
	v2 rectOffset;
	rectOffset.x = -animation->frames[animation->currentFrame].w / 2;
	rectOffset.y = -animation->frames[animation->currentFrame].h;
	targetPosition = v2Add(targetPosition, rectOffset);
	drawAnimation(target, animation, targetPosition);
}
static void updateAnimation(Animation* animation, Uint32 currentTime)
{
	while (currentTime >= animation->nextFrameAt)
	{
		animation->nextFrameAt += animation->animationSpeed;
		animation->currentFrame = (1 + animation->currentFrame) % animation->frameCount;
	}
	animation->currentFrameTime = currentTime;
}
static void updateRPGAnimation(RPGAnimation* animation, Uint32 currentTime)
{
	updateAnimation(animation->walkDown, currentTime);
	updateAnimation(animation->walkUp, currentTime);
	updateAnimation(animation->walkLeft, currentTime);
	updateAnimation(animation->walkRight, currentTime);
}

static Animation* createAnimation(SDL_Surface* surface, int frameCount, Uint32 animationSpeed)
{
	Animation* result = malloc(sizeof(Animation));
	SDL_memset(result, 0, sizeof(Animation));
	result->surface = surface;
	result->frameCount = frameCount;
	result->frames = malloc(sizeof(SDL_Rect) * frameCount);
	SDL_memset(result->frames, 0, sizeof(SDL_Rect) * frameCount);
	result->animationSpeed = animationSpeed;
	return result;
}
static Animation* createSingleFrameAnimation(const char* fileName)
{
	SDL_Surface* surface = IMG_Load(fileName);
	if (surface == NULL)
	{
		SDL_ShowSimpleMessageBox(0, "Error", IMG_GetError(), 0);
	}
	Animation* result = createAnimation(surface, 1, 0);
	result->frames[0] = createRect(0, 0, surface->w, surface->h);
	return result;
}
static void freeAnimation(Animation* animation)
{
	if (animation != NULL)
	{
		if (animation->frames != NULL)
		{
			free(animation->frames);
		}

		free(animation);
	}
}
static void freeAnimationAndSurface(Animation* animation)
{
	if (animation != NULL)
	{
		if (animation->surface != NULL)
		{
			SDL_FreeSurface(animation->surface);
		}
		freeAnimation(animation);
	}
}
static RPGAnimation* createRPGAnimation(const char* fileName, Uint32 animationSpeed)
{
	RPGAnimation* result = malloc(sizeof(RPGAnimation));
	SDL_Surface* surface = IMG_Load(fileName);
	if (surface == NULL)
	{
		SDL_ShowSimpleMessageBox(0, "Error", IMG_GetError(), 0);
	}
	else
	{
		result->walkUp = createAnimation(surface, 4, animationSpeed);
		result->walkUp->frames[0] = createRect(32, 96, 32, 32);
		result->walkUp->frames[1] = createRect(0, 96, 32, 32);
		result->walkUp->frames[2] = createRect(32, 96, 32, 32);
		result->walkUp->frames[3] = createRect(64, 96, 32, 32);

		result->walkDown = createAnimation(surface, 4, animationSpeed);
		result->walkDown->frames[0] = createRect(32, 0, 32, 32);
		result->walkDown->frames[1] = createRect(0, 0, 32, 32);
		result->walkDown->frames[2] = createRect(32, 0, 32, 32);
		result->walkDown->frames[3] = createRect(64, 0, 32, 32);

		result->walkLeft = createAnimation(surface, 4, animationSpeed);
		result->walkLeft->frames[0] = createRect(32, 32, 32, 32);
		result->walkLeft->frames[1] = createRect(0, 32, 32, 32);
		result->walkLeft->frames[2] = createRect(32, 32, 32, 32);
		result->walkLeft->frames[3] = createRect(64, 32, 32, 32);

		result->walkRight = createAnimation(surface, 4, animationSpeed);
		result->walkRight->frames[0] = createRect(32, 64, 32, 32);
		result->walkRight->frames[1] = createRect(0, 64, 32, 32);
		result->walkRight->frames[2] = createRect(32, 64, 32, 32);
		result->walkRight->frames[3] = createRect(64, 64, 32, 32);
	}

	return result;
}
static void freeRPGAnimation(RPGAnimation* animation)
{
	if (animation != NULL)
	{
		freeAnimationAndSurface(animation->walkUp);
		freeAnimation(animation->walkDown);
		freeAnimation(animation->walkLeft);
		freeAnimation(animation->walkRight);
		free(animation);
	}
}

static void drawGrid(SDL_Surface* target, Uint32 color, int w, int h)
{
	int countX = 1 + target->w / w;
	int countY = 1 + target->h / h;
	SDL_Rect lineRect;

	for (int i = 0; i < countX; i++)
	{
		lineRect.x = w * i;
		lineRect.y = 0;
		lineRect.w = 1;
		lineRect.h = target->h;

		SDL_FillRect(target, &lineRect, color);
	}
	for (int i = 0; i < countY; i++)
	{
		lineRect.x = 1;
		lineRect.y = h * i;
		lineRect.w = target->w;
		lineRect.h = 1;

		SDL_FillRect(target, &lineRect, color);
	}
}

static void drawGameObject(SDL_Surface* target, GameObject* gameObject, Renderer* rend)
{
	switch (gameObject->type)
	{
		case typeSnakeHead:
			rend->drawSnakeHead(target, gameObject, rend);
			break;
		case typeSnakeBody:
			rend->drawSnakeBody(target, gameObject, rend);
			break;
		case typeSnakeTail:
			rend->drawSnakeTail(target, gameObject, rend);
			break;
		case typeFruit:
			rend->drawFruit(target, gameObject, rend);
			break;
		case typeWall:
			rend->drawWall(target, gameObject, rend);
			break;
	}
}
static void drawGameObjectByType(SDL_Surface* target, GameState* game, Renderer* rend, GameObjectType typeFilter)
{
	GameObject* it = enumerateGameObjectByType(game, NULL, typeFilter);
	while (it != NULL)
	{
		drawGameObject(target, it, rend);
		it = enumerateGameObjectByType(game, it, typeFilter);
	}
}

static void drawSnake(SDL_Surface* target, GameState* game, Renderer* rend)
{
	drawGameObjectByType(target, game, rend, typeSnake);
}
static void drawFruit(SDL_Surface* target, GameState* game, Renderer* rend)
{
	drawGameObjectByType(target, game, rend, typeFruit);
}
static void drawWall(SDL_Surface* target, GameState* game, Renderer* rend)
{
	drawGameObjectByType(target, game, rend, typeWall);
}
static void drawLives(SDL_Surface* target, GameState* game, Renderer* rend)
{
	rend->drawLives(target, game, rend);
}
static void drawScore(SDL_Surface* target, GameState* game, Renderer* rend)
{
	rend->drawScore(target, game, rend);
}


static void debugDrawSnakeHead(SDL_Surface* target, GameObject* gameObject, Renderer* rend)
{
	FillRectTilePosition(target, gameObject->tilePosition, CONST_SIZE_TILE_WIDTH, CONST_SIZE_TILE_HEIGHT, CONST_COLOR_DEBUG_SNAKE_HEAD);
}
static void debugDrawSnakeBody(SDL_Surface* target, GameObject* gameObject, Renderer* rend)
{
	FillRectTilePosition(target, gameObject->tilePosition, CONST_SIZE_TILE_WIDTH, CONST_SIZE_TILE_HEIGHT, CONST_COLOR_DEBUG_SNAKE_BODY);
}
static void debugDrawSnakeTail(SDL_Surface* target, GameObject* gameObject, Renderer* rend)
{
	FillRectTilePosition(target, gameObject->tilePosition, CONST_SIZE_TILE_WIDTH, CONST_SIZE_TILE_HEIGHT, CONST_COLOR_DEBUG_SNAKE_TAIL);
}
static void debugDrawWall(SDL_Surface* target, GameObject* gameObject, Renderer* rend)
{
	FillRectTilePosition(target, gameObject->tilePosition, CONST_SIZE_TILE_WIDTH, CONST_SIZE_TILE_HEIGHT, CONST_COLOR_DEBUG_WALL);
}
static void debugDrawFruit(SDL_Surface* target, GameObject* gameObject, Renderer* rend)
{
	FillRectTilePosition(target, gameObject->tilePosition, CONST_SIZE_TILE_WIDTH, CONST_SIZE_TILE_HEIGHT, CONST_COLOR_DEBUG_FRUIT);
}

static void debugDrawLives(SDL_Surface* target, GameState* game, Renderer* rend)
{
	v2 basePosition = createV2(CONST_SIZE_LIVE_MARGIN_LEFT, CONST_SIZE_LIVE_MARGIN_TOP);
	for (int i = 0; i < game->lives; i++)
	{
		FillRect(target, basePosition.x, basePosition.y, CONST_SIZE_TILE_WIDTH, CONST_SIZE_TILE_HEIGHT, CONST_COLOR_DEBUG_HEART);
		basePosition = v2Add(basePosition, createV2(CONST_SIZE_TILE_WIDTH + CONST_SIZE_LIVE_MARGIN_LEFT, 0));
	}
}
static void debugDrawScore(SDL_Surface* target, GameState* game, Renderer* rend)
{

	char scoreText[CONST_MAX_SCORE_TEXT_LENGTH];
	sprintf(scoreText, "Score: %20d", game->score + game->scoreThisRound);
	SDL_Color color = TO_COLOR(CONST_COLOR_SCORE_TEXT);
	SDL_Surface* scoreSurface = TTF_RenderText_Solid(rend->font, scoreText, color);
	if (scoreSurface != NULL)
	{
		SDL_Rect destRect;
		destRect.x = target->w - (CONST_SIZE_SCORE_MARGIN_RIGHT + scoreSurface->w);
		destRect.y = CONST_SIZE_SCORE_MARGIN_TOP;
		destRect.w = scoreSurface->w;
		destRect.h = scoreSurface->h;

		SDL_BlitSurface(scoreSurface, NULL, target, &destRect);
	}
	SDL_FreeSurface(scoreSurface);
}

static void debugUpdate(GameState* game, Renderer* rend)
{
}

static v2* getTilePositionFromRenderState(int id, Renderer* rend)
{
	for (int i = 0; i < CONST_MAX_GAMEOBJECTS; i++)
	{
		if (rend->extra.positionCache1[i].id == id)
		{
			return &rend->extra.positionCache1[i].tilePosition;
		}
	}
	return NULL;
}
static void updateRenderState(GameState* game, Renderer* rend)
{
	SDL_memcpy(rend->extra.positionCache1, rend->extra.positionCache2, sizeof(IdPair) * CONST_MAX_GAMEOBJECTS);
	SDL_memset(rend->extra.positionCache2, 0, sizeof(IdPair) * CONST_MAX_GAMEOBJECTS);
	for (int i = 0; i < game->activeGameObjects; i++)
	{
		rend->extra.positionCache2[i].id = game->gameObjects[i].id;
		rend->extra.positionCache2[i].tilePosition = game->gameObjects[i].tilePosition;
	}
}


static void L1DrawSnake(SDL_Surface* target, GameObject* gameObject, Renderer* rend)
{
	Animation* animation = NULL;
	switch (gameObject->snakePart.currentDirection)
	{
		case DIR_UP:
			animation = rend->extra.animation->walkUp;
			break;
		case DIR_DOWN:
			animation = rend->extra.animation->walkDown;
			break;
		case DIR_LEFT:
			animation = rend->extra.animation->walkLeft;
			break;
		case DIR_RIGHT:
			animation = rend->extra.animation->walkRight;
			break;
	}

	v2 pixelPosition = tilePositionToPixelPosition(gameObject->tilePosition);
	v2* oldTilePosition = getTilePositionFromRenderState(gameObject->id, rend);
	if (oldTilePosition == NULL)
	{
		drawAnimationTilePosition(target, animation, gameObject->tilePosition);
	}
	else
	{
		v2 oldPixelPos = tilePositionToPixelPosition(*oldTilePosition);
		v2 currentPosition = v2Lerp(oldPixelPos, pixelPosition, rend->extra.framePercentage);
		drawAnimation(target, animation, currentPosition);
	}
}
static void L1DrawFruit(SDL_Surface* target, GameObject* gameObject, Renderer* rend)
{
	Animation* animation = NULL;
	switch (gameObject->fruit.type)
	{
		case fruit1UP:
			animation = rend->extra.ani1UP;
			break;
		case fruitHeartContainer:
			animation = rend->extra.aniHeart;
			break;
		case fruitStar:
			animation = rend->extra.aniStar;
			break;
	}
	drawAnimationTilePosition(target, animation, gameObject->tilePosition);
}
static void L1DrawWall(SDL_Surface* target, GameObject* gameObject, Renderer* rend)
{
	drawAnimationTilePosition(target, rend->extra.aniWall, gameObject->tilePosition);
}
static void L1DrawLives(SDL_Surface* target, GameState* game, Renderer* rend)
{
	v2 basePosition = createV2(CONST_SIZE_LIVE_MARGIN_LEFT, CONST_SIZE_LIVE_MARGIN_TOP);
	for (int i = 0; i < game->lives; i++) 
	{
		drawAnimation(target, rend->extra.aniLive, basePosition);
		basePosition = v2Add(basePosition, createV2(CONST_SIZE_TILE_WIDTH + CONST_SIZE_LIVE_MARGIN_LEFT, 0));
	}
}

static void L1Update(GameState* game, Renderer* rend)
{
	Uint32 currentTime = SDL_GetTicks();
	updateRPGAnimation(rend->extra.animation, currentTime);
	updateAnimation(rend->extra.aniWall, currentTime);

	rend->extra.framePercentage = (float)(currentTime - game->simulationTime) / (float)game->updateInterval;

	if (rend->extra.simulationTime != game->simulationTime)
	{
		updateRenderState(game, rend);
		rend->extra.simulationTime = game->simulationTime;
	}
}


static void createDebug(Renderer* rend)
{
	rend->drawSnakeHead = debugDrawSnakeHead;
	rend->drawSnakeBody = debugDrawSnakeBody;
	rend->drawSnakeTail = debugDrawSnakeTail;
	rend->drawFruit = debugDrawFruit;
	rend->drawWall = debugDrawWall;

	rend->drawLives = debugDrawLives;
	rend->drawScore = debugDrawScore;

	rend->update = debugUpdate;

	rend->tileMap = createColorTileMap(CONST_COLOR_DEBUG_TILE);
}
static void freeDebug(Renderer* rend)
{
	freeTileMap(&rend->tileMap);
}

static void createL1(Renderer* rend)
{
	createDebug(rend);

	rend->drawSnakeHead = L1DrawSnake;
	rend->drawSnakeBody = L1DrawSnake;
	rend->drawSnakeTail = L1DrawSnake;
	rend->drawFruit = L1DrawFruit;
	rend->drawLives = L1DrawLives;
	rend->drawWall = L1DrawWall;

	rend->update = L1Update;

	rend->extra.animation = createRPGAnimation("Resources/char.png", 150);
	rend->extra.ani1UP = createSingleFrameAnimation("Resources/1up.png");
	rend->extra.aniHeart = createSingleFrameAnimation("Resources/heartContainer.png");
	rend->extra.aniStar = createSingleFrameAnimation("Resources/star.png");
	rend->extra.aniLive = createSingleFrameAnimation("Resources/live.png");
	rend->extra.aniBack = createSingleFrameAnimation("Resources/background.png");

	freeTileMap(&rend->tileMap);
	rend->tileMap.tileMapSurface = IMG_Load("Resources/tileset.png");
	rend->tileMap.sourceRects[TopLeft] = createRect(0, 0, 32, 32);
	rend->tileMap.sourceRects[Top] = createRect(33, 0, 31, 32);
	rend->tileMap.sourceRects[TopRight] = createRect(66, 0, 32, 32);

	rend->tileMap.sourceRects[Left] = createRect(0, 33, 32, 32);
	rend->tileMap.sourceRects[Middle] = createRect(33, 33, 32, 32);
	rend->tileMap.sourceRects[Right] = createRect(66, 33, 32, 32);

	rend->tileMap.sourceRects[BottomLeft] = createRect(0, 66, 32, 32);
	rend->tileMap.sourceRects[Bottom] = createRect(33, 66, 32, 32);
	rend->tileMap.sourceRects[BottomRight] = createRect(66, 66, 32, 32);

	SDL_Surface* wallSurface = IMG_Load("Resources/boo.png");
	rend->extra.aniWall = createAnimation(wallSurface, 4, 100);
	rend->extra.aniWall->frames[0] = createRect(0, 0, 32, 33);
	rend->extra.aniWall->frames[1] = createRect(32, 0, 32, 33);
	rend->extra.aniWall->frames[2] = createRect(64, 0, 32, 33);
	rend->extra.aniWall->frames[3] = createRect(32, 0, 32, 33);
}
static void freeL1(Renderer* rend)
{
	if (rend != NULL)
	{
		freeDebug(rend);

		if (rend->extra.animation != NULL)
		{
			freeRPGAnimation(rend->extra.animation);
		}
		if (rend->extra.ani1UP != NULL)
		{
			freeAnimationAndSurface(rend->extra.ani1UP);
		}
		if (rend->extra.aniHeart != NULL)
		{
			freeAnimationAndSurface(rend->extra.aniHeart);
		}
		if (rend->extra.aniStar != NULL)
		{
			freeAnimationAndSurface(rend->extra.aniStar);
		}
		if (rend->extra.aniLive != NULL)
		{
			freeAnimationAndSurface(rend->extra.aniLive);
		}
		if (rend->extra.aniBack != NULL)
		{
			freeAnimationAndSurface(rend->extra.aniBack);
		}
		if (rend->extra.aniWall != NULL)
		{
			freeAnimationAndSurface(rend->extra.aniWall);
		}
	}
}


void render(SDL_Surface* target, GameState* game, Renderer* rend)
{
	rend->update(game, rend);

	SDL_FillRect(target, NULL, 0);
	if (rend->type == rL1)
	{
		SDL_BlitScaled(rend->extra.aniBack->surface, NULL, target, NULL);
	}
	if (game->gameMode == modeGameOver)
	{
		char scoreText[CONST_MAX_SCORE_TEXT_LENGTH];
		sprintf(scoreText, "Score: %20d", game->score + game->scoreThisRound);
		SDL_Color color = TO_COLOR(CONST_COLOR_SCORE_TEXT);
		SDL_Surface* scoreSurface = TTF_RenderText_Solid(rend->font, scoreText, color);
		sprintf(scoreText, "Press Any Key to start (G:Grid,C:CG)");
		SDL_Surface* continueSurface = TTF_RenderText_Solid(rend->font, scoreText, color);

		SDL_Rect center;
		center.x = (target->w / 2) - (continueSurface->w / 2);
		center.y = (target->h / 2) - (continueSurface->h / 2);
		center.w = continueSurface->w;
		center.h = continueSurface->h;

		SDL_BlitSurface(continueSurface, NULL, target, &center);
		center.y -= scoreSurface->h;
		center.h = scoreSurface->h;
		SDL_BlitSurface(scoreSurface, NULL, target, &center);

		SDL_FreeSurface(scoreSurface);
		SDL_FreeSurface(continueSurface);
	}
	else if (game->gameMode == modeRun)
	{
		SDL_Rect mainToCenter;
		float aspectWidth = (float)CONST_SIZE_GRID_WIDTH / (float)CONST_SIZE_WINDOW_WIDTH;
		float aspectHeight = (float)CONST_SIZE_GRID_HEIGHT / (float)CONST_SIZE_WINDOW_HEIGHT;
		int mainWidth = (int)(target->w * aspectWidth);
		int mainHeight = (int)(target->h * aspectHeight);

		mainToCenter.x = (target->w / 2) - (mainWidth / 2);
		mainToCenter.y = (target->h / 2) - (mainHeight / 2);
		mainToCenter.w = mainWidth;
		mainToCenter.h = mainHeight;

		drawTileMap(rend->mainRenderTarget, rend);
		drawFruit(rend->mainRenderTarget, game, rend);
		drawWall(rend->mainRenderTarget, game, rend);
		drawSnake(rend->mainRenderTarget, game, rend);
		drawLives(target, game, rend);
		drawScore(target, game, rend);

		if (rend->drawGrid)
		{
			drawGrid(rend->mainRenderTarget, 0, CONST_SIZE_TILE_WIDTH, CONST_SIZE_TILE_HEIGHT);
		}
		SDL_BlitScaled(rend->mainRenderTarget, NULL, target, &mainToCenter);
	}
}
Renderer* createRenderer(RendererType type)
{
	Renderer* result = malloc(sizeof(Renderer));
	SDL_memset(result, 0, sizeof(Renderer));
	result->type = type;
	result->drawGrid = SDL_FALSE;
	result->mainRenderTarget = SDL_CreateRGBSurface(0, CONST_SIZE_GRID_WIDTH, CONST_SIZE_GRID_HEIGHT, 32, 0, 0, 0, 0);
	result->font = TTF_OpenFont("Resources/font.ttf", 46);
	if (result->font == NULL)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", TTF_GetError(), NULL);
	}

	switch (type)
	{
		case rDebug:
			createDebug(result);
			break;
		case rL1:
			createL1(result);
			break;
	}

	return result;
}
void freeRenderer(Renderer* rend)
{
	if (rend != NULL)
	{
		switch (rend->type)
		{
			case rDebug:
				freeDebug(rend);
				break;
			case rL1:
				freeL1(rend);
				break;
		}
		if (rend->mainRenderTarget != NULL)
		{
			SDL_FreeSurface(rend->mainRenderTarget);
		}
		if (rend->font != NULL)
		{
			TTF_CloseFont(rend->font);
		}
		free(rend);
	}
}