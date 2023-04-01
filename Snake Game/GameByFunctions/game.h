#ifndef GAME_H
#define GAME_H

#include "all.h"
#include "math.h"

enum FruitType
{
	fruit1UP,
	fruitHeartContainer,
	fruitStar,

	FRUIT_TYPE_MAX,
};

enum WallType
{
	wallRock,

	WALL_TYPE_MAX,
};

enum GameMode
{
	modeRun,
	modeGameOver,
	modeClose,
};

enum GameObjectType
{
	typeSnakeHead = 1 << 0,
	typeSnakeTail = 1 << 1,
	typeSnakeBody = 1 << 2,
	typeFruit = 1 << 3,
	typeWall = 1 << 4,
	
	typeSnake = typeSnakeBody | typeSnakeHead | typeSnakeTail,
	typeCollidable = typeSnake | typeFruit | typeWall,
	typeAll = typeCollidable,
};

struct SnakePart
{
	Direction currentDirection;
	int nextPart;
	int prevPart;
};

struct Fruit
{
	FruitType type;
};

struct Wall
{
	WallType type;
};

struct GameObject
{
	union
	{
		SnakePart snakePart;
		Fruit fruit;
		Wall wall;
	};
	int id;
	v2 tilePosition;
	GameObjectType type;
};

struct GameState
{
	GameMode gameMode;
	Uint32 simulationTime;
	Uint32 updateInterval;
	int score;
	int scoreThisRound;
	int lives;
	Direction inputDirection;
	Direction lastDirection;

	int nextId;
	int activeGameObjects;
	GameObject gameObjects[CONST_MAX_GAMEOBJECTS];
};

GameState* createGame();
void freeGame(GameState* game);


GameObject* enumerateGameObjectByType(GameState* game, GameObject* iterator, GameObjectType typeFilter);
void inputDirection(GameState* game, Direction direction);

void update(GameState* game);
void restartGame(GameState* game);

void cheatAddLive(GameState* game, int amount);
void cheatAddSnakePart(GameState* game, int amount);
void cheatAddScore(GameState* game, int amount);

#endif 