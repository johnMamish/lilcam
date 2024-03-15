#ifndef _HM0360_INIT_VALUES_H
#define _HM0360_INIT_VALUES_H

#include <stdint.h>

typedef struct hm0360_reg_write {
    uint16_t ui16Reg;
    uint8_t ui8Val;
} hm0360_reg_write_t;

const extern hm0360_reg_write_t hm0360_init_values[];
const extern int sizeof_hm0360_init_values;

#endif
