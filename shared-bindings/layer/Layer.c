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


STATIC mp_obj_t layer_make_new(const mp_obj_type_t *type, size_t n_args,
        size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 4, 5, false);

    layer_obj_t *self = m_new_obj(layer_obj_t);
    self->base.type = type;

    self->width = mp_obj_get_int(args[0]);
    self->height = mp_obj_get_int(args[1]);
    self->x = 0;
    self->y = 0;
    self->frame = 0;
    self->mirror = false;

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[2], &bufinfo, MP_BUFFER_READ);
    self->graphic = bufinfo.buf;
    if (bufinfo.len != 2048) {
        mp_raise_ValueError("graphic must be 2048 bytes long");
    }

    mp_get_buffer_raise(args[3], &bufinfo, MP_BUFFER_READ);
    self->palette = bufinfo.buf;
    if (bufinfo.len != 32) {
        mp_raise_ValueError("palette must be 32 bytes long");
    }

    if (n_args > 4) {
        mp_get_buffer_raise(args[4], &bufinfo, MP_BUFFER_READ);
        self->map = bufinfo.buf;
        if (bufinfo.len < (self->width * self->height) / 2) {
            mp_raise_ValueError("map buffer too small");
        }
    } else {
        self-> map = NULL;
    }

    return MP_OBJ_FROM_PTR(self);
}

uint16_t get_layer_pixel(layer_obj_t *self, int16_t x, uint16_t y) {
    x -= self->x;
    y -= self->y;

    if ((x < 0) || (x >= self->width << 4) ||
            (y < 0) || (y >= self->height << 4)) {
        // out of bounds
        return TRANSPARENT;
    }

    uint8_t frame = self->frame;
    if (self->map) {
        uint8_t tx = x >> 4;
        uint8_t ty = y >> 4;

        frame = self->map[(tx * self->width + ty) >> 1];
        if (ty & 0x01) {
            frame &= 0x0f;
        } else {
            frame >>= 4;
        }
    }

    x &= 0x0f;
    y &= 0x0f;

    if (self->mirror) {
        y = 15 - y;
    }

    uint8_t pixel = self->graphic[(frame << 7) + (x << 3) + (y >> 1)];
    if (y & 0x01) {
        pixel &= 0x0f;
    } else {
        pixel >>= 4;
    }

    return self->palette[pixel];
}


STATIC mp_obj_t layer_pixel(mp_obj_t self_in, mp_obj_t x_in, mp_obj_t y_in) {
    layer_obj_t *self = MP_OBJ_TO_PTR(self_in);
    int16_t x = mp_obj_get_int(x_in);
    int16_t y = mp_obj_get_int(y_in);

    return MP_OBJ_NEW_SMALL_INT(get_layer_pixel(self, x, y));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(layer_pixel_obj, layer_pixel);


STATIC mp_obj_t layer_move(mp_obj_t self_in, mp_obj_t x_in, mp_obj_t y_in) {
    layer_obj_t *self = MP_OBJ_TO_PTR(self_in);
    self->x = mp_obj_get_int(x_in);
    self->y = mp_obj_get_int(y_in);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(layer_move_obj, layer_move);


STATIC mp_obj_t layer_frame(mp_obj_t self_in, mp_obj_t frame_in,
                            mp_obj_t mirror_in) {
    layer_obj_t *self = MP_OBJ_TO_PTR(self_in);
    self->frame = mp_obj_get_int(frame_in);
    self->mirror = mp_obj_get_int(mirror_in);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(layer_frame_obj, layer_frame);


STATIC const mp_rom_map_elem_t layer_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_pixel), MP_ROM_PTR(&layer_pixel_obj) },
    { MP_ROM_QSTR(MP_QSTR_move), MP_ROM_PTR(&layer_move_obj) },
    { MP_ROM_QSTR(MP_QSTR_frame), MP_ROM_PTR(&layer_frame_obj) },
};
STATIC MP_DEFINE_CONST_DICT(layer_locals_dict, layer_locals_dict_table);

const mp_obj_type_t mp_type_layer = {
    { &mp_type_type },
    .name = MP_QSTR_Layer,
    .make_new = layer_make_new,
    .locals_dict = (mp_obj_dict_t*)&layer_locals_dict,
};
