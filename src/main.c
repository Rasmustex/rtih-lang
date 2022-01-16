/*
 * Very simple toy programming language. It is stack based and will probably
only have an interpreted version
 * TODO: Add error printing and -handling
 * TODO: File reading
 * TODO: Tokeniser and parser for program reading
 * TODO: More operations
 * TODO: Data types
 * TODO: Pointers
 * TODO: Strings
 * TODO: Tidy into multiple files
 * TODO: Functions/procedures, probably using goto, call and ret
 */

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

enum OP {
    OP_PUSH,
    OP_PLUS,
    OP_MINUS,
    OP_DUMP,
    OP_EXIT,
    NUM_OPS
};

typedef enum {
    NUM,
    NAME,
    OP
} TOK_TYPE;

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

struct command *read_program_from_file( const char *fname );

int main( int argc, const char **argv ) {
    //struct command program[] = { push_op(10), push_op(35), plus_op(), dump_op(), push_op(400), push_op(40), minus_op(), dump_op(), exit_program_op(0) };
    struct command *program = read_program_from_file( "test.p" );
    sim( program );
    free( program );
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
    op[OP_PUSH]  = push;
    op[OP_PLUS]  = plus;
    op[OP_MINUS] = minus;
    op[OP_DUMP]  = dump;
    op[OP_EXIT]  = exit_program;
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

#define STACK_SIZE 10000

uint64_t stack[STACK_SIZE];
uint64_t *sp = stack;

#define POP_SIM *--sp
#define PUSH_SIM(x) *sp++ = x

inline void push( int argc, int args[] ) {
    PUSH_SIM(args[0]);
    return;
}

inline void plus() {
    register uint64_t temp = POP_SIM;
    temp += POP_SIM;
    PUSH_SIM(temp);
    return;
}

inline void minus() {
    register uint64_t temp = POP_SIM;
    temp = POP_SIM - temp;
    PUSH_SIM(temp);
    return;
}

inline void dump() {
    printf( "%lu\n", POP_SIM );
    return;
}

inline void exit_program( int argc, int args[10] ) {
    exit(args[0]);
    return;
}

#define MAXTOK 100

struct command *read_program_from_file( const char *fname ) {
    FILE* f = fopen( fname, "r" );
    int proglen = 1000;
    TOK_TYPE tt;
    char tok[MAXTOK];
    char *p = tok;
    char c;
    struct command *prog = malloc( proglen * sizeof( struct command ) );
    struct command *pp = prog;

    while( !feof( f ) ) {
        p = tok;
        assert( pp - prog < proglen );
        // feof(f)
        while( (c = fgetc( f )) == ' ' || c == '\t' || c == '\n' )
            ;
        if( feof( f ) )
            break;
        if( isalpha( c ) ) {
            do {
                *p++ = c;
            } while( isalpha( c = fgetc(f) ) );
            tt = NAME;
        } else if( isdigit(c) ) {
            do {
                *p++ = c;
            } while( isdigit( c = fgetc( f ) )) ;
            tt = NUM;
        } else {
            tt = OP;
        }
        *p = '\0';
        switch( tt ) {
        case NUM:
            *pp = push_op( atol(tok));
            break;
        case NAME:
            if ( !strcmp( tok, "exit" ) ) {
                *pp = exit_program_op( 0 );
            }
            break;
        default:
            switch( c ) {
            case '+':
                *pp = plus_op();
                break;
            case '-':
                *pp = minus_op();
                break;
            case '.':
                *pp = dump_op();
            }
            break;
        }
        pp++;
    }
    fclose( f );
    return prog;
}
