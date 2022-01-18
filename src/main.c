/*
 * Very simple toy programming language. It is stack based and will probably
only have an interpreted version
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
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

enum OP {
    OP_PUSH,
    OP_PLUS,
    OP_MINUS,
    OP_DUMP,
    OP_EXIT,
    OP_EQ,
    OP_DUP,
    OP_IF,
    OP_END,
    OP_PROGRAM_END,
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
    int argc;
    uint64_t args[10];
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


int sim( struct command *program ) {
    void sim_setup_function_array( void (*op[NUM_OPS])( int argc, uint64_t args[10] ) );
    void link_blocks( struct command *program );

    void (*op[NUM_OPS])( int argc, uint64_t args[10] );
    struct command *p = program;
    sim_setup_function_array( op );
    link_blocks( program );

    assert(NUM_OPS == 10 && "Unhandled operations in simulation mode");

    while( 1 ) {
        op[p->op]( p->argc, p->args );
        if( p->op == OP_IF ) {
            /* if( !(p->args[1]) ) */
            p = program + !(p->args[1]) * p->args[0] + p->args[1] * (p - program);
            //p = program + p->args[0];
        }
        ++p;
    }
    return 0;
}

void push( int argc, uint64_t args[10] );
void plus();
void minus();
void dump();
void exit_program();
void eq();
void dup_stack();
void iff( int argc, uint64_t args[10] );
void end();

void sim_setup_function_array( void (*op[NUM_OPS])( int argc, uint64_t args[10] ) ) {
    assert(NUM_OPS == 10 && "Unhandled operations in simulation mode");
    op[OP_PUSH]  = push;
    op[OP_PLUS]  = plus;
    op[OP_MINUS] = minus;
    op[OP_DUMP]  = dump;
    op[OP_EXIT]  = exit_program;
    op[OP_EQ]    = eq;
    op[OP_DUP]   = dup_stack;
    op[OP_IF]    = iff;
    op[OP_END]   = end;
}



// Maybe this could be void using pointers, allowing for inline?
struct command make_op( enum OP op, int argc, uint64_t *args ) {
    struct command com = {
        .op = op,
        .argc = argc
    };
    for( uint8_t i = 0; i < argc ; i++ )
        com.args[i] = args[i];
    return com;
}

struct command push_op( uint64_t x )   { return make_op( OP_PUSH, 1, &x ); }
struct command plus_op( void )         { return make_op( OP_PLUS, 0, NULL ); }
struct command minus_op( void )        { return make_op( OP_MINUS, 0, NULL ); }
struct command dump_op( void )         { return make_op( OP_DUMP, 0, NULL ); }
struct command exit_program_op( void ) { return make_op( OP_EXIT, 0, NULL ); }
struct command eq_op( void )           { return make_op( OP_EQ, 0, NULL ); }
struct command dup_stack_op( void )    { return make_op( OP_DUP, 0, NULL ); }
struct command if_op( uint64_t addr )  {
    uint64_t args[2] = { addr, 0 };
    return make_op( OP_IF, (addr != -1) * 2, args );
};
struct command end_op( void )          { return make_op( OP_END, 0, NULL ); }
struct command program_end_op( void )  { return make_op( OP_PROGRAM_END, 0, NULL ); }

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

inline void dup_stack() {
    register uint64_t temp = *(sp - 1);
    *sp++ = temp;
    return;
}

void iff( int argc, uint64_t args[10] ) {
    assert( argc == 2 && "remember to call link_blocks before simulating" );
    args[1] = POP_SIM != 0;
    return;
}

void end() {
    return;
}

#define MAXTOK 100

int tt; // The type of token that the tokenizer has processed

struct command *read_program_from_file( const char *fname ) {
    assert(NUM_OPS == 10 && "Unhandled operations in simulation mode");

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
            } else if( !strcmp( tok, "dup" ) ) {
                *pp++ = dup_stack_op();
            } else if ( !strcmp( tok, "if" ) ) {
                *pp++ = if_op( -1 );
            } else if (!strcmp(tok, "end") ) {
                *pp++ = end_op();
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

        if( !(pp - prog < proglen - 2) ) { // reallocate more memory for the program array if we run out - keep a buffer so we can always add the exit op at the end if the user hasn't
            temp = pp - prog;
            if( !(prog = (struct command*)realloc( prog, (proglen *= 2) * sizeof( struct command ) ) ) ) {
                printf( "Error: program memory reallocation failed\n" );
                exit(1);
            }
            pp = prog + temp; // reset pp to the same position in case the array has moved in memory
        }
    }
    fclose( f );
    if( (pp - 1)->op != OP_EXIT ) {
        *pp++ = push_op(0);
        *pp++ = exit_program_op();
    } // if there is no exit statement in the program, we just add one ourselves at the end
    *pp = program_end_op(); // Makes sure the block linker doesn't go further than it needs to by signifying end of program
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

#define IF_STACK_SIZE 100

void link_blocks( struct command *prog ) {
    assert(NUM_OPS == 10 && "Unhandled operations in simulation mode - only need to add block ops here");
    struct command *p = prog;
    uint64_t ifs[IF_STACK_SIZE];
    uint64_t *s = ifs;
    while( p->op != OP_PROGRAM_END ) {
        if( p->op == OP_IF ) {
            *s++ = p - prog;
        }
        else if( p->op == OP_END ) {
            *( prog + *--s ) = if_op( p - prog );
            assert( ( prog + *s )->op == OP_IF && "For now, only if can be ended" );
        }
        ++p;
    }
    if( (s - ifs) ) {
        printf( "Error: if(s) missing an end block" );
        exit( 1 );
    }
    return;
}

void print_help( const char* progname ) {
    printf( "Usage: %s [ARGS]\n", progname );
    printf( "    %s <filename>\truns the given file with the plang interpreter\n", progname );
    return;
}
