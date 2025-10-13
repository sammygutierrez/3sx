#include "port/config.h"
#include "common.h"
#include "port/paths.h"

#include <SDL3/SDL.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONFIG_FILENAME "config"
#define CONFIG_ENTRY_COUNT 3

typedef enum {
    CONFIG_TYPE_BOOL,
    CONFIG_TYPE_INT,
} ConfigType;

typedef struct {
    ConfigType type;
    bool* value;
} ConfigBoolValue;

typedef struct {
    ConfigType type;
    int* value;
} ConfigIntValue;

typedef union {
    ConfigType type;
    ConfigBoolValue bool_value;
    ConfigIntValue int_value;
} ConfigValue;

typedef struct {
    const char* key;
    ConfigValue value;
} ConfigEntry;

typedef struct {
    char* key;
    char* value;
} RawConfigEntry;

static char* trim_whitespace(char* str) {
    char* end;

    if (str == NULL) {
        return NULL;
    }

    while (isspace((unsigned char)*str)) {
        str++;
    }

    if (*str == '\0') {
        return str;
    }

    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) {
        end--;
    }

    end[1] = '\0';

    return str;
}

// Parse config file into untyped key-value pairs
static size_t parse_config_file(const char* file_contents, size_t file_size, RawConfigEntry* entries) {
    size_t parsed_entry_count = 0;
    char* line_start = (char*)file_contents;
    char* line_end = (char*)file_contents;
    int line_number = 0;

    while (line_end < file_contents + file_size && parsed_entry_count < CONFIG_ENTRY_COUNT) {
        line_number++;

        // Find end of line
        while (line_end < file_contents + file_size && *line_end != '\n' && *line_end != '\r') {
            line_end++;
        }

        // Create a null-terminated copy of the line
        size_t line_len = line_end - line_start;
        if (line_len > 0) {
            char* line = SDL_malloc(line_len + 1);
            if (!line) {
                // Clean up already allocated entries
                for (size_t i = 0; i < parsed_entry_count; i++) {
                    SDL_free(entries[i].key);
                    SDL_free(entries[i].value);
                }
                return 0;
            }
            memcpy(line, line_start, line_len);
            line[line_len] = '\0';

            char* trimmed = trim_whitespace(line);
            bool should_parse = false;

            // Skip empty lines and full-line comments
            if (trimmed[0] != '\0' && trimmed[0] != '#') {
                // Strip inline comments
                char* comment_pos = strchr(trimmed, '#');
                if (comment_pos) {
                    *comment_pos = '\0';
                    trimmed = trim_whitespace(trimmed);
                }

                // Check if line still has content after comment stripping
                if (trimmed[0] != '\0') {
                    should_parse = true;
                }
            }

            if (should_parse) {
                // Parse key-value pair
                char* saveptr;
                char* key = SDL_strtok_r(trimmed, " \t=", &saveptr);
                char* value = SDL_strtok_r(NULL, " \t=", &saveptr);

                if (key && value) {
                    value = trim_whitespace(value);

                    entries[parsed_entry_count].key = SDL_strdup(key);
                    entries[parsed_entry_count].value = SDL_strdup(value);

                    if (!entries[parsed_entry_count].key || !entries[parsed_entry_count].value) {
                        SDL_free(entries[parsed_entry_count].key);
                        SDL_free(entries[parsed_entry_count].value);
                        SDL_free(line);
                        for (size_t i = 0; i < parsed_entry_count; i++) {
                            SDL_free(entries[i].key);
                            SDL_free(entries[i].value);
                        }
                        return 0;
                    }

                    parsed_entry_count++;
                } else {
                    SDL_Log("Config warning: Line %d has no value, skipping", line_number);
                }
            }

            SDL_free(line);
        }

        while (line_end < file_contents + file_size && (*line_end == '\n' || *line_end == '\r')) {
            line_end++;
        }
        line_start = line_end;
    }

    return parsed_entry_count;
}

static void map_raw_entries_to_config(Config* config, const RawConfigEntry* entries, size_t parsed_entry_count) {
    ConfigEntry config_entries[] = {
        { "fullscreen", { .bool_value = { CONFIG_TYPE_BOOL, &config->fullscreen } } },
        { "width", { .int_value = { CONFIG_TYPE_INT, &config->width } } },
        { "height", { .int_value = { CONFIG_TYPE_INT, &config->height } } },
    };
    const size_t known_entry_count = sizeof(config_entries) / sizeof(config_entries[0]);

    for (size_t i = 0; i < parsed_entry_count; i++) {
        const char* key = entries[i].key;
        const char* value = entries[i].value;

        bool found = false;
        for (size_t j = 0; j < known_entry_count; j++) {
            if (strcmp(key, config_entries[j].key) == 0) {
                found = true;
                switch (config_entries[j].value.type) {
                case CONFIG_TYPE_BOOL: {
                    bool bool_val = false;
                    if (SDL_strcasecmp(value, "true") == 0 || SDL_strcasecmp(value, "yes") == 0 ||
                        SDL_strcasecmp(value, "1") == 0 || SDL_strcasecmp(value, "on") == 0) {
                        bool_val = true;
                    }
                    *config_entries[j].value.bool_value.value = bool_val;
                    break;
                }
                case CONFIG_TYPE_INT: {
                    char* endptr;
                    long val = strtol(value, &endptr, 10);
                    if (*endptr == '\0' && endptr != value) {
                        *config_entries[j].value.int_value.value = (int)val;
                    } else {
                        SDL_Log("Config warning: Invalid integer value '%s' for %s", value, key);
                    }
                    break;
                }
                }
                break;
            }
        }

        if (!found) {
            SDL_Log("Config warning: Unknown config key '%s'", key);
        }
    }
}

static void free_raw_entries(RawConfigEntry* entries, size_t parsed_entry_count) {
    for (size_t i = 0; i < parsed_entry_count; i++) {
        SDL_free(entries[i].key);
        SDL_free(entries[i].value);
    }
}

static void init_defaults(Config* config) {
    if (!config) {
        return;
    }

    config->fullscreen = false;
    config->width = 640;
    config->height = 480;
}

static char* Config_GetPath() {
    char* pref_path = Paths_GetPrefsPath();
    if (!pref_path) {
        SDL_Log("Failed to get preference path: %s", SDL_GetError());
        return NULL;
    }

    size_t path_len = strlen(pref_path) + strlen(CONFIG_FILENAME) + 1;
    char* config_path = SDL_malloc(path_len);
    if (!config_path) {
        SDL_free(pref_path);
        return NULL;
    }

    SDL_snprintf(config_path, path_len, "%s%s", pref_path, CONFIG_FILENAME);
    SDL_free(pref_path);

    return config_path;
}

static void Config_Init() {
    char* config_path = Config_GetPath();
    if (!config_path) {
        return;
    }

    // Check if config file already exists
    SDL_IOStream* existing = SDL_IOFromFile(config_path, "r");
    if (existing) {
        SDL_CloseIO(existing);
        SDL_free(config_path);
        return;
    }

    // Get the path to the default config file
    char* base_path = Paths_GetBasePath();
    if (!base_path) {
        SDL_free(config_path);
        return;
    }

    char* default_config_path = NULL;
    SDL_asprintf(&default_config_path, "%sconfig.default", base_path);
    SDL_free(base_path);

    if (!default_config_path) {
        SDL_free(config_path);
        return;
    }

    // Try to load the default config
    size_t default_size;
    char* default_contents = SDL_LoadFile(default_config_path, &default_size);
    SDL_free(default_config_path);

    if (!default_contents) {
        SDL_free(config_path);
        return;
    }

    // Write the default config to the prefs folder
    SDL_IOStream* file = SDL_IOFromFile(config_path, "w");
    if (!file) {
        SDL_free(default_contents);
        SDL_free(config_path);
        return;
    }

    SDL_WriteIO(file, default_contents, default_size);
    SDL_CloseIO(file);
    SDL_free(default_contents);
    SDL_free(config_path);
}

bool Config_Load(Config* config) {
    if (!config) {
        return false;
    }

    Config_Init();
    init_defaults(config);

    char* config_path = Config_GetPath();
    if (!config_path) {
        return false;
    }

    size_t file_size;
    char* file_contents = SDL_LoadFile(config_path, &file_size);
    SDL_free(config_path);

    if (!file_contents) {
        SDL_Log("Config file not found, using defaults");
        return true;
    }

    RawConfigEntry raw_entries[CONFIG_ENTRY_COUNT];
    size_t parsed_entry_count = parse_config_file(file_contents, file_size, raw_entries);

    SDL_free(file_contents);

    if (parsed_entry_count == 0) {
        SDL_Log("Config file is empty, using defaults");
        return true;
    }

    map_raw_entries_to_config(config, raw_entries, parsed_entry_count);
    free_raw_entries(raw_entries, parsed_entry_count);

    return true;
}
