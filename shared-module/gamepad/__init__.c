#include <stdbool.h>

#include "__init__.h"

#include "shared-bindings/digitalio/Pull.h"
#include "shared-bindings/digitalio/DigitalInOut.h"


volatile uint8_t gamepad_pressed;


void gamepad_tick(void) {
    if (!gamepad_singleton) {
        return;
    }
    uint8_t gamepad_current = 0;
    for (int i=0; i<8; ++i) {
        digitalio_digitalinout_obj_t* pin = gamepad_singleton->pins[i];
        if (!pin) {
            break;
        }
        if (!common_hal_digitalio_digitalinout_get_value(pin)) {
            gamepad_current |= 1<<i;
        }
    }
    gamepad_singleton->pressed |= gamepad_singleton->last & gamepad_current;
    gamepad_singleton->last = gamepad_current;
}

void gamepad_init(size_t n_pins, const mp_obj_t* pins) {
    for (size_t i=0; i<8; ++i) {
        gamepad_singleton->pins[i] = NULL;
    }
    for (size_t i=0; i<n_pins; ++i) {
        digitalio_digitalinout_obj_t *pin = MP_OBJ_TO_PTR(pins[i]);
        gamepad_singleton->pins[i] = pin;
        common_hal_digitalio_digitalinout_switch_to_input(pin, PULL_UP);
    }
    gamepad_singleton->last = 0;
}
