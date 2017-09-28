
#include <stdbool.h>

#include "buttons.h"

#include "shared-bindings/digitalio/Pull.h"
#include "shared-bindings/digitalio/DigitalInOut.h"


volatile uint8_t buttons_pressed;
static digitalio_digitalinout_obj_t* button_pins[6];


void buttons_tick(void) {
    static uint8_t buttons_last = 0;

    if (!button_pins[0]) {
        return;  // Not configured yet.
    }
    uint8_t buttons_current = 0;
    uint8_t bit = 1;
    for (int i=0; i<6; ++i) {
        if (common_hal_digitalio_digitalinout_get_value(button_pins[i])) {
            buttons_current |= bit;
        }
        bit <<= 1;
    }
    buttons_pressed = buttons_last & buttons_current;
    buttons_last = buttons_current;
}

void buttons_setup(
        digitalio_digitalinout_obj_t* pin_up,
        digitalio_digitalinout_obj_t* pin_down,
        digitalio_digitalinout_obj_t* pin_left,
        digitalio_digitalinout_obj_t* pin_right,
        digitalio_digitalinout_obj_t* pin_o,
        digitalio_digitalinout_obj_t* pin_x) {
    button_pins[0] = pin_up;
    button_pins[1] = pin_down;
    button_pins[2] = pin_left;
    button_pins[3] = pin_right;
    button_pins[4] = pin_o;
    button_pins[5] = pin_x;
    for (int i=0; i<6; ++i) {
        common_hal_digitalio_digitalinout_switch_to_input(
                button_pins[i], PULL_UP);
    }
}
