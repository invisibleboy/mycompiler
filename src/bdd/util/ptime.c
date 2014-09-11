/* LINTLIBRARY */
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <bdd/util.h>

/* backwards compatibility */
long 
ptime()
{
    return util_cpu_time();
}
