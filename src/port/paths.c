#include "port/paths.h"

#include <SDL3/SDL.h>

#define PORT_ORG_NAME "CrowdedStreet"
#define PORT_APP_NAME "3SX"

char* Paths_GetPrefsPath() {
    return SDL_GetPrefPath(PORT_ORG_NAME, PORT_APP_NAME);
}

char* Paths_GetBasePath() {
    const char* base_path = SDL_GetBasePath();
    if (!base_path) {
        return NULL;
    }
    return SDL_strdup(base_path);
}
