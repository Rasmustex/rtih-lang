#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/lexer.h"

char tok[MAXTOK];
int tt;

// changes escaped escape characters such as \\n to \n in a string
void unescape_characters( char *str, size_t len ) {
    char *copy; // holds copy of str with escaped characters
    char *s, c, *cop;
    s = str;
    if( !( copy = (char*)malloc( len ) ) ) {
        printf( "Error allocating string memory in %s", __func__ );
        exit(1);
    }
    cop = copy;

    while( ( c = *s ) != '\0' ) {
        if( c == '\\' ) {
            switch( c = *++s ) {
            case 'a':
                *cop++ = '\a';
                break;
            case 'b':
                *cop++ = '\b';
                break;
            case 'f':
                *cop++ = '\f';
                break;
            case 'n':
                *cop++ = '\n';
                break;
            case 'r':
                *cop++ = '\r';
                break;
            case 't':
                *cop++ = '\t';
                break;
            case 'v':
                *cop++ = '\v';
                break;
            case '\\':
                *cop++ = '\\';
                break;
            case '\?':
                *cop++ = '\?';
                break;
            case '\'':
                *cop++ = '\'';
                break;
            case '\"':
                *cop++ = '\"';
                break;
            default:
                break;
            }
        } else {
            *cop++ = c;
        }
        ++s;
    }
    *cop = '\0';
    strncpy( str, copy, len );
    free( copy );
    return;
}

enum TOK_TYPE tokenize( FILE *f, uint64_t *linecount ) {
    char *p = tok;
    int dotcounter = 0;
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
    } else if( isdigit(c) || c == '-' ) {
        for( *p++ = c; (isdigit( c = fgetc( f ) ) || c == '.' || c == 'U' || c == 'u' ) && p - tok < MAXTOK - 1; ) {
            if( c == '.' )
                ++dotcounter;
            if( dotcounter > 1 ) {
                printf( "error: too many dots in pushed number\n" );
                exit(1);
            }
            *p++ = c;
        }

        *p++ = '\0';
        ungetc( c, f );
        if( !strcmp( tok, "-" ) )
            return tt = '-';
        else if( dotcounter ) {
            if( *(p - 2) == 'u' || *(p - 2) == 'U' ) {
                printf( "error: %s: floating point numbers cannot be unsigned\n", tok );
                exit(1);
            }
            return tt = FLOAT;
        }
        else if( *(p - 2) == 'u' || *(p - 2) == 'U' )
            return tt = UINT;
        else
            return tt = INT;
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
    } else if( c == '\'' ) {
        for( ; (c = fgetc(f)) != EOF && c != '\'' && p - tok < MAXTOK; *p++ = c )
            ;
        *p++ = '\0';
        unescape_characters( tok, MAXTOK );
        if( c == '\'' )
            return tt = CHAR;
        else
            return tt = TOK_ERROR;
    } else {
        *p++ = c;
        *p = '\0';
        return tt = c;
    }
}
