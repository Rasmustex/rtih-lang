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
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../include/operations.h"
#include "../include/parser.h"
#include "../include/sim.h"

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

void print_help( const char *progname ) {
    printf( "Usage: %s [ARGS]\n", progname );
    printf( "    %s <filename>\truns the given file with the plang interpreter\n", progname );
    return;
}
