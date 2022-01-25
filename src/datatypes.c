#include <stdarg.h>
#include <stdint.h>
#include "../include/datatypes.h"


typedef union sim_data_holder {
    int64_t i;
    uint64_t u;
    double f;
    char c;
} holder;

struct sim_stack_data {
    type t;
    holder stackdata;
};

data make_data( type t, ... ) {
    va_list vl;
    va_start( vl, t );
    data d = {
        .t = t
    };
    switch( t ) {
    case I64:
        d.stackdata.i = va_arg( vl, int64_t );
        break;
    case U64:
        d.stackdata.u = va_arg( vl, uint64_t );
        break;
    case F64:
        d.stackdata.f = va_arg( vl, double );
        break;
    case U8:
        d.stackdata.c = va_arg( vl, int );
        break;
    default:
        break;
    }
    return d;
}
