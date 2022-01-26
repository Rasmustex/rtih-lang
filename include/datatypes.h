#pragma once
#include <stdint.h>

// contains all possible data types of stack data
typedef enum DATA_TYPE {
    I64,
    U64,
    F64,
    U8,
    P64
} type;

typedef struct sim_stack_data {
    type t;
    union {
        int64_t i;
        uint64_t u;
        double f;
        unsigned char c;
        void *p;
    };
} data;

/*
 * accepts a data type and some data (HAS TO MATCH)
 * and returns a stack element with that data and type
 */
data make_data( type t, ... );
// adds two data blocks and returns the result as a data block
data add_data( data a, data b );
// subtracts b from a and returns a
data sub_data( data a, data b );

uint8_t eq_data( data a, data b );
uint8_t lt_data( data a, data b );
uint8_t gt_data( data a, data b );
