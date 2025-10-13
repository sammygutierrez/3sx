#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

/// Configuration structure for video settings.
typedef struct {
    bool fullscreen; ///< Fullscreen mode enabled
    int width;       ///< Window/screen width in pixels
    int height;      ///< Window/screen height in pixels
} Config;

/// Load config from file. Creates default config file if it doesn't exist.
/// @param config Pointer to Config structure to populate
/// @return `true` on success, `false` on failure.
bool Config_Load(Config* config);

#endif // CONFIG_H
