#pragma once
#include <stdio.h>
#include "operations.h"

/*
 * this file declares the functions and data structures needed to link all
 * the jumping commands in rtih
 */

// give jumping commands pointers to the program
void prep_jumping_commands( struct command *program, struct command **p );

// linking starts of blocks to ends of blocks
void link_blocks( struct command *prog );

/* second pass through file to link gotos and function calls */
void second_pass( const char *fname, FILE *f, struct command *program );

/* function list data structure handling */

// adds a function with a given position and name to end of func list
void add_to_func_list( uint64_t pos, const char *name );
// finds function position by name and uses foundfunc to indicate whether it is found
uint64_t find_func_by_name( const char *name, uint8_t *foundfunc );
// finds function name by position
char *find_func_by_pos( uint64_t pos );
// frees entire func data structure from heap
void free_funcs( void );
