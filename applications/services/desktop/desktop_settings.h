#pragma once

#include <furi_hal.h>
#include <stdint.h>
#include <stdbool.h>
#include <toolbox/saved_struct.h>
#include <storage/storage.h>
#include <loader/loader.h>

#define DESKTOP_SETTINGS_VER (8)

#define DESKTOP_SETTINGS_OLD_PATH CFG_PATH("desktop.settings")
#define DESKTOP_SETTINGS_PATH INT_PATH(".desktop.settings")
#define DESKTOP_SETTINGS_MAGIC (0x17)
#define PIN_MAX_LENGTH 12

#define DESKTOP_SETTINGS_RUN_PIN_SETUP_ARG "run_pin_setup"

#define MAX_PIN_SIZE 10
#define MIN_PIN_SIZE 4
#define MAX_APP_LENGTH 128

typedef struct {
    InputKey data[MAX_PIN_SIZE];
    uint8_t length;
} PinCode;

typedef struct {
    bool is_external;
    char name_or_path[MAX_APP_LENGTH];
} FavoriteApp;

typedef struct {
    FavoriteApp favorite_primary;
    FavoriteApp favorite_secondary;
    PinCode pin_code;
    uint32_t auto_lock_delay_ms;
    bool auto_lock_with_pin;
} DesktopSettings;

bool DESKTOP_SETTINGS_SAVE(DesktopSettings* x);

bool DESKTOP_SETTINGS_LOAD(DesktopSettings* x);
