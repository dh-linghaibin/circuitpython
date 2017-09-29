
#include <stdbool.h>

#include "buttons.h"

#include "shared-bindings/digitalio/Pull.h"
#include "shared-bindings/digitalio/DigitalInOut.h"


volatile uint8_t buttons_pressed;
static digitalio_digitalinout_obj_t* button_pins[8];


void buttons_tick(void) {
    static uint8_t buttons_last = 0;

    uint8_t buttons_current = 0;
    uint8_t bit = 1;
    for (int i=0; i<6; ++i) {
        if (!button_pins[i]) {
            break;
        }
        if (!common_hal_digitalio_digitalinout_get_value(button_pins[i])) {
            buttons_current |= bit;
        }
        bit <<= 1;
    }
    buttons_pressed |= buttons_last & buttons_current;
    buttons_last = buttons_current;
}

void buttons_setup(size_t n_pins, const mp_obj_t* pins) {
    for (size_t i=0; i<8; ++i) {
        button_pins[i] = NULL;
    }
    for (size_t i=0; i<n_pins; ++i) {
        digitalio_digitalinout_obj_t *pin = MP_OBJ_TO_PTR(pins[i]);
        button_pins[i] = pin;
        common_hal_digitalio_digitalinout_switch_to_input(pin, PULL_UP);
    }
}
