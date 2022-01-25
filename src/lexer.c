#include <ctype.h>
#include "../include/lexer.h"

char tok[MAXTOK];
int tt;

enum TOK_TYPE tokenize( FILE *f, uint64_t *linecount ) {
    char *p = tok;
    register char c;
    // TODO: Operating on lines - would let us ignore comments
    while( ( c = fgetc( f ) ) == ' ' || c == '\t' || c == '\n' )
        *linecount += (c == '\n'); // incremnet line count if newline

    // Does token start with an alphabetic character? Then it must be a name
    if( isalpha( c ) ) {
        for( *p++ = c; ( isalnum( c = fgetc( f ) ) || c == ':' || c == '_' ) && p - tok < MAXTOK - 1; ) // copy c to tok as long as c is alphanumeric or : or _
            *p++ = c;

        *p = '\0'; // null-termination
        ungetc( c, f );
        return tt = WORD;
    } else if( isdigit(c) ) { // If c is a digit, it must be part of a number that should be pushed to the stack
        for( *p++ = c; isdigit( c = fgetc( f ) ) && p - tok < MAXTOK - 1; )
            *p++ = c;

        *p = '\0';
        ungetc( c, f );
        return tt = NUM;
    } else if ( c == '#' ) { // We've found a comment. Skip to the next line
        while( ( c = fgetc( f ) ) != '\n' && c != EOF )
            ;
        *linecount += (c == '\n');
        return tt = COMMENT;
    } else if (c == '{') {
        *p++ = c;
        *p = '\0';
        return tt = SCOPE_OPEN;
    } else if (c == '}') {
        *p++ = c;
        *p = '\0';
        return tt = SCOPE_CLOSE;
    } else {
        *p++ = c;
        *p = '\0';
        return tt = c;
    }
}
