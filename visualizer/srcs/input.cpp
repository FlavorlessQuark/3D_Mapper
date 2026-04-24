#include <SDL2/SDL.h>
#include "../incl/visualizer.h"

// // Global Camera Variables
// Vect3 eye    = {0.0f, 0.0f, 5.0f};  // Starting position
// Vect3 target = {0.0f, 0.0f, 0.0f};  // Looking at the origin
// Vect3 up     = {0.0f, 1.0f, 0.0f};  // Up direction

// float moveSpeed = 0.1f;

// void InputLoop(void)
// {
//     SDL_Event event;
//     while (SDL_PollEvent(&event))
//     {
//         if (event.type == SDL_QUIT) exit(0);
//     }

//     // Get current keyboard state for smooth movement
//     const Uint8 *state = SDL_GetKeyboardState(NULL);

//     if (state[SDL_SCANCODE_ESCAPE]) exit(0);

//     // Forward / Backward (Z-axis)
//     if (state[SDL_SCANCODE_W]) {
//         eye.z -= moveSpeed;
//         target.z -= moveSpeed;
//     }
//     if (state[SDL_SCANCODE_S]) {
//         eye.z += moveSpeed;
//         target.z += moveSpeed;
//     }

//     // Left / Right (X-axis)
//     if (state[SDL_SCANCODE_A]) {
//         eye.x -= moveSpeed;
//         target.x -= moveSpeed;
//     }
//     if (state[SDL_SCANCODE_D]) {
//         eye.x += moveSpeed;
//         target.x += moveSpeed;
//     }

//     // Up / Down (Y-axis)
//     if (state[SDL_SCANCODE_SPACE]) {
//         eye.y += moveSpeed;
//         target.y += moveSpeed;
//     }
//     if (state[SDL_SCANCODE_LSHIFT]) {
//         eye.y -= moveSpeed;
//         target.y -= moveSpeed;
//     }

//     // Apply the changes to your matrix
//     Update_Camera(eye, target, up);
// }


// Orbit State
Vect3 target = {0.0f, 0.0f, 0.0f};  // The point we orbit
float distance = 5.0f;             // Radius
float azimuth = 1.57f;             // Horizontal angle (radians)
float polar = 1.57f;               // Vertical angle (radians)

float moveSpeed = 0.05f;           // Speed of target movement
float orbitSpeed = 0.01f;          // Speed of rotation
float zoomSpeed = 0.1f;            // Speed of distance change

void InputLoop(void)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT) exit(0);
    }

    const Uint8 *state = SDL_GetKeyboardState(NULL);
    if (state[SDL_SCANCODE_ESCAPE]) exit(0);

    // --- ARROW KEYS: Move the Orbit Target ---
    if (state[SDL_SCANCODE_UP])    target.z -= moveSpeed;
    if (state[SDL_SCANCODE_DOWN])  target.z += moveSpeed;
    if (state[SDL_SCANCODE_LEFT])  target.x -= moveSpeed;
    if (state[SDL_SCANCODE_RIGHT]) target.x += moveSpeed;

    // --- WASD: Rotate the Camera Angle ---
    if (state[SDL_SCANCODE_A]) azimuth -= orbitSpeed;
    if (state[SDL_SCANCODE_D]) azimuth += orbitSpeed;
    if (state[SDL_SCANCODE_W]) polar -= orbitSpeed;
    if (state[SDL_SCANCODE_S]) polar += orbitSpeed;

    // Clamp polar angle to prevent the camera from flipping over the top (Gimbal Lock-ish)
    if (polar < 0.1f) polar = 0.1f;
    if (polar > M_PI - 0.1f) polar = M_PI - 0.1f;

    // --- SPACE / TAB: Zoom Distance ---
    if (state[SDL_SCANCODE_SPACE]) distance -= zoomSpeed;
    if (state[SDL_SCANCODE_TAB])   distance += zoomSpeed;
    if (distance < 1.0f) distance = 1.0f; // Prevent going inside the target

    // --- Calculate New Eye Position ---
    Vect3 eye;
    eye.x = target.x + distance * sinf(polar) * cosf(azimuth);
    eye.y = target.y + distance * cosf(polar);
    eye.z = target.z + distance * sinf(polar) * sinf(azimuth);

    Vect3 up = {0.0f, 1.0f, 0.0f};
    Update_Camera(eye, target, up);
}