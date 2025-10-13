#ifndef PATHS_H
#define PATHS_H

/// Get the application preference directory path.
/// The returned string must be freed with SDL_free().
/// @return Allocated preference path string, or NULL on failure
char* Paths_GetPrefsPath();

/// Get the application base path (where resources are located).
/// The returned string must be freed with SDL_free().
/// @return Allocated base path string, or NULL on failure
char* Paths_GetBasePath();

#endif // PATHS_H
