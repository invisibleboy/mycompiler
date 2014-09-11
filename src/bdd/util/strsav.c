/* LINTLIBRARY */

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <bdd/util.h>


/*
 *  util_strsav -- save a copy of a string
 */
char *
util_strsav(char *s)
{
    return strcpy(ALLOC(char, strlen(s)+1), s);
}
