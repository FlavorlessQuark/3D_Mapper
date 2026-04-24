#pragma once
#include <SDL2/SDL.h>

typedef struct 
{
    SDL_Window      *window;
    SDL_Renderer    *renderer;
    SDL_Texture     *background;
	SDL_Color		bgColor;

    int             win_w;
    int             win_h;
}   SDLX_Display;


typedef struct  {
    double x;
    double y;
} Vect2;

typedef struct Vect3 {
    double x;
    double y;
    double z;
} Vect3;

typedef struct  {
    double r;
    double angle_z;
    double angle_y;
} Vect3_Polar;

typedef struct  {
    Vect3 rotation;
    Vect3 position;
}   Camera;

typedef struct  {
    double mat [4][4];
    int n = 4;
    int m = 4;
}   Matrix;