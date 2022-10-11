#include "../nfc_i.h"

enum SubmenuIndex {
    SubmenuIndexBac,
    SubmenuIndexPace,
};

void nfc_scene_passport_menu_submenu_callback(void* context, uint32_t index) {
    Nfc* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, index);
}

void nfc_scene_passport_menu_on_enter(void* context) {
    Nfc* nfc = context;
    Submenu* submenu = nfc->submenu;

    //TODO: enable/disable available methods

    submenu_add_item(
        submenu, "BAC", SubmenuIndexBac, nfc_scene_passport_menu_submenu_callback, nfc);
    submenu_add_item(
        submenu, "PACE", SubmenuIndexPace, nfc_scene_passport_menu_submenu_callback, nfc);

    submenu_set_selected_item(
        nfc->submenu, scene_manager_get_scene_state(nfc->scene_manager, NfcScenePassportMenu));

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
}

bool nfc_scene_passport_menu_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexBac) {
            nfc->dev->format = NfcDeviceSaveFormatUid;
            // Clear device name
            nfc_device_set_name(nfc->dev, "");
            scene_manager_next_scene(nfc->scene_manager, NfcScenePassportBac);
            consumed = true;
        } else if(event.event == SubmenuIndexPace) {
            //scene_manager_next_scene(nfc->scene_manager, NfcScenePassportPace);
            consumed = true;
        }
        scene_manager_set_scene_state(nfc->scene_manager, NfcScenePassportMenu, event.event);
    } else if(event.type == SceneManagerEventTypeBack) {
        consumed = scene_manager_previous_scene(nfc->scene_manager);
    }

    return consumed;
}

void nfc_scene_passport_menu_on_exit(void* context) {
    Nfc* nfc = context;

    // Clear view
    submenu_reset(nfc->submenu);
}
