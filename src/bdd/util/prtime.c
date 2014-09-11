/* LINTLIBRARY */

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <bdd/util.h>


/*
 *  util_print_time -- massage a long which represents a time interval in
 *  milliseconds, into a string suitable for output 
 *
 *  Hack for IBM/PC -- avoids using floating point
 */

char *
util_print_time(unsigned long t)
{
    static char s[40];

    (void) sprintf(s, "%ld.%02ld sec", t/1000, (t%1000)/10);
    return s;
}
