#include <applications.h>
#include <lib/toolbox/value_index.h>

#include "../desktop_settings_app.h"
#include "desktop_settings_scene.h"

#define SCENE_EVENT_SELECT_FAVORITE_PRIMARY 0
#define SCENE_EVENT_SELECT_FAVORITE_SECONDARY 1
// #define SCENE_EVENT_SELECT_FAVORITE_GAME 2
#define SCENE_EVENT_SELECT_PIN_SETUP 2
#define SCENE_EVENT_SELECT_AUTO_LOCK_DELAY 3
#define SCENE_EVENT_SELECT_AUTO_LOCK_PIN 4
#define SCENE_EVENT_SELECT_BATTERY_DISPLAY 5

#define AUTO_LOCK_DELAY_COUNT 9
const char* const auto_lock_delay_text[AUTO_LOCK_DELAY_COUNT] = {
    "OFF",
    "10s",
    "15s",
    "30s",
    "60s",
    "90s",
    "2min",
    "5min",
    "10min",
};
const uint32_t auto_lock_delay_value[AUTO_LOCK_DELAY_COUNT] =
    {0, 10000, 15000, 30000, 60000, 90000, 120000, 300000, 600000};

static void desktop_settings_scene_start_var_list_enter_callback(void* context, uint32_t index) {
    DesktopSettingsApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

static void desktop_settings_scene_start_auto_lock_delay_changed(VariableItem* item) {
    DesktopSettingsApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, auto_lock_delay_text[index]);
    app->settings.auto_lock_delay_ms = auto_lock_delay_value[index];
}

static void desktop_settings_scene_start_auto_lock_pin_changed(VariableItem* item) {
    DesktopSettingsApp* app = variable_item_get_context(item);
    uint8_t value = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, value ? "ON" : "OFF");
    app->settings.auto_lock_with_pin = value;
}

void desktop_settings_scene_start_on_enter(void* context) {
    DesktopSettingsApp* app = context;
    VariableItemList* variable_item_list = app->variable_item_list;

    VariableItem* item;
    uint8_t value_index;

    variable_item_list_add(variable_item_list, "Primary Fav App (Up)", 1, NULL, NULL);

    variable_item_list_add(variable_item_list, "Secondary Fav App (Down)", 1, NULL, NULL);

    // variable_item_list_add(variable_item_list, "Favorite Game", 1, NULL, NULL);

    variable_item_list_add(variable_item_list, "PIN Setup", 1, NULL, NULL);

    item = variable_item_list_add(
        variable_item_list,
        "Auto Lock Time",
        AUTO_LOCK_DELAY_COUNT,
        desktop_settings_scene_start_auto_lock_delay_changed,
        app);

    value_index = value_index_uint32(
        app->settings.auto_lock_delay_ms, auto_lock_delay_value, AUTO_LOCK_DELAY_COUNT);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, auto_lock_delay_text[value_index]);

    item = variable_item_list_add(
        variable_item_list,
        "Auto Lock Pin",
        2,
        desktop_settings_scene_start_auto_lock_pin_changed,
        app);

    variable_item_set_current_value_index(item, app->settings.auto_lock_with_pin);
    variable_item_set_current_value_text(item, app->settings.auto_lock_with_pin ? "ON" : "OFF");

    variable_item_list_set_enter_callback(
        variable_item_list, desktop_settings_scene_start_var_list_enter_callback, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, DesktopSettingsAppViewVarItemList);
}

bool desktop_settings_scene_start_on_event(void* context, SceneManagerEvent sme) {
    DesktopSettingsApp* app = context;
    bool consumed = false;

    if(sme.type == SceneManagerEventTypeCustom) {
        switch(sme.event) {
        case SCENE_EVENT_SELECT_FAVORITE_PRIMARY:
            scene_manager_set_scene_state(
                app->scene_manager, DesktopSettingsAppSceneFavorite, true);
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppSceneFavorite);
            consumed = true;
            break;
        case SCENE_EVENT_SELECT_FAVORITE_SECONDARY:
            scene_manager_set_scene_state(
                app->scene_manager, DesktopSettingsAppSceneFavorite, false);
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppSceneFavorite);
            consumed = true;
            break;
        // case SCENE_EVENT_SELECT_FAVORITE_GAME:
        // scene_manager_set_scene_state(app->scene_manager, DesktopSettingsAppSceneFavorite, 2);
        // scene_manager_next_scene(app->scene_manager, DesktopSettingsAppSceneFavorite);
        // consumed = true;
        // break;
        case SCENE_EVENT_SELECT_PIN_SETUP:
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppScenePinMenu);
            consumed = true;
            break;
        case SCENE_EVENT_SELECT_AUTO_LOCK_DELAY:
            consumed = true;
            break;
        case SCENE_EVENT_SELECT_BATTERY_DISPLAY:
            consumed = true;
            break;
        }
    }
    return consumed;
}

void desktop_settings_scene_start_on_exit(void* context) {
    DesktopSettingsApp* app = context;
    variable_item_list_reset(app->variable_item_list);
    DESKTOP_SETTINGS_SAVE(&app->settings);
}
