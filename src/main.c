/*
 * Very simple toy programming language. It is stack based and will probably
only have an interpreted version
 * TODO: if stack
 * TODO: Better error printing and -handling
 * TODO: Tidy op args
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
    OP_EQ,
    NUM_OPS
};

enum TOK_TYPE {
    NUM,
    WORD,
    OP,
    COMMENT
};

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

int sim( struct command *program ) {
    void (*op[NUM_OPS])( int argc, uint64_t args[10] );
    sim_setup_function_array( op );
    assert(NUM_OPS == 6 && "Unhandled operations in simulation mode");
    while( 1 ) {
        op[program->op]( program->argc, program->args );
        ++program;
    }
    return 0;
}

void push( int argc, uint64_t args[10] );
void plus();
void minus();
void dump();
void exit_program();
void eq();

void sim_setup_function_array( void (*op[NUM_OPS])( int argc, uint64_t args[10] ) ) {
    op[OP_PUSH]  = push;
    op[OP_PLUS]  = plus;
    op[OP_MINUS] = minus;
    op[OP_DUMP]  = dump;
    op[OP_EXIT]  = exit_program;
    op[OP_EQ]    = eq;
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

struct command eq_op() {
    struct command com = {
        .op = OP_EQ,
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

inline void eq() {
    register uint64_t temp = POP_SIM;
    temp = ( temp == POP_SIM );
    PUSH_SIM(temp);
    return;
}

#define MAXTOK 100

int tt; // The type of token that the tokenizer has processed

struct command *read_program_from_file( const char *fname ) {
    enum TOK_TYPE tokenize( FILE *f, char* tok, uint64_t *linecount );
    char tok[MAXTOK]; // Holds the found token

    size_t proglen = 10000; // Initial length of operation array
    struct command *prog = (struct command*)malloc( proglen * sizeof( struct command ) ); // array of the commands that the program will execute
    if( !prog ) {
        printf( "Error: failed to allocate memory for program\n" );
    }
    struct command *pp = prog;

    FILE *f;
    if( !access( fname, F_OK ) ) {
        f = fopen( fname, "r" ); // Open the requested file
    } else {
        printf( "Error: could not access file %s\n", fname );
        exit(1);
    }

    uint64_t lineno = 1; // Keeps track of the line of tokens
    uint64_t temp; // temporarily holds position of pp relative to prog

    while( tokenize( f, tok, &lineno ) != EOF ) {
        switch( tt ) {
        case NUM: // If number, push
            *pp++ = push_op( atol(tok) );
            break;
        case '+':
            *pp++ = plus_op();
            break;
        case '-':
            *pp++ = minus_op();
            break;
        case '.':
            *pp++ = dump_op();
            break;
        case '=':
            *pp++ = eq_op();
            break;
        case WORD: // if name, check if exit, and then exit. Other names yet to be implemented
            if ( !strcmp( tok, "exit" ) ) {
                *pp++ = exit_program_op();
            } else {
                printf( "%s:%lu: Error: word %s not recognised, and custom names not implemented\n", fname, lineno, tok );
                exit(1);
            }
            break;
        case COMMENT:
            break;
        default:
            printf( "%s:%lu: Error: token %s is not recognised\n", fname, lineno, tok );
            exit(1);
            break;
        }

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

enum TOK_TYPE tokenize( FILE *f, char *tok, uint64_t *linecount ) {
    char *p = tok;
    register char c;
    // TODO: Operating on lines - would let us ignore comments
    while( ( c = fgetc( f ) ) == ' ' || c == '\t' || c == '\n' )
        *linecount += (c == '\n'); // incremnet line count if newline

    // Does token start with an alphabetic character? Then it must be a name
    if( isalpha( c ) ) {
        for( *p++ = c; isalnum( c = fgetc( f ) ) && p - tok < MAXTOK - 1; ) // copy c to tok as long as c is alphanumeric and we haven't exceeded max length
            *p++ = c;

        *p = '\0'; // null-termination
        ungetc( c, f );
        return tt = WORD;
    } else if( isdigit(c) ) { // If c is a digit, it must be part of a number that should be pushed to the stack
        for( *p++ = c; isdigit( c = fgetc( f ) ) && p - tok < MAXTOK - 1; )
            *p++ = c;

        *p = '\0';
        ungetc( c, f );
        return tt = NUM;
    } else if ( c == '#' ) { // We've found a comment. Skip to the next line
        while( ( c = fgetc( f ) ) != '\n' && c != EOF )
            ;
        *linecount += (c == '\n');
        return tt = COMMENT;
    } else {
        *p++ = c;
        *p = '\0';
        return tt = c;
    }
}

void print_help( const char* progname ) {
    printf( "Usage: %s [ARGS]\n", progname );
    printf( "    %s <filename>\truns the given file with the plang interpreter\n", progname );
    return;
}
