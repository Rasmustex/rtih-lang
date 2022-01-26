#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include "../include/operations.h"
#include "../include/simulate_ops.h"
#include "../include/datatypes.h"

void sim_setup_function_array( void (*op[NUM_OPS])( int argc, data args[10] ) ) {
    assert(NUM_OPS == 20 && "Unhandled operations in simulation mode");
    op[OP_PUSH]  = push;
    op[OP_PLUS]  = plus;
    op[OP_MINUS] = minus;
    op[OP_DUMP]  = dump;
    op[OP_EXIT]  = exit_program;
    op[OP_EQ]    = eq;
    op[OP_LT]    = lt;
    op[OP_GT]    = gt;
    op[OP_DUP]   = dup_stack;
    op[OP_SWAP]  = swap;
    op[OP_DROP]  = drop;
    op[OP_ROT]   = rot;
    op[OP_IF]    = iff;
    op[OP_ELSE]  = elsee;
    op[OP_GOTO]  = goto_label;
    op[OP_FUN]   = fun;
    op[OP_CALL]  = call;
    op[OP_RET]   = ret;
    op[OP_END]   = end;
}

#define STACK_SIZE 10000
// stack for simulation
data stack[STACK_SIZE] = {0};
data *sp = stack;
#define POP_SIM *--sp
#define PUSH_SIM(x) *sp++ = x

// Call stack setup
#define CALL_STACK_SIZE 100000
uint64_t call_stack[CALL_STACK_SIZE] = {0};
uint64_t *csp = call_stack;

inline void push( int argc, data args[] ) {
    PUSH_SIM(args[0]);
    return;
}

inline void plus( void ) {
    register data temp = POP_SIM;
    temp = add_data( POP_SIM, temp );
    PUSH_SIM(temp);
    return;
}

inline void minus( void ) {
    register data temp = POP_SIM;
    temp = sub_data( POP_SIM, temp );
    PUSH_SIM(temp);
    return;
}

inline void dump( void ) {
    register data temp = POP_SIM;
    switch( temp.t ) {
    case I64:
        printf( "%ld\n", temp.i );
        break;
    case U64:
        printf( "%lu\n", temp.u );
        break;
    case F64:
        printf( "%g\n", temp.f );
        break;
    case U8:
        printf( "%c", temp.c );
        break;
    default:
        printf( "error: unprintable datatype: %d", temp.t );
        break;
    }
    return;
}

inline void exit_program( void ) {
    exit((POP_SIM).c);
    return;
}

inline void eq( void ) {
    register data tempd = POP_SIM;
    tempd = make_data( U8, eq_data( POP_SIM, tempd ) );
    PUSH_SIM(tempd);
    return;
}

inline void lt( void ) {
    register data tempd = POP_SIM;
    tempd = make_data( U8, lt_data( POP_SIM, tempd ) );
    PUSH_SIM(tempd);
    return;
}

inline void gt( void ) {
    register data tempd = POP_SIM;
    tempd = make_data( U8, gt_data( POP_SIM, tempd ) );
    PUSH_SIM(tempd);
    return;
}

inline void dup_stack( int argc, data args[10] ) {
    assert( argc == 1 && "You need to pass how many times you want to dup" );
    register data temp;
    register data *lsp = sp;
    for( register uint64_t i = 0; i < args[0].u; i++ ) {
        temp = *--lsp;
        PUSH_SIM(temp);
    }
    return;
}

inline void swap( int argc, data args[10] ) {
    assert( argc == 1 && "remember to pass the amount of elements to be swapped" );
    data swap_1[10];
    data swap_2[10];

    register uint64_t i;
    for( i = 0; i < args[0].u; ++i )
        swap_1[i] = POP_SIM;

    for( i = 0; i < args[0].u; ++i )
        swap_2[i] = POP_SIM;

    for( i = args[0].u; i > 0; --i )
        PUSH_SIM(swap_1[i - 1]);

    for( i = args[0].u; i > 0; --i )
        PUSH_SIM(swap_2[i - 1]);
    return;
}

inline void drop( void ) {
    --sp;
    return;
}

inline void rot( void ) {
    register data temp1 = *(sp - 1);
    register data temp2 = *(sp - 2);
    *(sp - 1) = *(sp - 3);
    *(sp - 2) = temp1;
    *(sp - 3) = temp2;
    return;
}

inline void iff( int argc, data args[10] ) {
    assert( argc == 4 && "remember to call link_blocks before simulating" );
    struct command *prog = (struct command *)args[argc - 2].p;
    struct command **p = (struct command **)args[argc - 1].p;
    register data tempd = POP_SIM;
    if( tempd.t != F64 ) {
        if( !tempd.u ) {
            *p = prog + args[0].u;
            if( (*p)->op == OP_ELSE )
                (*p)->args[1].c = 1;
        }
    } else {
        if( !tempd.f ) {
            *p = prog + args[0].u;
            if( (*p)->op == OP_ELSE )
                (*p)->args[1].c = 1;
        }
    }
    return;
}

inline void elsee( int argc, data args[10] ) {
    assert( argc == 4 && "remember to call link_blocks before simulating" );
    struct command *prog = (struct command *)args[argc - 2].p;
    struct command **p = (struct command **)args[argc - 1].p;
    if( !args[1].c )
        *p = prog + args[0].u;
    else
        args[1].c = 0;
    return;
}

// goes to the args[0]th label in the program
inline void goto_label( int argc, data args[10] ) {
    struct command *prog = (struct command *)args[argc - 2].p;
    struct command **p = (struct command **)args[argc - 1].p;
    *p = prog + args[0].u - 1;
    return;
}

inline void fun( int argc, data args[10] ) {
    assert( argc == 4 && "remember to call link_blocks before simulating" );
    struct command *prog = (struct command *)args[argc - 2].p;
    struct command **p = (struct command **)args[argc - 1].p;
    if( !args[1].c ) { // only jumps to end if call arg is 0
        *p = prog + args[0].u;
    } else { // sets call arg to 0 so it doesn't get auto-triggered
        (*p)->args[1].u = 0;
    }
    return;
}

inline void call( int argc, data args[10] ) {
    struct command *prog = (struct command *)args[argc - 2].p;
    struct command **p = (struct command **)args[argc - 1].p;
    *csp++ = *p - prog;
    *p = prog + args[0].u;
    (*p)->args[1].c = 1; // let fun know that it is being called
    return;
}

inline void ret( int argc, data args[10] ) {
    struct command *prog = (struct command *)args[argc - 2].p;
    struct command **p = (struct command **)args[argc - 1].p;
    *p = prog + *--csp;
    return;
}

inline void end( void ) {
    return;
}
