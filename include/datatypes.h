#pragma once

// contains all possible data types of stack data
typedef enum DATA_TYPE {
    I64,
    U64,
    F64,
    U8
} type;

typedef struct sim_stack_data data;

/*
    * accepts a data type and some data (HAS TO MATCH)
    * and returns a stack element with that data and type
    */
data make_data( type t, ... );
