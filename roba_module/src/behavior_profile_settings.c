/*
 * Per-Bluetooth-profile Apple/Windows layout and connection policy.
 *
 * The behavior is event-source local: a key pressed on the peripheral is
 * forwarded to the central, where the BLE and keymap state lives.
 */

#define DT_DRV_COMPAT zmk_behavior_profile_settings

#include <zephyr/device.h>
#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/settings/settings.h>
#include <zephyr/sys/util.h>
#include <string.h>

#include <drivers/behavior.h>
#include <zmk/behavior.h>

#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
#include <zmk/ble.h>
#include <zmk/event_manager.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/keymap.h>
#endif

#define ROBA_PROFILE_WIN 0
#define ROBA_PROFILE_APPLE 1
#define ROBA_PROFILE_KEEP 2
#define ROBA_PROFILE_DISCONNECT 3

#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)

static uint8_t profile_apple[ZMK_BLE_PROFILE_COUNT];
static uint8_t profile_disconnect[ZMK_BLE_PROFILE_COUNT];

static const uint8_t apple_layer = DT_INST_PROP(0, apple_layer);
static const uint8_t base_layer = DT_INST_PROP(0, base_layer);

static void apply_layout(uint8_t profile) {
    if (profile >= ZMK_BLE_PROFILE_COUNT) {
        return;
    }

    if (profile_apple[profile]) {
        zmk_keymap_layer_activate(apple_layer);
        zmk_keymap_layer_deactivate(base_layer);
    } else {
        zmk_keymap_layer_deactivate(apple_layer);
        zmk_keymap_layer_activate(base_layer);
    }
}

static void disconnect_inactive_profiles(void) {
    const int active = zmk_ble_active_profile_index();

    for (int i = 0; i < ZMK_BLE_PROFILE_COUNT; i++) {
        if (i != active) {
            zmk_ble_prof_disconnect(i);
        }
    }
}

static void apply_connection_policy(void) {
    const int active = zmk_ble_active_profile_index();

    if (active >= 0 && active < ZMK_BLE_PROFILE_COUNT && profile_disconnect[active]) {
        disconnect_inactive_profiles();
    }
}

/* iOS may reconnect shortly after a profile switch, so check once more. */
static void delayed_policy_work_handler(struct k_work *work);
static K_WORK_DELAYABLE_DEFINE(delayed_policy_work, delayed_policy_work_handler);

static void delayed_policy_work_handler(struct k_work *work) {
    ARG_UNUSED(work);
    apply_connection_policy();
}

static void apply_active_profile(void) {
    const int active = zmk_ble_active_profile_index();

    if (active < 0 || active >= ZMK_BLE_PROFILE_COUNT) {
        return;
    }

    apply_layout(active);
    apply_connection_policy();
    k_work_reschedule(&delayed_policy_work, K_MSEC(1500));
}

static int profile_settings_set(const char *name, size_t len,
                                settings_read_cb read_cb, void *cb_arg) {
    if (strcmp(name, "apple") == 0) {
        read_cb(cb_arg, profile_apple, MIN(len, sizeof(profile_apple)));
    } else if (strcmp(name, "disconnect") == 0) {
        read_cb(cb_arg, profile_disconnect,
                MIN(len, sizeof(profile_disconnect)));
    }

    return 0;
}

static int profile_settings_commit(void) {
    apply_active_profile();
    return 0;
}

SETTINGS_STATIC_HANDLER_DEFINE(roba_profile, "roba/profile", NULL,
                               profile_settings_set, profile_settings_commit, NULL);

static int profile_changed_cb(const zmk_event_t *event) {
    ARG_UNUSED(event);
    apply_active_profile();
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(roba_profile_listener, profile_changed_cb);
ZMK_SUBSCRIPTION(roba_profile_listener, zmk_ble_active_profile_changed);

static int set_profile_value(uint8_t command) {
    const int active = zmk_ble_active_profile_index();

    if (active < 0 || active >= ZMK_BLE_PROFILE_COUNT) {
        return -EINVAL;
    }

    switch (command) {
    case ROBA_PROFILE_WIN:
        {
            const uint8_t previous = profile_apple[active];
            int rc;

            profile_apple[active] = 0;
            rc = settings_save_one("roba/profile/apple", profile_apple,
                                   sizeof(profile_apple));
            if (rc < 0) {
                profile_apple[active] = previous;
                return rc;
            }
            apply_layout(active);
            break;
        }
    case ROBA_PROFILE_APPLE:
        {
            const uint8_t previous = profile_apple[active];
            int rc;

            profile_apple[active] = 1;
            rc = settings_save_one("roba/profile/apple", profile_apple,
                                   sizeof(profile_apple));
            if (rc < 0) {
                profile_apple[active] = previous;
                return rc;
            }
            apply_layout(active);
            break;
        }
    case ROBA_PROFILE_KEEP:
        {
            const uint8_t previous = profile_disconnect[active];
            int rc;

            profile_disconnect[active] = 0;
            rc = settings_save_one("roba/profile/disconnect", profile_disconnect,
                                   sizeof(profile_disconnect));
            if (rc < 0) {
                profile_disconnect[active] = previous;
                return rc;
            }
            break;
        }
    case ROBA_PROFILE_DISCONNECT:
        {
            const uint8_t previous = profile_disconnect[active];
            int rc;

            profile_disconnect[active] = 1;
            rc = settings_save_one("roba/profile/disconnect", profile_disconnect,
                                   sizeof(profile_disconnect));
            if (rc < 0) {
                profile_disconnect[active] = previous;
                return rc;
            }
            apply_connection_policy();
            k_work_reschedule(&delayed_policy_work, K_MSEC(1500));
            break;
        }
    default:
        return -ENOTSUP;
    }

    return 0;
}

#endif /* central */

static int on_binding_pressed(struct zmk_behavior_binding *binding,
                              struct zmk_behavior_binding_event event) {
    ARG_UNUSED(event);

#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    return set_profile_value(binding->param1);
#else
    ARG_UNUSED(binding);
    return 0;
#endif
}

static int on_binding_released(struct zmk_behavior_binding *binding,
                               struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);
    ARG_UNUSED(event);
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api profile_settings_driver_api = {
    .binding_pressed = on_binding_pressed,
    .binding_released = on_binding_released,
    .locality = BEHAVIOR_LOCALITY_EVENT_SOURCE,
};

BEHAVIOR_DT_INST_DEFINE(0, NULL, NULL, NULL, NULL, POST_KERNEL,
                        CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
                        &profile_settings_driver_api);
