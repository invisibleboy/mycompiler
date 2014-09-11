/* LINTLIBRARY */

/* 10/29/02 REK Adding config.h */
#include <config.h>

#ifdef LACK_SYS5

char * memcpy(char *s1, char *s2, int n)
{
    extern bcopy();
    bcopy(s2, s1, n);
    return s1;
}

char * memset(char *s, int c, int n)
{
    extern bzero();
    register int i;

    if (c == 0) {
	bzero(s, n);
    } else {
	for(i = n-1; i >= 0; i--) {
	    *s++ = c;
	}
    }
    return s;
}

char * strchr(char *s, int c)
{
    extern char *index();
    return index(s, c);
}

char * strrchr(char *s, int c)
{
    extern char *rindex();
    return rindex(s, c);
}


#endif

#ifndef UNIX
#include <stdio.h>

FILE * popen(const char *string, const char *mode)
{
    (void) fprintf(stderr, "popen not supported on your operating system\n");
    return NULL;
}


int pclose(FILE *fp)
{
    (void) fprintf(stderr, "pclose not supported on your operating system\n");
    return -1;
}
#endif

/* put something here in case some compilers abort on empty files ... */
int
util_do_nothing()
{
    return 1;
}
