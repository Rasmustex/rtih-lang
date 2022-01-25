#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include "../include/operations.h"
#include "../include/lexer.h"
#include "../include/jump_linker.h"
#include "../include/parser.h"
// returns 1 and chops : off of word if word is label
static uint8_t is_label( char *word ) {
    char *w = word;
    while( *w ) // Go to end of word
        ++w;
    if( *(w - 1) == ':' ) {
        *(w - 1) = '\0';
        return 1;
    }
    return 0;
}

char labels[MAXLABELS][MAXTOK];
uint64_t label_poses[MAXLABELS];
uint64_t n_labels; // initialize to 0

struct command *read_program_from_file( const char *fname ) {
    assert(NUM_OPS == 20 && "Unhandled operations in simulation mode");
    char func_name[MAXTOK];

    void second_pass( const char *fname, FILE *f, struct command *program );

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
        case SCOPE_OPEN:
            printf( "%s:%lu: '%s' error: previous operator is not a valid scope opener\n", fname, lineno, tok );
            exit(1);
            break;
        case SCOPE_CLOSE:
            *pp++ = end_op();
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
                if( tokenize( f, &lineno ) != SCOPE_OPEN ) {
                    printf( "%s:%lu: error: 'if' opens a scope, so you need to use { to indicate the start of this scope\n", fname, lineno );
                    exit(1);
                }
            } else if( !strcmp( tok, "else" ) ) {
                if( (pp - 1)->op != OP_END ) {
                    printf( "%s:%lu: error: you can only use operator '%s' right after the closing of an if scope\n", fname, lineno, tok );
                    exit(1);
                }
                *--pp = else_op( -1 );
                ++pp;
                if( tokenize( f, &lineno ) != SCOPE_OPEN ) {
                    printf( "%s:%lu: error: 'else' opens a scope, so you need to use { to indicate the start of this scope\n", fname, lineno );
                    exit(1);
                }
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
                    exit(1);
                }
                strncpy( func_name, tok, MAXTOK );
                if( tokenize( f, &lineno ) != SCOPE_OPEN ) {
                    printf( "%s:%lu: error: function declaration opens a scope, so you need to use { to indicate the start of this scope\n", fname, lineno );
                    exit(1);
                }
                add_to_func_list( pp - prog, func_name );
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
    link_blocks( prog );
    free_funcs();
    return prog;
}
