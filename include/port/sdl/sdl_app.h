#ifndef SDL_APP_H
#define SDL_APP_H

#include "port/config.h"
#include <SDL3/SDL.h>

extern SDL_Window* window;

/// Initialize SDL application.
/// @param config Pointer to configuration to use for initialization
/// @return 0 on success, non-zero on failure
int SDLApp_Init(Config* config);
void SDLApp_Quit();

/// @brief Poll SDL events.
/// @return `true` if the main loop should continue running, `false` otherwise.
bool SDLApp_PollEvents();

void SDLApp_BeginFrame();
void SDLApp_EndFrame();
void SDLApp_Exit();

#endif
