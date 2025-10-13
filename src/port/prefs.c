#include "port/prefs.h"

#include <SDL3/SDL.h>

#define PORT_ORG_NAME "CrowdedStreet"
#define PORT_APP_NAME "3SX"

char* Prefs_GetPath() {
    return SDL_GetPrefPath(PORT_ORG_NAME, PORT_APP_NAME);
}
