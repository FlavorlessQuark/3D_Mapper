#include <SDL2/SDL.h>
#include "../incl/visualizer.h"
#include <iostream>

/* 
    | z 
    |  / y 
    | / 
    |/_ _ _ _ _
             x

*/
void DrawCenteredRect(SDL_Renderer* renderer, float centerX, float centerY, float size) {
    SDL_FRect rect;
    
    // Calculate the top-left corner to center the square
    rect.x = centerX - (size / 2.0f);
    rect.y = centerY - (size / 2.0f);
    rect.w = size;
    rect.h = size;

    // Draw the filled rectangle
    SDL_RenderFillRectF(renderer, &rect);
}

int main(void) {
    SDL_Window *window;
    SDLX_Display *display;

    Vect3_Polar *p3D;
    Vect3 *pc3D = new Vect3[8];
    Vect2 *p2D = new Vect2[8];

    pc3D[0] = (Vect3){.x= 1, .y = 1, .z = 1};
    pc3D[1] = (Vect3){.x= 1, .y = 0, .z = 1};
    pc3D[2] = (Vect3){.x= 1, .y = 1, .z = 0};
    pc3D[3] = (Vect3){.x= 1, .y = 0, .z = 0};
    pc3D[4] = (Vect3){.x=  0, .y = 1, .z = 1};
    pc3D[5] = (Vect3){.x=  0, .y = 0, .z = 1};
    pc3D[6] = (Vect3){.x=  0, .y = 1, .z = 0};
    pc3D[7] = (Vect3){.x=  0, .y = 0, .z = 0};

    SDLX_InitDefault();

    display = SDLX_DisplayGet();
    SDL_SetRenderDrawColor(display->renderer, 0,100,0,255);

    while (1) {
        SDL_RenderClear(display->renderer);
        // handles camera controlls
		InputLoop();

        // color of points
        SDL_SetRenderDrawColor(display->renderer, 200,100,0,255);

        project(pc3D, p2D, 8);
        for (int i = 0; i < 8; i++) {

            std::cout << p2D[i].x << " " << p2D[i].y << std::endl;
            // SDL_RenderDrawPointF(
            //     display->renderer,
            //     (p2D[i].x + 1.0) * display->win_w,
            //     (p2D[i].y + 1.0) * display->win_h
            // );
            float screenX = (p2D[i].x + 1.0f) * display->win_w;
            float screenY = (p2D[i].y + 1.0f) * display->win_h;

            // Draw a 10x10 rectangle centered at (screenX, screenY)
            DrawCenteredRect(display->renderer, screenX, screenY, 10.0f);
        }

        // background color
        SDL_SetRenderDrawColor(display->renderer, 0,100,0,255);
        SDL_RenderPresent(display->renderer);
    }
}
