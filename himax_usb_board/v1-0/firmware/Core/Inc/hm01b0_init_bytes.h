#ifndef _HM01B0_INIT_VALUES_H
#define _HM01B0_INIT_VALUES_H

#include <stdint.h>

typedef struct hm01b0_reg_write {
    uint16_t ui16Reg;
    uint8_t ui8Val;
} hm01b0_reg_write_t;

const extern hm01b0_reg_write_t hm01b0_init_values[];
const extern int sizeof_hm01b0_init_values;

#endif
