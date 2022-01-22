/*
 * Very simple toy programming language. It is stack based and will probably
only have an interpreted version
 * TODO: Move all secondary linking passes through the program to the same loop for optimization
 * TODO: Better error printing and -handling
 * TODO: Tidy op args
 * TODO: More operations
 * TODO: Data types
 * TODO: Pointers
 * TODO: Strings
 * TODO: Tidy into multiple files
 */

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>

// enumerated list of operation types
enum OP {
    OP_PUSH,
    OP_PLUS,
    OP_MINUS,
    OP_DUMP,
    OP_EXIT,
    OP_EQ,
    OP_LT,
    OP_GT,
    OP_DUP,
    OP_SWAP,
    OP_DROP,
    OP_ROT,
    OP_IF,
    OP_END,
    OP_GOTO,
    OP_FUN,
    OP_CALL,
    OP_RET,
    OP_PROGRAM_END,
    NUM_OPS
};

// token types from tokenizer
enum TOK_TYPE {
    NUM,
    WORD,
    OP,
    COMMENT
};

// program commands; contains information needed for exectution
struct command {
    enum OP op;
    int argc;
    uint64_t args[10];
};

typedef struct function func;
struct function {
    uint64_t pos;
    char name[100];
    func *next;
};

// simulate the program, linking blocks and jumping commands
int sim( struct command *program );

// Read and parse program from file fname. Returns program that needs block and jump command linking
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
void lt();
void gt();
void dup_stack( int argc, uint64_t args[10] );
void swap( int argc, uint64_t args[10] );
// drops the top element from stack
void drop();
// rotates top 3 elements of stack left with wraparound
void rot();
void iff( int argc, uint64_t args[10] );
void end();
void goto_label( int argc, uint64_t args[10] );
void fun( int argc, uint64_t args[10] );
void call( int argc, uint64_t args[10] );
void ret( int argc, uint64_t args[10] );

// maps operations to their corresponding spots in the operation array
void sim_setup_function_array( void (*op[NUM_OPS])( int argc, uint64_t args[10] ) ) {
    assert(NUM_OPS == 19 && "Unhandled operations in simulation mode");
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
    op[OP_GOTO]  = goto_label;
    op[OP_FUN]   = fun;
    op[OP_CALL]  = call;
    op[OP_RET]   = ret;
    op[OP_END]   = end;
}

// places pointer to the program and pointer to pointer to current command in the args of jumping commands
void prep_jumping_commands( struct command *program, struct command **p ) {
    struct command *prog = program;
    while( prog->op != OP_PROGRAM_END ) {
        if( prog->op == OP_IF || prog->op == OP_GOTO || prog->op == OP_FUN || prog->op == OP_CALL || prog->op == OP_RET ) {
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
struct command push_op( uint64_t x )             { return make_op( OP_PUSH, 1, &x ); }
// returns command with OP_PLUS
struct command plus_op( void )                   { return make_op( OP_PLUS, 0, NULL ); }
// returns command with OP_MINUS
struct command minus_op( void )                  { return make_op( OP_MINUS, 0, NULL ); }
// returns command with OP_DUMP
struct command dump_op( void )                   { return make_op( OP_DUMP, 0, NULL ); }
// returns command with OP_EXIT
struct command exit_program_op( void )           { return make_op( OP_EXIT, 0, NULL ); }
// returns command with OP_EQ
struct command eq_op( void )                     { return make_op( OP_EQ, 0, NULL ); }
struct command lt_op(void)                       { return make_op(OP_LT, 0, NULL); }
struct command gt_op(void)                       { return make_op(OP_GT, 0, NULL); }
// returns command with OP_DUP and amount of elements to dup as args[0]
struct command dup_stack_op( uint64_t elements ) { return make_op( OP_DUP, 1, &elements ); }
// returns command with OP_SWAP and amount of elements to swap
struct command swap_op( uint64_t elements )      { return make_op( OP_SWAP, 1, &elements ); }
struct command drop_op( void )                   { return make_op( OP_DROP, 0, NULL ); }
struct command rot_op( void )                   { return make_op( OP_ROT, 0, NULL ); }
// returns if op with address of end of block and 3 empty spaces
struct command if_op( uint64_t addr )            {
    uint64_t args[4] = { addr, 0, 0, 0 };
    return make_op( OP_IF, (addr != -1) * 4, args );
}
// returns command with OP_END to indicate end of block
struct command end_op( void )                    { return make_op( OP_END, 0, NULL ); }
// returns command with OP_GOTO and args consisting of: address of label and 2 empty spots
struct command goto_label_op( uint64_t addr )    {
    uint64_t args[3] = { addr, 0, 0 };
    return make_op( OP_GOTO, 3, args );
}
// returns a func operation with end addr as first arg and whether it's called as second
struct command fun_op( uint64_t addr )           {
    uint64_t args[4] = { addr, 0, 0, 0 };
    return make_op( OP_FUN, 4, args );
}
// returns a call operation with the call address as args[0]
struct command call_op( uint64_t addr )          {
    uint64_t args[3] = { addr, 0, 0 };
    return make_op( OP_CALL, 3, args );
}
// returns a return operation
struct command ret_op( void )                          {
    uint64_t args[2] = { 0, 0 };
    return make_op( OP_RET, 2, args );
}
// indicates end of program for functions that crawl the command array
struct command program_end_op( void )            { return make_op( OP_PROGRAM_END, 0, NULL ); }

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

// pushes args[0] to stack
inline void push( int argc, uint64_t args[] ) {
    PUSH_SIM(args[0]);
    return;
}

// adds the 2 top elements of stack
inline void plus( void ) {
    register uint64_t temp = POP_SIM;
    temp += POP_SIM;
    PUSH_SIM(temp);
    return;
}

// subtracts top of stack from 2nd element on stack and pushes result
inline void minus( void ) {
    register uint64_t temp = POP_SIM;
    temp = POP_SIM - temp;
    PUSH_SIM(temp);
    return;
}

// prints top element of stack as uint64
inline void dump( void ) {
    printf( "%lu\n", POP_SIM );
    return;
}

// exits program with the top element on the stack as exit code
inline void exit_program( void ) {
    exit(POP_SIM);
    return;
}

// pops 2 elements from stack and pushes 1 if equal, 0 if not
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

#define MAXTOK 100
#define MAXLABELS 1000

char tok[MAXTOK]; // Holds the found token
int tt; // The type of token that the tokenizer has processed

char labels[MAXLABELS][MAXTOK];
uint64_t label_poses[MAXLABELS];
uint64_t n_labels = 0;

enum TOK_TYPE tokenize( FILE *f, uint64_t *linecount );

func func_head = {
    .pos = 0,
    .next = NULL
};

void add_to_func_list( uint64_t pos, const char *name );
uint64_t find_func_by_name( const char *name, uint8_t *foundfunc );
// TODO: different block syntax because I don't like end
// Reads fname for tokens and parses tokens to create program out of commands
struct command *read_program_from_file( const char *fname ) {
    assert(NUM_OPS == 19 && "Unhandled operations in simulation mode");
    void free_funcs();

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
    uint8_t tempbool;

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
        case '<':
            *pp++ = lt_op();
            break;
        case '>':
            *pp++ = gt_op();
            break;
        case WORD: // if word, check if exit, and then exit. Other words yet to be implemented
            if ( !strcmp( tok, "exit" ) ) {
                *pp++ = exit_program_op();
            } else if( !strcmp( tok, "dup" ) ) {
                *pp++ = dup_stack_op( 1 );
            } else if(!strcmp(tok, "dup2") ) {
                *pp++ = dup_stack_op( 2 );
            } else if( !strcmp( tok, "swap" ) ) {
                *pp++ = swap_op( 1 );
            } else if( !strcmp( tok, "swap2" ) ) {
                *pp++ = swap_op( 2 );
            } else if( !strcmp( tok, "if" ) ) {
                *pp++ = if_op( -1 );
            } else if(!strcmp( tok, "end" ) ) {
                *pp++ = end_op();
            } else if( is_label( tok ) ) {
                strncpy( labels[n_labels], tok, 100 ); // copy label name into labels table
                label_poses[n_labels++] = pp - prog; // set label pos to current op - will be incremented to next op
            } else if( !strcmp(tok, "goto") ) {
                if( tokenize( f, &lineno ) != WORD ) {
                    printf( "%s:%lu: Error: goto: %s is not a valid label name\n", fname, lineno, tok );
                    exit(1);
                }
                *pp++ = goto_label_op( 0 );
            } else if( !strcmp(tok, "fun") ) {
                if( tokenize( f, &lineno ) != WORD ) {
                    printf( "%s:%lu: Error: fun: %s is not a valid function name\n", fname, lineno, tok );
                    exit(1);
                }
                find_func_by_name( tok, &tempbool );
                if( tempbool ) {
                    printf( "%s:%lu: Error: fun: %s is already defined\n", fname, lineno, tok );
                }
                add_to_func_list( pp - prog, tok );
                *pp++ = fun_op( 0 );
            } else if( !strcmp(tok, "call") ) {
                if( tokenize( f, &lineno ) != WORD ) {
                    printf( "%s:%lu: Error: call: %s is not a valid function name\n", fname, lineno, tok );
                    exit(1);
                }
                *pp++ = call_op( 0 );
            } else if( !strcmp(tok, "ret") ) {
                *pp++ = ret_op();
            } else if ( !strcmp(tok, "drop") ) {
                *pp++ = drop_op();
            } else if ( !strcmp(tok, "rot") ) {
                *pp++ = rot_op();
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
    free_funcs();
    return prog;
}

// passes through program and then file if necessary a second time, to make sure goto can see labels ahead of the command. TODO: Merge with the other second program passes
void second_pass( const char *fname, FILE *f, struct command *program ) {
    uint8_t foundfunc = 0;
    fseek( f, 0, SEEK_SET ); // Go back to start of file
    uint64_t gotolineno, calllineno;
    calllineno = gotolineno = 1;
    register uint8_t label_found; // boolean that is set if corresponding label to goto is found
    fpos_t gotopos, callpos;
    fgetpos( f, &gotopos );
    fgetpos( f, &callpos );

    while( program->op != OP_PROGRAM_END ) {
        if( program->op == OP_GOTO ) {
            fsetpos( f, &gotopos );
            while( tokenize( f, &gotolineno ) != EOF ) { // go through file until the corresponding goto is found - first goto stops at first occurence and so on
                if( tt == WORD && !strcmp( tok, "goto" ) ) {
                    tokenize( f, &gotolineno ); // get the goto label
                    for( uint64_t i = 0; i < n_labels; i++ ) { // go through labels
                        if( !strcmp( tok, labels[i] ) ) { // does the label after goto match any known labels?
                            *program = goto_label_op( label_poses[i] ); // if yes, link goto to corresponding label pos
                            label_found = 1;
                            break;
                        }
                    }
                    if( !label_found ) {
                        printf( "%s:%lu: error: goto: %s label doesn't exist", fname, gotolineno, tok );
                        exit(1);
                    }
                    fgetpos( f, &gotopos );
                    break; // if  label found, break out of loop
                }
            }
        } else if( program->op == OP_CALL ) {
            fsetpos( f, &callpos );
            while( tokenize( f, &calllineno ) != EOF ) {
                if( tt == WORD && !strcmp( tok, "call" ) ) {
                    tokenize( f, &calllineno );
                    *program = call_op( find_func_by_name( tok, &foundfunc ) );
                    if( !foundfunc ) {
                        printf( "%s:%lu: error: call: function %s doesn't exist", fname, calllineno, tok );
                        exit(1);
                    }
                    fgetpos( f, &callpos );
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
        for( *p++ = c; ( isalnum( c = fgetc( f ) ) || c == ':' || c == '_' ) && p - tok < MAXTOK - 1; ) // copy c to tok as long as c is alphanumeric or : or _
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
    assert(NUM_OPS == 19 && "Unhandled operations in simulation mode - only need to add block ops here");
    char *find_func_by_pos( uint64_t pos );
    struct command *p = prog;
    uint64_t ifs[IF_STACK_SIZE];
    uint64_t *s = ifs;
    uint64_t fun = 0;
    uint8_t in_fun = 0;
    while( p->op != OP_PROGRAM_END ) {
        if( p->op == OP_IF ) {
            *s++ = p - prog; // add the if to if stack
        } else if ( p->op == OP_FUN ) {
            if( in_fun ) {
                printf( "Error: cannot define function inside other function\n" );
                exit(1);
            }
            in_fun = 1;
            fun = p - prog;
        } else if( p->op == OP_END ) {
            if( s != ifs ) {
                *( prog + *--s ) = if_op( p - prog ); // pop if from if stack and set it to point to the adress of end op
                assert( ( prog + *s )->op == OP_IF && "For now, only if can be put in the if stack" );
            } else if( in_fun ) {
                *(prog + fun) = fun_op( p - prog );
                in_fun = 0;
                fun = 0;
            } else {
                printf( "Error: end with no corresponding block start\n" );
                exit(1);
            }
        }
        ++p;
    }
    if( s - ifs ) {
        printf( "Error: if(s) missing an end block\n" );
        exit( 1 );
    }
    if( in_fun ) {
        printf( "Error: fun %s missing an end block\n", find_func_by_pos( fun ) );
        exit(1);
    }
    return;
}

void add_to_func_list( uint64_t pos, const char *name ) {
    func *f = &func_head;
    while( f->next )
        f = f->next;
    f->next = (func *)malloc( sizeof(func) );
    strncpy( f->next->name, name, MAXTOK );
    f->next->pos = pos;
    f->next->next = NULL;
    return;
}

uint64_t find_func_by_name( const char *name, uint8_t *foundfunc ) {
    func *f = func_head.next;
    while( f ) {
        if( !strcmp( f->name, name ) ) {
            *foundfunc = 1;
            return f->pos;
        }
        f = f->next;
    }
    *foundfunc = 0;
    return 0;
}

void print_funcs() {
    func *f = func_head.next;
    while( f ) {
        printf( "element: %s\n", f->name );
        f = f->next;
    }
}

char *find_func_by_pos( uint64_t pos ) {
    func *f = &func_head;
    while( f->next ) {
        if( f->pos == pos ) {
            return f->name;
        }
        f = f->next;
    }
    return NULL;
}

void free_funcs() {
    if( !func_head.next )
        return;
    func *f = func_head.next;
    func *next;
    while( f ) {
        next = f->next;
        free( f );
        f = next;
    }
    return;
}

void print_help( const char *progname ) {
    printf( "Usage: %s [ARGS]\n", progname );
    printf( "    %s <filename>\truns the given file with the plang interpreter\n", progname );
    return;
}
