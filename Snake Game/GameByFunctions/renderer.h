#ifndef RENDERER_H
#define RENDERER_H

#include "all.h"
#include "math.h"

typedef enum TileType
{
	TopLeft,
	Top,
	TopRight,
	Left,
	Middle,
	Right,
	BottomLeft,
	Bottom,
	BottomRight,

	TILE_TYPE_COUNT,
} TileType;

typedef struct TileMap
{
	SDL_Surface* tileMapSurface;
	SDL_Rect sourceRects[TILE_TYPE_COUNT];
} TileMap;

enum RendererType
{
	rDebug,
	rL1,

	RENDERERTYPE_COUNT,
};

typedef struct Animation
{
	SDL_Surface* surface;
	SDL_Rect* frames;
	int frameCount;
	int currentFrame;

	Uint32 nextFrameAt;
	Uint32 currentFrameTime;
	Uint32 animationSpeed;

} Animation;

typedef struct RPGAnimation
{
	Animation* walkUp;
	Animation* walkDown;
	Animation* walkLeft;
	Animation* walkRight;
} RPGAnimation;

typedef struct IdPair
{
	int id;
	v2 tilePosition;
} IdPair;

struct Renderer
{
	union
	{
		struct {
			RPGAnimation* animation;
			Animation* ani1UP;
			Animation* aniHeart;
			Animation* aniStar;
			Animation* aniLive;
			Animation* aniBack;
			Animation* aniWall;

			float framePercentage;
			Uint32 simulationTime;

			SDL_bool isInit;
			IdPair positionCache1[CONST_MAX_GAMEOBJECTS];
			IdPair positionCache2[CONST_MAX_GAMEOBJECTS];
		};
	} extra;

	drawCall* drawSnakeHead;
	drawCall* drawSnakeBody;
	drawCall* drawSnakeTail;

	drawCall* drawFruit;
	drawCall* drawWall;

	drawStateCall* drawLives;
	drawStateCall* drawScore;

	drawUpdate* update;

	TileMap tileMap;
	SDL_Surface* mainRenderTarget;
	SDL_bool drawGrid;

	TTF_Font* font;

	RendererType type;
};

Renderer* createRenderer(RendererType type);
void freeRenderer(Renderer* renderer);

void render(SDL_Surface* target, GameState* game, Renderer* renderer);

#endif 