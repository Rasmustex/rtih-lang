#pragma once
#include "operations.h"
#include "lexer.h"

// reads program from file and links it
struct command *read_program_from_file( const char *fname );

#define MAXLABELS 10000 // defining limit for label amounts in rtih program

extern char labels[MAXLABELS][MAXTOK];
extern uint64_t label_poses[MAXLABELS];
extern uint64_t n_labels;
