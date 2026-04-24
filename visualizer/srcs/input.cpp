#include <SDL2/SDL.h>
#include "../incl/visualizer.h"

// Orbit State
Vect3 target = {0.0f, 0.0f, 0.0f};  // The point we orbit
float distance = 5.0f;             // Radius
float azimuth = 1.57f;             // Horizontal angle (radians)
float polar = 1.57f;               // Vertical angle (radians)

float moveSpeed = 0.6f;           // Speed of target movement
float orbitSpeed = 0.06f;          // Speed of rotation
float zoomSpeed = 0.1f;            // Speed of distance change

void InputLoop(float dt)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT) exit(0);
    }

    const Uint8 *state = SDL_GetKeyboardState(NULL);

    if (state[SDL_SCANCODE_ESCAPE]) exit(0);

    // --- CAMERA ROTATION ---
    if (state[SDL_SCANCODE_A]) azimuth -= orbitSpeed * dt;
    if (state[SDL_SCANCODE_D]) azimuth += orbitSpeed * dt;
    if (state[SDL_SCANCODE_W]) polar   -= orbitSpeed * dt;
    if (state[SDL_SCANCODE_S]) polar   += orbitSpeed * dt;

    // Wrap azimuth
    if (azimuth > 2.0f * M_PI) azimuth -= 2.0f * M_PI;
    if (azimuth < 0.0f)        azimuth += 2.0f * M_PI;

    // Clamp polar
    const float epsilon = 0.2f;
    if (polar < epsilon) polar = epsilon;
    if (polar > M_PI - epsilon) polar = M_PI - epsilon;

    // --- CAMERA-RELATIVE MOVEMENT ---
    Vect3 forward = { cosf(azimuth), 0.0f, sinf(azimuth) };
    Vect3 right   = { -sinf(azimuth), 0.0f, cosf(azimuth) };

    if (state[SDL_SCANCODE_UP]) {
        target.x += forward.x * moveSpeed * dt;
        target.z += forward.z * moveSpeed * dt;
    }
    if (state[SDL_SCANCODE_DOWN]) {
        target.x -= forward.x * moveSpeed * dt;
        target.z -= forward.z * moveSpeed * dt;
    }
    if (state[SDL_SCANCODE_LEFT]) {
        target.x -= right.x * moveSpeed * dt;
        target.z -= right.z * moveSpeed * dt;
    }
    if (state[SDL_SCANCODE_RIGHT]) {
        target.x += right.x * moveSpeed * dt;
        target.z += right.z * moveSpeed * dt;
    }

    // --- ZOOM ---
    if (state[SDL_SCANCODE_Z]) distance *= (1.0f - zoomSpeed * dt);
    if (state[SDL_SCANCODE_X])   distance *= (1.0f + zoomSpeed * dt);

    if (distance < 1.0f) distance = 1.0f;

    // --- CALCULATE CAMERA POSITION ---
    Vect3 eye;
    eye.x = target.x + distance * sinf(polar) * cosf(azimuth);
    eye.y = target.y + distance * cosf(polar);
    eye.z = target.z + distance * sinf(polar) * sinf(azimuth);

    Vect3 up = {0.0f, 1.0f, 0.0f};
    Update_Camera(eye, target, up);
}