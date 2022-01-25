#include <stdlib.h>
#include "../include/operations.h"

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
struct command rot_op( void )                    { return make_op( OP_ROT, 0, NULL ); }
// returns if op with address of end of block and 3 empty spaces
struct command if_op( uint64_t addr )            {
    uint64_t args[4] =                           { addr, 0, 0, 0 };
    return make_op( OP_IF, (addr != -1) * 4, args );
}
struct command else_op( uint64_t addr )          {
    uint64_t args[4] = { addr, 0, 0, 0 };
    return make_op( OP_ELSE, (addr != -1) * 4, args );
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
struct command ret_op( void )                    {
    uint64_t args[2] = { 0, 0 };
    return make_op( OP_RET, 2, args );
}
// indicates end of program for functions that crawl the command array
struct command program_end_op( void )            { return make_op( OP_PROGRAM_END, 0, NULL ); }
