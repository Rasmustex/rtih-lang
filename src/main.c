/*
 * Very simple toy programming language. It is stack based and will probably
only have an interpreted version
 * TODO: Add error printing and -handling
 * TODO: Infinite program length
 * TODO: Tidy op args
 * TODO: Arg handling in main
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
#include <unistd.h>
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
    OP,
    COMMENT
} TOK_TYPE;

struct command {
    enum OP op;
    uint64_t args[10];
    int argc;
};

int sim( struct command *program );

struct command *read_program_from_file( const char *fname );
void print_help( const char* progname );

int main( int argc, const char **argv ) {
    if( argc < 2 ) {
        print_help( argv[0] );
        printf( "Error: no file name specified" );
        exit(1);
    }
    struct command *program = read_program_from_file( argv[1] );
    sim( program );
    free( program );
    return 0;
}

/*
 * ============================================================ simulation mode
 */

void sim_setup_function_array( void (*op[NUM_OPS])( int argc, uint64_t args[10] ) );
void push( int argc, uint64_t args[10] );
void plus();
void minus();
void dump();
void exit_program();

int sim( struct command *program ) {
    void (*op[NUM_OPS])( int argc, uint64_t args[10] );
    sim_setup_function_array( op );
    //assert(!"Simulation mode not yet implemented");
    assert(NUM_OPS == 5 && "Unhandled operations in simulation mode");
    while( 1 ) {
        op[program->op]( program->argc, program->args );
        ++program;
    }
    return 0;
}

void sim_setup_function_array( void (*op[NUM_OPS])( int argc, uint64_t args[10] ) ) {
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

struct command exit_program_op() {
    struct command com = {
        .op = OP_EXIT,
        .argc = 0
    };
    return com;
}

#define STACK_SIZE 10000

uint64_t stack[STACK_SIZE] = {0};
uint64_t *sp = stack;

#define POP_SIM *--sp
#define PUSH_SIM(x) *sp++ = x

inline void push( int argc, uint64_t args[] ) {
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

inline void exit_program() {
    exit(POP_SIM);
    return;
}

#define MAXTOK 100
#define MAXLINE 1000

char* fgetline( FILE *f, char* buf ) {
    register uint64_t n;
    register char c;
    for( n = 0; (c = fgetc( f )) != '\n' && c != EOF; n++ )
        buf[n] = c;
    buf[n + 1] = '\0';
    return buf;
}

struct command *read_program_from_file( const char *fname ) {
    uint64_t temp;
    FILE *f;
    if( !access( fname, F_OK ) ) {
        f = fopen( fname, "r" ); // Open the requested file
    } else {
        printf( "Error: could not access file %s\n", fname );
        exit(1);
    }
    size_t proglen = 10000; // Initial length of operation array
    TOK_TYPE tt; // The type of token that the tokenizer is processing
    char tok[MAXTOK]; // Holds the found token
    char *p = tok;
    register char c;
    struct command *prog = (struct command*)malloc( proglen * sizeof( struct command ) ); // array of the commands that the program will execute
    if( !prog ) {
        printf( "Error: failed to allocate memory for program\n" );
    }
    struct command *pp = prog;

    /* char line[MAXLINE]; */
    /* uint64_t lineno; // Keeps track of the line of tokens */

    while( 1 ) {
        p = tok; // Reset p to the start of token string

        /* TODO: Separate tokenizer to separate function - also make prettier. */
        // TODO: Operating on lines
        while( (c = fgetc( f )) == ' ' || c == '\t' || c == '\n' )
            ;
        if( c == EOF )
            break;
        // Does token start with an alphabetic character? Then it must be a name
        if( isalpha( c ) ) {
            do {
                *p++ = c; // copy token to tok
            } while( isalnum( c = fgetc(f) ) ); // Names are allowed to contain numbers
            tt = NAME;
        } else if( isdigit(c) ) { // If c is a digit, it must be part of a number that should be pushed to the stack
            do {
                *p++ = c; // copy token to toke
            } while( isdigit( c = fgetc( f ) ) );
            tt = NUM;
        } else if ( c == '#' ) { // We've found a comment. Skip to the next line
            while( (c = fgetc( f )) != '\n' && c != EOF )
                ;
            tt = COMMENT;
        } else { // Otherwise it must be an operator. This will probably change in the future
            tt = OP;
        }
        *p = '\0'; // Null-terminate tok
        switch( tt ) {
        case NUM: // If number, push
            *pp = push_op( atol(tok) );
            break;
        case NAME: // if name, check if exit, and then exit. Other names yet to be implemented
            if ( !strcmp( tok, "exit" ) ) {
                *pp = exit_program_op();
            } else {
                printf( "Error: name %s not recognised, and custom names not implemented\n", tok );
                exit(1);
            }
            break;
        case COMMENT:
            break;
        default:
            switch( c ) { // checks what was put into c
            case '+':
                *pp = plus_op();
                break;
            case '-':
                *pp = minus_op();
                break;
            case '.':
                *pp = dump_op();
                break;
            default:
                printf( "Token %s is not recognised\n", tok );
                exit(1);
            }
            break;
        }

        if( tt != COMMENT ) // if it was a comment, no operation was assigned to the program pointer, so it shouldn't be incremented
            pp++;

        if( !(pp - prog < proglen - 1) ) { // reallocate more memory for the program array if we run out - keep a buffer so we can always add the exit op at the end if the user hasn't
            temp = pp - prog;
            if( !(prog = ( struct command*)realloc( prog, (proglen += 1000) * sizeof( struct command ) ) ) ) {
                printf( "Error: program memory reallocation failed\n" );
                exit(1);
            }
            pp = prog + temp; // reset pp to the same position in case the array has moved in memory
        }
    }
    fclose( f );
    if( (pp - 1)->op != OP_EXIT ) {
        *pp++ = push_op(0);
        *pp = exit_program_op();
    } // if there is no exit statement in the program, we just add one ourselves at the end
    return prog;
}

void print_help( const char* progname ) {
    printf( "Usage: %s [ARGS]\n", progname );
    printf( "    %s <filename>\truns the given file with the plang interpreter\n", progname );
    return;
}
