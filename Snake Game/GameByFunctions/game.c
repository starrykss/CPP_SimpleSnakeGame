#include "game.h"
#include <stdio.h>
#include <stdlib.h>

static v2 wrapTilePosition(v2 position);
static void eatFruit(GameState* game, GameObject* fruit);
static SDL_bool isGameObjectType(GameObject* gameObject, GameObjectType type);
static void endRound(GameState* game);
static GameObject* spawnFruit(GameState* game);

static SDL_bool onCollosion(GameState* game, GameObject* gameObject1, GameObject* gameObject2)
{
	if (gameObject1->type == typeSnakeHead && gameObject2->type == typeFruit)
	{
		eatFruit(game, gameObject2);
		cheatAddSnakePart(game, 1);
		spawnFruit(game);
		return SDL_TRUE;
	}
	else if ((gameObject1->type == typeSnakeHead && gameObject2->type == typeWall)
		|| (gameObject1->type == typeSnakeHead && isGameObjectType(gameObject2, typeSnake)))
	{
		endRound(game);
		return SDL_FALSE;
	}
	return SDL_FALSE;
}

static GameObject* acquireGameObject(GameState* game)
{
	++game->activeGameObjects;
	SDL_assert(game->activeGameObjects < CONST_MAX_GAMEOBJECTS);
	GameObject* resultAddress = game->gameObjects + (game->activeGameObjects - 1);
	SDL_memset(resultAddress, 0, sizeof(GameObject));
	resultAddress->id = game->nextId++;
	return resultAddress;

}
static void releaseGameObject(GameState* game, GameObject* gameObject)
{
	--game->activeGameObjects;
	SDL_assert(game->activeGameObjects >= 0);
	if (game->activeGameObjects > 0)
	{
		SDL_memcpy(gameObject, game->gameObjects + game->activeGameObjects, sizeof(GameObject));
	}
}
static GameObject* findGameObjectAt(GameState* game, v2 searchPosition, GameObjectType typeFilter)
{
	for (int i = 0; i < game->activeGameObjects; i++)
	{
		if ((game->gameObjects[i].type & typeFilter) != 0)
		{
			if (v2Equals(game->gameObjects[i].tilePosition, searchPosition))
			{
				return game->gameObjects + i;
			}
		}
	}
	return NULL;
}
static SDL_bool isGameObjectType(GameObject* gameObject, GameObjectType type)
{
	return (gameObject->type & type) != 0;
}
static int getGameObjectCountByType(GameState* game, GameObjectType typeFilter)
{
	int count = 0;
	for (int i = 0; i < game->activeGameObjects; i++)
	{
		if (isGameObjectType(game->gameObjects + i, typeFilter))
		{
			++count;
		}
	}
	return count;
}
GameObject* enumerateGameObjectByType(GameState* game, GameObject* iterator, GameObjectType typeFilter)
{
	GameObject* end = game->gameObjects + game->activeGameObjects;
	if (iterator == NULL)
	{
		iterator = game->gameObjects;
	}
	else
	{
		SDL_assert((game->gameObjects <= iterator) && (iterator < end));
		++iterator;
	}
	while (iterator != end)
	{
		if (isGameObjectType(iterator, typeFilter))
		{
			return iterator;
		}
		++iterator;
	}
	return NULL;
}
static SDL_bool moveGameObject(GameState* game, GameObject* gameObject, v2 newPosition)
{
	newPosition = wrapTilePosition(newPosition);
	GameObject* testObject = findGameObjectAt(game, newPosition, typeAll);
	if (testObject != NULL)
	{
		if (!onCollosion(game, gameObject, testObject))
		{
			return SDL_FALSE;
		}
	}
	gameObject->tilePosition = newPosition;
	return SDL_TRUE;
}
static GameObject* findGameObjectById(GameState* game, int id)
{
	for (int i = 0; i < game->activeGameObjects; i++)
	{
		if (game->gameObjects[i].id == id)
		{
			return game->gameObjects + i;
		}
	}
	return NULL;
}

static v2 directionToVector2(Direction direction)
{
	v2 result = { 0, 0 };
	switch (direction)
	{
		case DIR_UP:
			result.y = -1;
			break;
		case DIR_DOWN:
			result.y = 1;
			break;
		case DIR_LEFT:
			result.x = -1;
			break;
		case DIR_RIGHT:
			result.x = 1;
			break;
	}
	return result;
}
static Direction getOppsiteDirection(Direction direction)
{
	switch (direction)
	{
		case DIR_UP:
			direction = DIR_DOWN;
			break;
		case DIR_DOWN:
			direction = DIR_UP;
			break;
		case DIR_LEFT:
			direction = DIR_RIGHT;
			break;
		case DIR_RIGHT:
			direction = DIR_LEFT;
			break;
	}
	return direction;
}
static v2 wrapTilePosition(v2 position)
{
	while (position.x < 0)
	{
		position.x += CONST_SIZE_TILE_COUNTX;
	}
	position.x = position.x % CONST_SIZE_TILE_COUNTX;

	while (position.y < 0)
	{
		position.y += CONST_SIZE_TILE_COUNTY;
	}
	position.y = position.y % CONST_SIZE_TILE_COUNTY;

	return position;
}
static v2 getRandomTilePosition()
{
	return wrapTilePosition(createV2(rand(), rand()));
}
static v2 getRandomEmptyTilePosition(GameState* game)
{
	v2 result;
	do
	{
		result = getRandomTilePosition();
	} while (findGameObjectAt(game, result, typeCollidable) != NULL);
	return result;
}

static GameObject* getSnakeHead(GameState* game)
{
	for (int i = 0; i < game->activeGameObjects; i++)
	{
		if (game->gameObjects[i].type == typeSnakeHead)
		{
			return game->gameObjects + i;
		}
	}
	SDL_assert(SDL_FALSE);
	return NULL;
}
static GameObject* getSnakeTail(GameState* game)
{
	GameObject* currentSnakePart = getSnakeHead(game);
	while (findGameObjectById(game, currentSnakePart->snakePart.nextPart) != NULL)
	{
		currentSnakePart = findGameObjectById(game, currentSnakePart->snakePart.nextPart);
	}
	return currentSnakePart;
}
static GameObject* addSnakePart(GameState*	game, GameObject* gameObject)
{
	SDL_assert(isGameObjectType(gameObject, typeSnake));

	if (gameObject->type == typeSnakeTail)
	{
		gameObject->type = typeSnakeBody;
	}

	Direction oppsiteDirection = getOppsiteDirection(gameObject->snakePart.currentDirection);
	v2 tileOffset = directionToVector2(oppsiteDirection);

	GameObject* newTail = acquireGameObject(game);
	newTail->type = typeSnakeTail;
	newTail->tilePosition = v2Add(gameObject->tilePosition, tileOffset);
	newTail->snakePart.currentDirection = gameObject->snakePart.currentDirection;

	gameObject->snakePart.nextPart = newTail->id;
	newTail->snakePart.prevPart = gameObject->id;
	return newTail;
}
static GameObject* spawnSnake(GameState* game)
{
	GameObject* snakeHead = acquireGameObject(game);
	snakeHead->type = typeSnakeHead;
	snakeHead->tilePosition = createV2(CONST_INITIAL_SNAKE_TILEX, CONST_INITIAL_SNAKE_TILEY);
	snakeHead->snakePart.currentDirection = CONST_INITIAL_SNAKE_DIRECTION;

	GameObject* currentSnakePart = snakeHead;
	for (int i = 1; i < CONST_INITIAL_SNAKE_LENGTH; i++)
	{
		currentSnakePart = addSnakePart(game, currentSnakePart);
	}
	return snakeHead;
}
static void moveSnake(GameState* game, Direction moveDirection)
{
	GameObject* snakeHead = getSnakeHead(game);
	v2 tileOffset = directionToVector2(game->inputDirection);
	v2 oldPosition = snakeHead->tilePosition;
	v2 newPosition = v2Add(oldPosition, tileOffset);
	if (moveGameObject(game, snakeHead, newPosition))
	{
		Direction oldDirection = snakeHead->snakePart.currentDirection;
		Direction newDirection = moveDirection;
		snakeHead->snakePart.currentDirection = newDirection;
		GameObject* it = findGameObjectById(game, snakeHead->snakePart.nextPart);
		while (it != NULL)
		{
			newDirection = oldDirection;
			oldDirection = it->snakePart.currentDirection;
			it->snakePart.currentDirection = newDirection;
			newPosition = oldPosition;
			oldPosition = it->tilePosition;
			it->tilePosition = newPosition;
			it = findGameObjectById(game, it->snakePart.nextPart);
		}
	}
}
static int getSnakeCount(GameState* game)
{
	return getGameObjectCountByType(game, typeSnake);
}

static GameObject* spawnFruit(GameState* game)
{
	GameObject* newFruit = acquireGameObject(game);
	newFruit->type = typeFruit;
	newFruit->fruit.type = rand() % FRUIT_TYPE_MAX;
	newFruit->tilePosition = getRandomEmptyTilePosition(game);
	return newFruit;
}
static int getFruitScore(FruitType type)
{
	int resultScore = 0;
	switch (type)
	{
		case fruit1UP:
			resultScore = CONST_SCORE_FRUIT_1UP;
			break;
		case fruitHeartContainer:
			resultScore = CONST_SCORE_FRUIT_HEART_CONTAINER;
			break;
		case fruitStar:
			resultScore = CONST_SCORE_FRUIT_STAR;
			break;
		default:
			SDL_assert(SDL_FALSE);
			break;
	}
	return resultScore;
}
static void eatFruit(GameState* game, GameObject* gameObject)
{
	SDL_assert(isGameObjectType(gameObject, typeFruit));
	game->scoreThisRound += getFruitScore(gameObject->fruit.type);
	releaseGameObject(game, gameObject);
}
static int getFruitCount(GameState* game)
{
	return getGameObjectCountByType(game, typeFruit);
}

static GameObject* spawnWall(GameState* game)
{
	GameObject* newWall = acquireGameObject(game);
	newWall->type = typeWall;
	newWall->wall.type = rand() % WALL_TYPE_MAX;
	newWall->tilePosition = getRandomEmptyTilePosition(game);
	return newWall;
}
static int getWallCount(GameState* game)
{
	return getGameObjectCountByType(game, typeWall);
}

static void generateMap(GameState* game)
{
	for (int i = 0; i < CONST_INITIAL_FRUITS; i++)
	{
		spawnFruit(game);
	}
	for (int i = 0; i < CONST_INITIAL_WALLS; i++)
	{
		spawnWall(game);
	}
}
static void restartRound(GameState* game)
{
	SDL_memset(game->gameObjects, 0, sizeof(GameObject) * CONST_MAX_GAMEOBJECTS);
	game->activeGameObjects = 0;
	game->updateInterval = CONST_INITIAL_UPDATEINTERVAL;
	game->lastDirection = CONST_INITIAL_SNAKE_DIRECTION;
	game->inputDirection = CONST_INITIAL_SNAKE_DIRECTION;
	spawnSnake(game);
	generateMap(game);
}
void restartGame(GameState* game)
{
	game->gameMode = modeRun;
	game->simulationTime = SDL_GetTicks();
	game->lives = CONST_INITIAL_LIVES;
	game->score = CONST_INITIAL_SCORE;
	restartRound(game);
}
GameState* createGame()
{
	GameState* result = malloc(sizeof(GameState));
	SDL_memset(result, 0, sizeof(GameState));
	result->nextId = 1;
	restartGame(result);
	result->gameMode = modeGameOver;
	return result;
}
void freeGame(GameState* game)
{
	if (game != NULL)
	{
		free(game);
	}
}
static void onGameOver(GameState* game)
{
	game->gameMode = modeGameOver;
}
static void changeLives(GameState* game, int amount)
{
	game->lives += amount;
	if (game->lives > CONST_MAX_LIVES)
	{
		game->lives = CONST_MAX_LIVES;
	}
	else if (game->lives <= 0)
	{
		game->lives = 0;
		onGameOver(game);
	}
}
static void endRound(GameState* game)
{
	game->score += game->scoreThisRound;
	game->scoreThisRound = 0;
	changeLives(game, -1);
	restartRound(game);
}

void inputDirection(GameState* game, Direction direction)
{
	if (getOppsiteDirection(direction) != game->lastDirection)
	{
		game->inputDirection = direction;
	}
}
static void doUpdate(GameState* game)
{
	moveSnake(game, game->inputDirection);
	game->lastDirection = game->inputDirection;
}
void update(GameState* game)
{
	if (game->gameMode == modeRun)
	{
		Uint32 currentTime = SDL_GetTicks();
		while (game->simulationTime + game->updateInterval < (currentTime))
		{
			doUpdate(game);
			game->simulationTime += game->updateInterval;
		}
	}
}

void cheatAddLive(GameState* game, int amount)
{
	changeLives(game, amount);
}
void cheatAddSnakePart(GameState* game, int amount)
{
	for (int i = 0; i < amount; i++)
	{
		GameObject* snakeTail = getSnakeTail(game);
		addSnakePart(game, snakeTail);
	}
}
void cheatAddScore(GameState* game, int amount)
{
	game->scoreThisRound += amount;
}