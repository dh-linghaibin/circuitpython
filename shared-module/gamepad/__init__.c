#include <stdbool.h>

#include "__init__.h"

#include "shared-bindings/digitalio/Pull.h"
#include "shared-bindings/digitalio/DigitalInOut.h"


volatile uint8_t gamepad_pressed;
static digitalio_digitalinout_obj_t* gamepad_pins[8] = {};


void gamepad_tick(void) {
    static uint8_t gamepad_last = 0;

    uint8_t gamepad_current = 0;
    for (int i=0; i<8; ++i) {
        if (!gamepad_pins[i]) {
            break;
        }
        if (!common_hal_digitalio_digitalinout_get_value(gamepad_pins[i])) {
            gamepad_current |= 1<<i;
        }
    }
    gamepad_pressed |= gamepad_last & gamepad_current;
    gamepad_last = gamepad_current;
}

void gamepad_init(size_t n_pins, const mp_obj_t* pins) {
    for (size_t i=0; i<8; ++i) {
        gamepad_pins[i] = NULL;
    }
    for (size_t i=0; i<n_pins; ++i) {
        digitalio_digitalinout_obj_t *pin = MP_OBJ_TO_PTR(pins[i]);
        gamepad_pins[i] = pin;
        common_hal_digitalio_digitalinout_switch_to_input(pin, PULL_UP);
    }
}
