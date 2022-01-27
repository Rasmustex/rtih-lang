#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "../include/operations.h"
#include "../include/jump_linker.h"
#include "../include/lexer.h"
#include "../include/parser.h"

void prep_jumping_commands( struct command *program, struct command **p ) {
    struct command *prog = program;
    while( prog->op != OP_PROGRAM_END ) {
        if( prog->op == OP_IF || prog->op == OP_ELSE || prog->op == OP_GOTO || prog->op == OP_FUN || prog->op == OP_CALL || prog->op == OP_RET ) {
            prog->args[prog->argc - 2].p = (void*)program;
            prog->args[prog->argc - 1].p = (void*)p;
        }
        ++prog;
    }
    return;
}

#define IF_STACK_SIZE 100

void link_blocks( struct command *prog ) {
    assert(NUM_OPS == 21 && "Unhandled operations in simulation mode - only need to add block ops here");
    char *find_func_by_pos( uint64_t pos );
    struct command *p = prog;
    uint64_t ifs[IF_STACK_SIZE];
    uint64_t elses[IF_STACK_SIZE];
    uint64_t *is = ifs;
    uint64_t *es = elses;
    uint64_t fun = 0;
    uint8_t in_fun = 0;
    while( p->op != OP_PROGRAM_END ) {
        if( p->op == OP_IF ) {
            *is++ = p - prog; // add the if to if stack
        } else if ( p->op == OP_FUN ) {
            if( in_fun ) {
                printf( "Error: fun %s cannot define function inside other function\n", find_func_by_pos( p - prog ) );
                exit(1);
            }
            in_fun = 1;
            fun = p - prog;
        } else if( p->op == OP_ELSE ) {
            if( is != ifs ) {
                *( prog + *--is ) = if_op( make_data( U64, p - prog ) );
                *es++ = p - prog;
            } else {
                printf( "Error: only one else permitted per if\n" );
                exit(1);
            }
        } else if( p->op == OP_END ) {
            if( es != elses ) {
                *( prog + *--es ) = else_op( make_data( U64, p - prog ) );
                assert( ( prog + *es )->op == OP_ELSE && "For now, only else can be put in the else stack" );
            } else if( is != ifs ) {
                *( prog + *--is ) = if_op( make_data( U64, p - prog ) ); // pop if from if stack and set it to point to the adress of end op
                assert( ( prog + *is )->op == OP_IF && "For now, only if can be put in the if stack" );
            } else if( in_fun ) {
                *(prog + fun) = fun_op( make_data( U64, p - prog ) );
                in_fun = 0;
                fun = 0;
            } else {
                printf( "Error: scope close with no corresponding open\n" );
                exit(1);
            }
        }
        ++p;
    }
    if( is - ifs ) {
        printf( "Error: unclosed if block(s)\n" );
        exit( 1 );
    }
    if( es - elses ) {
        printf( "Error: unclosed else block(s)\n" );
        exit( 1 );
    }
    if( in_fun ) {
        printf( "Error: fun %s has not been closed\n", find_func_by_pos( fun ) );
        exit(1);
    }
    return;
}

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
                            *program = goto_label_op( make_data( U64, label_poses[i] ) ); // if yes, link goto to corresponding label pos
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
                    *program = call_op( make_data( U64, find_func_by_name( tok, &foundfunc ) ) );
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

// declaring the function structure
typedef struct function func;
struct function {
    uint64_t pos;
    char name[100];
    func *next;
};

static func func_head = {
    .pos = 0,
    .next = NULL
};

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

char *find_func_by_pos( uint64_t pos ) {
    func *f = &func_head;
    while( f ) {
        if( f->pos == pos ) {
            return f->name;
        }
        f = f->next;
    }
    return NULL;
}

void free_funcs( void ) {
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
