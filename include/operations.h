#pragma once
#include "datatypes.h"
#include <stdint.h>
/*
 * This header files declares the operations that our
 * rtih program can be made up of as well as some functions
 * to create and interact with them
 */

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
    OP_ELSE,
    OP_END,
    OP_GOTO,
    OP_FUN,
    OP_CALL,
    OP_RET,
    OP_PROGRAM_END,
    NUM_OPS
};

// program commands; contains information needed for execution
struct command {
    enum OP op;
    int argc;
    uint64_t args[10];
};
/*
    * functions to generate operations for the program array - to be overhauled
    */
// returns OP_PUSH command with args[0] as the item to be pushed
struct command push_op( uint64_t x );
struct command plus_op( void );
struct command minus_op( void );
struct command dump_op( void );
struct command exit_program_op( void );
struct command eq_op( void );
struct command lt_op( void );
struct command gt_op( void );
struct command dup_stack_op( uint64_t elements );
struct command swap_op( uint64_t elements );
struct command drop_op( void );
struct command rot_op( void );
struct command if_op( uint64_t addr );
struct command else_op( uint64_t addr );
struct command end_op( void );
struct command goto_label_op( uint64_t addr );
struct command fun_op( uint64_t addr );
struct command call_op( uint64_t addr );
struct command ret_op( void );
struct command program_end_op( void );
