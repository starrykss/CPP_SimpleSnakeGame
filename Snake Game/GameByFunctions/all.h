#include <SDL.h>
#include "SDL_ttf.h"
#include <SDL_image.h>
#include "constants.h"
#include "game.h"
#include "engine.h"
#include "platform.h"
#include "renderer.h"

#ifndef ALL_H
#define ALL_H

#define _CRT_SECURE_NO_WARNINGS

typedef struct PlatformState PlatformState;
typedef struct GameState GameState;

typedef struct SnakePart SnakePart;

typedef struct Fruit Fruit;
typedef enum FruitType FruitType;

typedef struct Wall Wall;
typedef enum WallType WallType;

typedef struct GameObject GameObject;

typedef enum GameMode GameMode;
typedef enum GameObjectType GameObjectType;

typedef struct Renderer Renderer;
typedef enum RendererType RendererType;
struct SDL_Surface;
typedef void drawCall(struct SDL_Surface* target, GameObject* gameObject, Renderer* rend);
typedef void drawStateCall(struct SDL_Surface* target, GameState* game, Renderer* rend);
typedef void drawUpdate(GameState* game, Renderer* rend);

#endif 
