#include "samd21_pins.h"

STATIC const mp_rom_map_elem_t board_global_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_SDA), MP_ROM_PTR(&pin_PA08) },
    { MP_ROM_QSTR(MP_QSTR_SCL), MP_ROM_PTR(&pin_PA09) },

    { MP_ROM_QSTR(MP_QSTR_SPEAKER), MP_ROM_PTR(&pin_PA02) },

    { MP_ROM_QSTR(MP_QSTR_K_RIGHT), MP_ROM_PTR(&pin_PA03) },
    { MP_ROM_QSTR(MP_QSTR_K_LEFT), MP_ROM_PTR(&pin_PA04) },
    { MP_ROM_QSTR(MP_QSTR_K_O), MP_ROM_PTR(&pin_PA05) },
    { MP_ROM_QSTR(MP_QSTR_K_DOWN), MP_ROM_PTR(&pin_PA06) },
    { MP_ROM_QSTR(MP_QSTR_K_UP), MP_ROM_PTR(&pin_PA07) },
    { MP_ROM_QSTR(MP_QSTR_K_X), MP_ROM_PTR(&pin_PA23) },

};
MP_DEFINE_CONST_DICT(board_module_globals, board_global_dict_table);
