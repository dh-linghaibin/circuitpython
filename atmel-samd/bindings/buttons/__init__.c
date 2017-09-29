/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Radomir Dopieralski for Adafruit Industries
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
 #include "py/obj.h"
 #include "py/runtime.h"
 #include "buttons.h"

//| :mod:`buttons` --- Button handling
//  ==================================
//|
//| .. module:: buttons
//|   :synopsis: Button handling
//|   :platform: SAMD21
//|

//| ..function:: get_pressed()
//|
//| Get the status of buttons pressed since the last call.
//|
//| Returns an 8-bit number, with bits that correspond to buttons, which
//| have been pressed (or held down) since the last call to this function
//| set to 1, and the remaining bits set to 0.
//|
STATIC mp_obj_t buttons_get_pressed(void) {
    mp_obj_t buttons = MP_OBJ_NEW_SMALL_INT(buttons_pressed);
    buttons_pressed = 0;
    return buttons;
}
MP_DEFINE_CONST_FUN_OBJ_0(buttons_get_pressed_obj, buttons_get_pressed);

//| ..function:: setup([b1[, b2[, b3[, b4[, b5[, b6[, b7[, b8]]]]]]]])
//|
//| Initializes button scanning routines.
//|
//| The ``b1``-``b8`` parameters are ``DigitalInOut`` objects, which
//| immediately get switched to input with a pull-up, and then scanned
//| regularly for button presses. The order is the same as the order of
//| bits returned by the ``get_pressed`` function. To disable button
//| scanning, call this without any arguments.
//|
STATIC mp_obj_t buttons_setup(size_t n_args, const mp_obj_t *args) {
    buttons_init(n_args, args);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(buttons_setup_obj, 0, 8, buttons_setup);

STATIC const mp_rom_map_elem_t buttons_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_buttons) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_pressed),  MP_ROM_PTR(&buttons_get_pressed_obj)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_setup),  MP_ROM_PTR(&buttons_setup_obj)},
};

STATIC MP_DEFINE_CONST_DICT(buttons_module_globals, buttons_module_globals_table);

const mp_obj_module_t buttons_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&buttons_module_globals,
};
