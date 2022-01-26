#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "../include/datatypes.h"



data make_data( type t, ... ) {
    va_list vl;
    va_start( vl, t );
    data d = {
        .t = t
    };
    switch( t ) {
    case I64:
        d.i = va_arg( vl, int64_t );
        break;
    case U64:
        d.u = va_arg( vl, uint64_t );
        break;
    case F64:
        d.f = va_arg( vl, double );
        break;
    case U8:
        d.c = va_arg( vl, int );
        break;
    case P64:
        d.p = va_arg( vl, void* );
        break;
    default:
        break;
    }
    return d;
}

data add_data( data a, data b ) {
    if( a.t == P64 || b.t == P64 ) {
        printf( "error: cannot add void pointers as of yet\n" );
        exit(1);
    }

    switch( a.t ) {
    case F64:
        if( b.t == U64 || b.t == U8 ) {
            a.f += b.u;
        } else if( b.t == I64 ) {
            a.f += b.i;
        } else if( b.t == F64 ) {
            a.f += b.f;
        }
        break;
    case U64:
        if( b.t == U64 || b.t == U8 ) {
            a.u += b.u;
        } else if( b.t == I64 ) {
            a.u += b.i;
        } else if( b.t == F64 ) {
            a.f = (double)(a.u + b.f);
            a.t = F64;
        }
        break;
    case U8:
        if( b.t == U64 || b.t == U8 ) {
            a.c += b.u;
        } else if( b.t == I64 ) {
            a.c += b.i;
        } else if( b.t == F64 ) {
            a.f = (double)(a.f + b.f);
            a.t = F64;
        }
        break;
    case I64:
        if( b.t == U64 || b.t == U8 ) {
            a.i += b.u;
        } else if( b.t == I64 ) {
            a.i += b.i;
        } else if( b.t == F64 ) {
            a.f = (double)(a.i + b.f);
            a.t = F64;
        }
        break;
    default:
        printf( "error: unimplemented plus\n" );
        exit(1);
        break;
    }
    return a;
}

data sub_data( data a, data b ) {
    if( a.t == P64 || b.t == P64 ) {
        printf( "error: cannot subtract void pointers as of yet\n" );
        exit(1);
    }

    switch( a.t ) {
    case F64:
        if( b.t == U64 || b.t == U8 ) {
            a.f -= b.u;
        } else if( b.t == I64 ) {
            a.f -= b.i;
        } else if( b.t == F64 ) {
            a.f -= b.f;
        }
        break;
    case U64:
        if( b.t == U64 || b.t == U8 ) {
            a.u -= b.u;
        } else if( b.t == I64 ) {
            a.u -= b.i;
        } else if( b.t == F64 ) {
            a.f = (double)(a.u - b.f);
            a.t = F64;
        }
        break;
    case U8:
        if( b.t == U64 || b.t == U8 ) {
            a.c -= b.u;
        } else if( b.t == I64 ) {
            a.c -= b.i;
        } else if( b.t == F64 ) {
            a.f = (double)(a.c - b.f);
            a.t = F64;
        }
        break;
    case I64:
        if( b.t == U64 || b.t == U8 ) {
            a.i -= b.u;
        } else if( b.t == I64 ) {
            a.i -= b.i;
        } else if( b.t == F64 ) {
            a.f = (double)(a.i - b.f);
            a.t = F64;
        }
        break;
    default:
        printf( "error: unimplemented minus\n" );
        exit(1);
        break;
    }
    return a;
}

uint8_t eq_data( data a, data b ) {
    if( a.t == P64 || b.t == P64 ) {
        printf( "error: cannot compare void pointers as of yet\n" );
        exit(1);
    }

    if( a.t != F64 ) {
        if( b.t != F64 ) {
            return a.u == b.u;
        } else {
            return a.u == b.f;
        }
    } else {
        if( b.t != F64 ) {
            return a.f == b.u;
        } else {
            return a.f == b.f;
        }
    }
    return 0;
}

uint8_t lt_data( data a, data b ) {
    if( a.t == P64 || b.t == P64 ) {
        printf( "error: cannot compare void pointers as of yet\n" );
        exit(1);
    }

    if( a.t == U64 || a.t == U8 ) {
        if( b.t == U64 || b.t == U8 ) {
            return a.u < b.u;
        } else if( b.t == I64 ) {
            return a.u < b.i;
        } else {
            return a.u < b.f;
        }
    } else if( a.t == F64 ) {
        if( b.t == U64 || b.t == U8 ) {
            return a.f < b.u;
        } else if( b.t == I64 ) {
            return a.f < b.i;
        } else {
            return a.f < b.f;
        }
    } else {
        if( b.t == U64 || b.t == U8 ) {
            return a.i < b.u;
        } else if( b.t == I64 ) {
            return a.i < b.i;
        } else {
            return a.i < b.f;
        }
    }
    return 0;
}

uint8_t gt_data( data a, data b ) {
    if( a.t == P64 || b.t == P64 ) {
        printf( "error: cannot compare void pointers as of yet\n" );
        exit(1);
    }

    if( a.t == U64 || a.t == U8 ) {
        if( b.t == U64 || b.t == U8 ) {
            return a.u > b.u;
        } else if( b.t == I64 ) {
            return a.u > b.i;
        } else {
            return a.u > b.f;
        }
    } else if( a.t == F64 ) {
        if( b.t == U64 || b.t == U8 ) {
            return a.f > b.u;
        } else if( b.t == I64 ) {
            return a.f > b.i;
        } else {
            return a.f > b.f;
        }
    } else {
        if( b.t == U64 || b.t == U8 ) {
            return a.i > b.u;
        } else if( b.t == I64 ) {
            return a.i > b.i;
        } else {
            return a.i > b.f;
        }
    }
    return 0;
}

/* int64_t cmp_data( data a, data b ) { */
/* } */
