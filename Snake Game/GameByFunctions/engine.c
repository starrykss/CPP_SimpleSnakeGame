#include "engine.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

void initEngine()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", SDL_GetError(), NULL);
	}
	else
	{
		if (TTF_Init() < 0)
		{
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", TTF_GetError(), NULL);
		}
		else
		{
			if (IMG_Init(IMG_INIT_PNG) < 0)
			{
				SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", IMG_GetError(), NULL);
			}
		}
	}
}

void quitEngine()
{
	IMG_Quit();
	TTF_Quit();
	SDL_Quit();
}