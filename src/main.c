#include <bits/types.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>

#define POP_SIM *--sp
#define PUSH_SIM(x) *sp++ = x

enum OP {
    OP_PUSH,
    OP_PLUS,
    OP_MINUS,
    OP_DUMP,
    OP_EXIT,
    NUM_OPS
};

struct command {
    enum OP op;
    int args[10];
    int argc;
};

int sim( struct command *program );

struct command push_op( int x );
struct command plus_op( void );
struct command minus_op( void );
struct command dump_op( void );
struct command exit_program_op( int exit_code );

int main( int argc, const char** argv ) {
    struct command program[] = { push_op(10), push_op(35), plus_op(), dump_op(), push_op(400), push_op(40), minus_op(), dump_op(), exit_program_op(0) };
    sim( program );
    return 0;
}

/*
 * ============================================================ simulation mode
 */

void sim_setup_function_array( void (*op[NUM_OPS])( int argc, int args[10] ) );
void push( int argc, int args[10] );
void plus();
void minus();
void dump();
void exit_program( int argc, int args[10] );

int sim( struct command *program ) {
    void (*op[NUM_OPS])( int argc, int args[10] );
    sim_setup_function_array( op );
    //assert(!"Simulation mode not yet implemented");
    assert(NUM_OPS == 5 && "Unhandled operations in simulation mode");
    while( 1 ) {
        op[program->op]( program->argc, program->args );
        ++program;
    }
    return 0;
}

void sim_setup_function_array( void (*op[NUM_OPS])( int argc, int args[10] ) ) {
    op[OP_PUSH] = push;
    op[OP_PLUS] = plus;
    op[OP_MINUS] = minus;
    op[OP_DUMP] = dump;
    op[OP_EXIT] = exit_program;
}

struct command push_op( int x ) {
    struct command com = {
        .op = OP_PUSH,
        .args = { x },
        .argc = 1
    };
    return com;
}

struct command plus_op( void ) {
    struct command com = {
        .op = OP_PLUS,
        .argc = 0
    };
    return com;
}

struct command minus_op( void ) {
    struct command com = {
        .op = OP_MINUS,
        .argc = 0
    };
    return com;
}

struct command dump_op( void ) {
    struct command com = {
        .op = OP_DUMP,
        .argc = 0
    };
    return com;
}

struct command exit_program_op( int exit_code ) {
    struct command com = {
        .op = OP_EXIT,
        .args = { exit_code },
        .argc = 1
    };
    return com;
}

__uint64_t stack[10000];
__uint64_t *sp = stack;

void push( int argc, int args[] ) {
    PUSH_SIM(args[0]);
    return;
}

void plus() {
    __uint64_t temp = POP_SIM;
    temp += POP_SIM;
    PUSH_SIM(temp);
    return;
}

void minus() {
    __uint64_t temp = POP_SIM;
    temp = POP_SIM - temp;
    PUSH_SIM(temp);
    return;
}

void dump() {
    printf( "%lu\n", POP_SIM );
    return;
}

void exit_program( int argc, int args[10] ) {
    exit(args[0]);
    return;
}
