#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include "../include/operations.h"
#include "../include/simulate_ops.h"

void sim_setup_function_array( void (*op[NUM_OPS])( int argc, uint64_t args[10] ) ) {
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
uint64_t stack[STACK_SIZE] = {0};
uint64_t *sp = stack;
#define POP_SIM *--sp
#define PUSH_SIM(x) *sp++ = x

// Call stack setup
#define CALL_STACK_SIZE 100000
uint64_t call_stack[CALL_STACK_SIZE] = {0};
uint64_t *csp = call_stack;

inline void push( int argc, uint64_t args[] ) {
    PUSH_SIM(args[0]);
    return;
}

inline void plus( void ) {
    register uint64_t temp = POP_SIM;
    temp += POP_SIM;
    PUSH_SIM(temp);
    return;
}

inline void minus( void ) {
    register uint64_t temp = POP_SIM;
    temp = POP_SIM - temp;
    PUSH_SIM(temp);
    return;
}

inline void dump( void ) {
    printf( "%lu\n", POP_SIM );
    return;
}

inline void exit_program( void ) {
    exit(POP_SIM);
    return;
}

inline void eq( void ) {
    register uint64_t temp = POP_SIM;
    temp = ( temp == POP_SIM );
    PUSH_SIM(temp);
    return;
}

inline void lt( void ) {
    register uint64_t temp = POP_SIM;
    temp = POP_SIM < temp;
    PUSH_SIM(temp);
    return;
}

inline void gt( void ) {
    register uint64_t temp = POP_SIM;
    temp = POP_SIM > temp;
    PUSH_SIM(temp);
    return;
}

inline void dup_stack( int argc, uint64_t args[10] ) {
    assert( argc == 1 && "You need to pass how many times you want to dup" );
    register uint64_t temp;
    register uint64_t *lsp = sp;
    for( register uint64_t i = 0; i < args[0]; i++ ) {
        temp = *--lsp;
        PUSH_SIM(temp);
    }
    return;
}

inline void swap( int argc, uint64_t args[10] ) {
    assert( argc == 1 && "remember to pass the amount of elements to be swapped" );
    uint64_t swap_1[10];
    uint64_t swap_2[10];

    register uint64_t i;
    for( i = 0; i < args[0]; ++i )
        swap_1[i] = POP_SIM;

    for( i = 0; i < args[0]; ++i )
        swap_2[i] = POP_SIM;

    for( i = args[0]; i > 0; --i )
        PUSH_SIM(swap_1[i - 1]);

    for( i = args[0]; i > 0; --i )
        PUSH_SIM(swap_2[i - 1]);
    return;
}

inline void drop( void ) {
    --sp;
    return;
}

inline void rot( void ) {
    register uint64_t temp1 = *(sp - 1);
    register uint64_t temp2 = *(sp - 2);
    *(sp - 1) = *(sp - 3);
    *(sp - 2) = temp1;
    *(sp - 3) = temp2;
    return;
}

inline void iff( int argc, uint64_t args[10] ) {
    assert( argc == 4 && "remember to call link_blocks before simulating" );
    struct command *prog = (struct command *)args[argc - 2];
    struct command **p = (struct command **)args[argc - 1];
    if( !POP_SIM ) {
        *p = prog + args[0];
        if( (*p)->op == OP_ELSE )
            (*p)->args[1] = 1;
    }
    return;
}

inline void elsee( int argc, uint64_t args[10] ) {
    assert( argc == 4 && "remember to call link_blocks before simulating" );
    struct command *prog = (struct command *)args[argc - 2];
    struct command **p = (struct command **)args[argc - 1];
    if( !args[1] )
        *p = prog + args[0];
    else
        args[1] = 0;
    return;
}

// goes to the args[0]th label in the program
inline void goto_label( int argc, uint64_t args[10] ) {
    struct command *prog = (struct command *)args[argc - 2];
    struct command **p = (struct command **)args[argc - 1];
    *p = prog + args[0] - 1;
    return;
}

inline void fun( int argc, uint64_t args[10] ) {
    assert( argc == 4 && "remember to call link_blocks before simulating" );
    struct command *prog = (struct command *)args[argc - 2];
    struct command **p = (struct command **)args[argc - 1];
    if( !args[1] ) { // only jumps to end if call arg is 0
        *p = prog + args[0];
    } else { // sets call arg to 0 so it doesn't get auto-triggered
        (*p)->args[1] = 0;
    }
    return;
}

inline void call( int argc, uint64_t args[10] ) {
    struct command *prog = (struct command *)args[argc - 2];
    struct command **p = (struct command **)args[argc - 1];
    *csp++ = *p - prog;
    *p = prog + args[0];
    (*p)->args[1] = 1; // let fun know that it is being called
    return;
}

inline void ret( int argc, uint64_t args[10] ) {
    struct command *prog = (struct command *)args[argc - 2];
    struct command **p = (struct command **)args[argc - 1];
    *p = prog + *--csp;
    return;
}

inline void end( void ) {
    return;
}
