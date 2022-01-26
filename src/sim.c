#include <assert.h>
#include "../include/operations.h"
#include "../include/jump_linker.h"
#include "../include/simulate_ops.h"



int sim( struct command *program ) {
    void (*op[NUM_OPS])( int argc, data args[10] );
    struct command *p = program;
    sim_setup_function_array( op );
    prep_jumping_commands( program, &p );

    while( 1 ) {
        op[p->op]( p->argc, p->args );
        ++p;
    }
    return 0;
}
