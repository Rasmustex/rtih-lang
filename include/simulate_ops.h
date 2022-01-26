#pragma once
/*
 * This file declares the operations that can be used when simulating the program.
 * It also declares a function that links these operations to slots in an array
 */
#include <stdint.h>
#include "operations.h"
#include "datatypes.h"

void sim_setup_function_array( void (*op[NUM_OPS])( int argc, data args[10] ) );

void push( int argc, data args[10] );
void plus();
void minus();
void dump();
void exit_program();
void eq();
void lt();
void gt();
void dup_stack( int argc, data args[10] );
void swap( int argc, data args[10] );
// drops the top element from stack
void drop();
// rotates top 3 elements of stack left with wraparound
void rot();
void iff( int argc, data args[10] );
void elsee( int argc, data args[10] );
void end();
void goto_label( int argc, data args[10] );
void fun( int argc, data args[10] );
void call( int argc, data args[10] );
void ret( int argc, data args[10] );
