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
 * TODO: Functions using fun, call and ret
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
    OP_DUP,
    OP_IF,
    OP_END,
    OP_GOTO,
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
    void prep_jumping_commands( struct command *program, struct command **p );
    void (*op[NUM_OPS])( int argc, uint64_t args[10] );
    struct command *p = program;
    sim_setup_function_array( op );
    link_blocks( program );
    prep_jumping_commands( program, &p );

    while( 1 ) {
        op[p->op]( p->argc, p->args );
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
void dup_stack( int argc, uint64_t args[10] );
void iff( int argc, uint64_t args[10] );
void goto_label( int argc, uint64_t args[10] );
void end();

// maps operations to their corresponding spots in the operation array
void sim_setup_function_array( void (*op[NUM_OPS])( int argc, uint64_t args[10] ) ) {
    assert(NUM_OPS == 11 && "Unhandled operations in simulation mode");
    op[OP_PUSH]  = push;
    op[OP_PLUS]  = plus;
    op[OP_MINUS] = minus;
    op[OP_DUMP]  = dump;
    op[OP_EXIT]  = exit_program;
    op[OP_EQ]    = eq;
    op[OP_DUP]   = dup_stack;
    op[OP_IF]    = iff;
    op[OP_GOTO]  = goto_label;
    op[OP_END]   = end;
}

// places pointer to the program and pointer to pointer to current command in the args of jumping commands
void prep_jumping_commands( struct command *program, struct command **p ) {
    struct command *prog = program;
    while( prog->op != OP_PROGRAM_END ) {
        if( prog->op == OP_IF || prog->op == OP_GOTO ) {
            prog->args[prog->argc - 2] = (uint64_t)program;
            prog->args[prog->argc - 1] = (uint64_t)p;
        }
        ++prog;
    }
    return;
}

// Maybe this could be void using pointers, allowing for inline?
// creates and returns command struct with desired operation, arg number and args
struct command make_op( enum OP op, int argc, uint64_t *args ) {
    struct command com = {
        .op = op,
        .argc = argc
    };
    for( uint8_t i = 0; i < argc ; i++ )
        com.args[i] = args[i];
    return com;
}

// returns OP_PUSH command with args[0] as the item to be pushed
struct command push_op( uint64_t x )          { return make_op( OP_PUSH, 1, &x ); }
// returns command with OP_PLUS
struct command plus_op( void )                { return make_op( OP_PLUS, 0, NULL ); }
// returns command with OP_MINUS
struct command minus_op( void )               { return make_op( OP_MINUS, 0, NULL ); }
// returns command with OP_DUMP
struct command dump_op( void )                { return make_op( OP_DUMP, 0, NULL ); }
// returns command with OP_EXIT
struct command exit_program_op( void )        { return make_op( OP_EXIT, 0, NULL ); }
// returns command with OP_EQ
struct command eq_op( void )                  { return make_op( OP_EQ, 0, NULL ); }
// returns command with OP_DUP and amount of elements to dup as args[0]
struct command dup_stack_op( uint64_t elements )  { return make_op( OP_DUP, 1, &elements ); }
// returns if op with address of end of block and 3 empty spaces
struct command if_op( uint64_t addr )         {
    uint64_t args[4] = { addr, 0, 0, 0 };
    return make_op( OP_IF, (addr != -1) * 4, args );
}
// returns command with OP_END to indicate end of block
struct command end_op( void )                 { return make_op( OP_END, 0, NULL ); }
// returns command with OP_GOTO and args consisting of: address of label and 2 empty spots
struct command goto_label_op( uint64_t addr ) {
    uint64_t args[3] = { addr, 0, 0 };
    return make_op( OP_GOTO, 3, args );
}
// indicates end of program for functions that crawl the command array
struct command program_end_op( void )         { return make_op( OP_PROGRAM_END, 0, NULL ); }

#define STACK_SIZE 10000

// stack for simulation
uint64_t stack[STACK_SIZE] = {0};
uint64_t *sp = stack;

#define POP_SIM *--sp
#define PUSH_SIM(x) *sp++ = x

// pushes args[0] to stack
inline void push( int argc, uint64_t args[] ) {
    PUSH_SIM(args[0]);
    return;
}

// adds the 2 top elements of stack
inline void plus() {
    register uint64_t temp = POP_SIM;
    temp += POP_SIM;
    PUSH_SIM(temp);
    return;
}

// subtracts top of stack from 2nd element on stack and pushes result
inline void minus() {
    register uint64_t temp = POP_SIM;
    temp = POP_SIM - temp;
    PUSH_SIM(temp);
    return;
}

// prints top element of stack as uint64
inline void dump() {
    printf( "%lu\n", POP_SIM );
    return;
}

// exits program with the top element on the stack as exit code
inline void exit_program() {
    exit(POP_SIM);
    return;
}

// pops 2 elements from stack and pushes 1 if equal, 0 if not
inline void eq() {
    register uint64_t temp = POP_SIM;
    temp = ( temp == POP_SIM );
    PUSH_SIM(temp);
    return;
}

// duplicates args[0] elements on the top of stack
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

// goes to corresponding end(args[0]) when condition is false, otherwise falls through
inline void iff( int argc, uint64_t args[10] ) {
    assert( argc == 4 && "remember to call link_blocks before simulating" );
    struct command *prog = (struct command *)args[argc - 2];
    struct command **p = (struct command **)args[argc - 1];
    if( !POP_SIM )
        *p = prog + args[0];
    return;
}

// goes to the args[0]th label in the program
inline void goto_label( int argc, uint64_t args[10] ) {
    struct command *prog = (struct command *)args[argc - 2];
    struct command **p = (struct command **)args[argc - 1];
    *p = prog + args[0] - 1;
    return;
}

inline void end() {
    return;
}

#define MAXTOK 100
#define MAXLABELS 1000

char tok[MAXTOK]; // Holds the found token
int tt; // The type of token that the tokenizer has processed

char labels[MAXLABELS][MAXTOK];
uint64_t label_poses[MAXLABELS];
uint64_t n_labels = 0;

enum TOK_TYPE tokenize( FILE *f, uint64_t *linecount );
// TODO: different block syntax because I don't like end
// Reads fname for tokens and parses tokens to create program out of commands
struct command *read_program_from_file( const char *fname ) {
    assert(NUM_OPS == 11 && "Unhandled operations in simulation mode");

    void second_pass( const char *fname, FILE *f, struct command *program );
    int is_label( char* word );

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

    while( tokenize( f, &lineno ) != EOF ) {
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
        case WORD: // if word, check if exit, and then exit. Other words yet to be implemented
            if ( !strcmp( tok, "exit" ) ) {
                *pp++ = exit_program_op();
            } else if( !strcmp( tok, "dup" ) ) {
                *pp++ = dup_stack_op( 1 );
            } else if(!strcmp(tok, "dup2") ) {
                *pp++ = dup_stack_op( 2 );
            } else if( !strcmp( tok, "if" ) ) {
                *pp++ = if_op( -1 );
            } else if(!strcmp( tok, "end" ) ) {
                *pp++ = end_op();
            } else if( is_label( tok ) ) {
                strncpy( labels[n_labels], tok, 100 ); // copy label name into labels table
                label_poses[n_labels++] = pp - prog; // set label pos to current op - will be incremented to next op
            } else if( !strcmp(tok, "goto") ) {
                if( tokenize( f, &lineno ) != WORD ) {
                    printf( "%s:%lu: Error: goto: %s is not a valid label name", fname, lineno, tok );
                    exit(1);
                }
                *pp++ = goto_label_op( 0 );
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
    if( (pp - 1)->op != OP_EXIT ) {
        *pp++ = push_op(0);
        *pp++ = exit_program_op();
    } // if there is no exit statement in the program, we just add one ourselves at the end
    *pp = program_end_op(); // Makes sure the block linker doesn't go further than it needs to by signifying end of program
    second_pass( fname, f, prog );
    return prog;
}

void second_pass( const char *fname, FILE *f, struct command *program ) {
    fseek( f, 0, SEEK_SET ); // Go back to start of program
    uint64_t lineno = 1;
    register uint64_t temp;
    while( program->op != OP_PROGRAM_END ) {
        if( program->op == OP_GOTO ) {
            while( tokenize( f, &lineno ) != EOF ) {
                if( tt == WORD && !strcmp( tok, "goto" ) ) {
                    tokenize( f, &lineno );
                    for( uint64_t i = 0; i < n_labels; i++ ) {
                        if( !strcmp( tok, labels[i] ) ) {
                            *program = goto_label_op( label_poses[i] );
                            temp = 1;
                            break;
                        }
                    }
                    if( !temp ) {
                        printf( "%s:%lu: error: goto: %s label doesn't exist", fname, lineno, tok );
                        exit(1);
                    }
                    break;
                }
            }
        }
        program++;
    }
    fclose( f );
    return;
}

// scans file f for next token, storing found tokens in tok, returning the token type and incrementing line count when \n is encountered
enum TOK_TYPE tokenize( FILE *f, uint64_t *linecount ) {
    char *p = tok;
    register char c;
    // TODO: Operating on lines - would let us ignore comments
    while( ( c = fgetc( f ) ) == ' ' || c == '\t' || c == '\n' )
        *linecount += (c == '\n'); // incremnet line count if newline

    // Does token start with an alphabetic character? Then it must be a name
    if( isalpha( c ) ) {
        for( *p++ = c; ( isalnum( c = fgetc( f ) ) || c == ':' ) && p - tok < MAXTOK - 1; ) // copy c to tok as long as c is alphanumeric or : and we haven't exceeded max length
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

// returns 1 and chops : off of word if word is label
int is_label( char *word ) {
    char *w = word;
    while( *w ) // Go to end of word
        ++w;
    if( *(w - 1) == ':' ) {
        *(w - 1) = '\0';
        return 1;
    }
    return 0;
}

#define IF_STACK_SIZE 100

// Link commands that start a block to corresponding end commands in the program
void link_blocks( struct command *prog ) {
    assert(NUM_OPS == 11 && "Unhandled operations in simulation mode - only need to add block ops here");
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
