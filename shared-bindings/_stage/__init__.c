/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Radomir Dopieralski
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

#include "__init__.h"
#include "Layer.h"
#include "shared-bindings/busio/SPI.h"
#include "py/mperrno.h"
#include "py/runtime.h"


// Render a rectangle of the screen, and send it over SPI to the display.
STATIC mp_obj_t stage_render(size_t n_args, const mp_obj_t *args) {
    uint8_t x0 = mp_obj_get_int(args[0]);
    uint8_t y0 = mp_obj_get_int(args[1]);
    uint8_t x1 = mp_obj_get_int(args[2]);
    uint8_t y1 = mp_obj_get_int(args[3]);

    size_t layers_size = 0;
    mp_obj_t *layers;
    mp_obj_get_array(args[4], &layers_size, &layers);

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[5], &bufinfo, MP_BUFFER_WRITE);
    uint16_t *buffer = bufinfo.buf;
    size_t buffer_size = bufinfo.len / 2; // 16-bit indexing

    busio_spi_obj_t *spi = MP_OBJ_TO_PTR(args[6]);

    // TODO(deshipu): Do a collision check of each layer with the
    // rectangle, and only process the layers that overlap with it.

    size_t index = 0;
    for (uint8_t y = y0; y < y1; ++y) {
        for (uint8_t x = x0; x < x1; ++x) {
            for (size_t layer = 0; layer < layers_size; ++layer) {
                uint16_t c = get_layer_pixel(MP_OBJ_TO_PTR(layers[layer]),
                        x, y);
                if (c != TRANSPARENT) {
                    buffer[index] = c;
                    break;
                }
            }
            index += 1;
            // The buffer is full, send it.
            if (index >= buffer_size) {
                if (!common_hal_busio_spi_write(spi,
                        ((uint8_t*)bufinfo.buf), bufinfo.len)) {
                    mp_raise_OSError(MP_EIO);
                }
                index = 0;
            }
        }
    }
    // Send the remaining data.
    if (index) {
        if (!common_hal_busio_spi_write(spi,
                ((uint8_t*)bufinfo.buf), index * 2)) {
            mp_raise_OSError(MP_EIO);
        }
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(stage_render_obj, 7, 7, stage_render);


STATIC const mp_rom_map_elem_t stage_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_layer) },
    { MP_ROM_QSTR(MP_QSTR_Layer), MP_ROM_PTR(&mp_type_layer) },
    { MP_ROM_QSTR(MP_QSTR_render), MP_ROM_PTR(&stage_render_obj) },
};

STATIC MP_DEFINE_CONST_DICT(stage_module_globals, stage_module_globals_table);

const mp_obj_module_t stage_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&stage_module_globals,
};
