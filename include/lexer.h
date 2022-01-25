#pragma once
/*
 * This header file contains functions and global variables
 * for working with the lexer
 * TODO: split file into lines instead of going a character at a time through whole file
 */

#include <stdio.h>
#include <stdint.h>

enum TOK_TYPE { // possible token types
    NUM,
    WORD,
    OP,
    SCOPE_OPEN,
    SCOPE_CLOSE,
    COMMENT
};

#define MAXTOK 100 // max token size

extern char tok[MAXTOK]; // holds found token
extern int tt; // holds type of found token

// finds next token in file while keeping track of line number
enum TOK_TYPE tokenize( FILE *f, uint64_t *linecount );
