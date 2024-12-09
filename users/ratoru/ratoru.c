#include "ratoru.h"
#include "features/select_word.h"
#include "features/swapper.h"
#include "features/oneshot.h"

bool sw_tab_active = false;
bool sw_control_tab_active = false;
bool sw_backtick_active = false;

oneshot_state os_shft_state = os_up_unqueued;
oneshot_state os_ctrl_state = os_up_unqueued;
oneshot_state os_alt_state = os_up_unqueued;
oneshot_state os_cmd_state = os_up_unqueued;
oneshot_state os_hypr_state = os_up_unqueued;

bool is_oneshot_cancel_key(uint16_t keycode) {
    switch (keycode) {
    case TL_LOWR:
    case TL_UPPR:
        return true;
    default:
        return false;
    }
}

bool is_oneshot_ignored_key(uint16_t keycode) {
    switch (keycode) {
    case TL_LOWR:
    case TL_UPPR:
    case KC_LSFT:
    case OS_SHFT:
    case OS_CTRL:
    case OS_ALT:
    case OS_CMD:
    case OS_HYPR:
        return true;
    default:
        return false;
    }
}

bool process_record_user(uint16_t keycode, keyrecord_t* record) {
  if (!process_select_word(keycode, record, SELWORD)) { return false; }

  // Adds functionality to switch apps and windows.
  update_swapper(
      &sw_tab_active, KC_LGUI, KC_TAB, SW_TAB,
      keycode, record
  );
  update_swapper(
      &sw_control_tab_active, KC_LCTL, KC_TAB, SW_CTAB,
      keycode, record
  );
  update_swapper(
      &sw_backtick_active, KC_LGUI, KC_GRAVE, SW_BTICK,
      keycode, record
  );

  // Process oneshot keys
  update_oneshot(
    &os_shft_state, KC_LSFT, OS_SHFT,
    keycode, record
  );
  update_oneshot(
    &os_ctrl_state, KC_LCTL, OS_CTRL,
    keycode, record
  );
  update_oneshot(
    &os_alt_state, KC_LALT, OS_ALT,
    keycode, record
  );
  update_oneshot(
    &os_cmd_state, KC_LCMD, OS_CMD,
    keycode, record
  );
  update_oneshot(
    &os_hypr_state, KC_HYPR, OS_HYPR,
    keycode, record
  );

  switch (keycode) {
    case UPDIR:  // Types ../ to go up a directory on the shell.
      if (record->event.pressed) {
        SEND_STRING("../");
      }
      return false;
    case S_END:
      if (record->event.pressed) {
        SEND_STRING(". ");
        tap_code16(OSM(KC_LSFT));
      }
      return false;
    case CPY_URL:
      if (record->event.pressed) {
        SEND_STRING(SS_LGUI("lc"));
      }
      return false;
    #ifdef RGBLIGHT_ENABLE
    case RGBT_NE:
      if (record->event.pressed) {
        rgblight_toggle_noeeprom();
      }
      return false;
    case RGB_IB_NE:
      if (record->event.pressed) {
        rgblight_increase_val_noeeprom();
      }
      return false;
    case RGB_DB_NE:
      if (record->event.pressed) {
        rgblight_decrease_val_noeeprom();
      }
      return false;
    #endif
  }
  return true;
}

// ---- Home Row Mods "Timeless" Config ----
#ifdef TIMELESSHMR_ENABLE
static uint16_t    next_keycode;
static keyrecord_t next_record;

bool pre_process_record_user(uint16_t keycode, keyrecord_t *record) {
    // Instant Tap Config
    static uint16_t prev_keycode;
    if (record->event.pressed) {
        // Store the previous keycode for instant tap decision
        prev_keycode = next_keycode;
        // Cache the next input for mod-tap decisions
        next_keycode = keycode;
        next_record  = *record;
    }
    // Match home row mod-tap keys when it is not preceded by a Layer key
    if (IS_HOMEROW(record) && IS_QK_MOD_TAP(keycode) && !IS_QK_LAYER_TAP(prev_keycode)) {
        // Tap the mod-tap key instantly when it follows a short interval
        if (record->event.pressed && last_input_activity_elapsed() < TAP_INTERVAL_MS) {
            record->keycode = keycode & 0xff;
            action_tapping_process(*record);
            return false;
        } else { // Send the base keycode key up event
            keyrecord_t base_record   = *record;
            base_record.keycode       = keycode & 0xff;
            base_record.event.pressed = false;
            action_tapping_process(base_record);
        }
    }
    return true;
}

bool get_permissive_hold(uint16_t keycode, keyrecord_t *record) {
    // Hold Control and Shift with a nested key tap on the opposite hand
    return IS_BILATERAL_TAP(record, next_record);
}

bool get_hold_on_other_key_press(uint16_t keycode, keyrecord_t *record) {
    // Replace the mod-tap key with its base keycode when
    // tapped with another key on the same hand
    if (IS_UNILATERAL_TAP(record, next_record)) {
        record->keycode = keycode & 0xff;
        process_record(record);
        record->event.pressed = false;
        process_record(record);
        return true;
    }
    // Hold layer with another key press
    else if (IS_QK_LAYER_TAP(keycode) && QK_LAYER_TAP_GET_LAYER(keycode)) {
        return true;
    }
    return false;
}
#endif

#ifdef RGBLIGHT_ENABLE
void keyboard_pre_init_user(void) {
  // Set our LED pin as output
  setPinOutput(24);
  // Turn the LED off
  // (Due to technical reasons, high is off and low is on)
  writePinHigh(24);
}

void keyboard_post_init_user(void) {
    rgblight_enable_noeeprom(); // Enables RGB, without saving settings
    rgblight_sethsv_noeeprom(HSV_PINK);
    rgblight_mode_noeeprom(1);
}

layer_state_t layer_state_set_user(layer_state_t state) {
    switch (get_highest_layer(state)) {
     case 1:
         rgblight_sethsv_noeeprom (HSV_MAGENTA);
         break;
     case 2:
         rgblight_sethsv_noeeprom (HSV_BLUE);
         break;
     case 3:
         rgblight_sethsv_noeeprom (HSV_GOLD);
         break;
     case 4:
         rgblight_sethsv_noeeprom (HSV_GREEN);
         break;
     default: //  for any other layers, or the default layer
         rgblight_sethsv_noeeprom (HSV_PINK);
         break;
     }
  return state;
 };

void suspend_power_down_user(void) {
    // code will run multiple times while keyboard is suspended
    rgblight_disable_noeeprom();
}

void suspend_wakeup_init_user(void) {
    // code will run on keyboard wakeup
    rgblight_enable_noeeprom();
    rgblight_sethsv_noeeprom(HSV_MAGENTA);
    rgblight_mode_noeeprom(1);
}
#endif
