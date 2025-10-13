#ifndef PREFS_H
#define PREFS_H

/// Get the application preference directory path.
/// The returned string must be freed with SDL_free().
/// @return Allocated preference path string, or NULL on failure
char* Prefs_GetPath();

#endif // PREFS_H
