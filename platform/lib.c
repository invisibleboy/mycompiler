/*****************************************************************************\
 *
 *		      Illinois Open Source License
 *                     University of Illinois/NCSA
 *                         Open Source License
 *
 * Copyright (c) 2004, The University of Illinois at Urbana-Champaign.
 * All rights reserved.
 *
 * Developed by:             
 *
 *		IMPACT Research Group
 *
 *		University of Illinois at Urbana-Champaign
 *
 *              http://www.crhc.uiuc.edu/IMPACT
 *              http://www.gelato.org
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal with the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimers.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimers in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * Neither the names of the IMPACT Research Group, the University of
 * Illinois, nor the names of its contributors may be used to endorse
 * or promote products derived from this Software without specific
 * prior written permission.  THE SOFTWARE IS PROVIDED "AS IS",
 * WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
 * OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS WITH THE SOFTWARE.
 *
\*****************************************************************************/
/*****************************************************************************\
 *      File:   lib.c  for  generic platform (with platform-specific defines)
 *      Author: Ben-Chung Cheng and Wen-mei Hwu
 *      Copyright (c) 1999 Ben-Chung Cheng, Wen-mei Hwu
 *		and The Board of Trustees of the University of Illinois.
 *			All rights reserved.
 *      Purpose: Let IMPACT represent the side-effect of library functions. 
 *               So, functions in this file are compiled by IMPACT, instead 
 *               of CC.
 *      Update: 10/23/01, sryoo
 *               Integrated the various machine-specific lib.c into single file
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include <config.h>

/* Machine flags (in the preprocessor, not config/*.cf): 
	IA64LIN_SOURCE - Itanium Linux
	X86LIN_SOURCE - X86 Linux
	_HPUX_SOURCE
	_SOLARIS_SOURCE - warning: needs works!
	WIN32 - NT
	__linux - either Itanium or X86 Linux
*/

/* 07/02/02 REK Adding definition for getopt_long */
/* 07/24/02 REK Adding definition for lstat */
/* 08/06/02 REK Adding definition for stat64, lstat64, open64, and fopen64 */
  
/* 08/06/02 REK Defining _LARGEFILE64_SOURCE to get the 64 bit definitions
 *              for stat64, etc.
 */
#if defined X86LIN_SOURCE || defined IA64LIN_SOURCE
#define _LARGEFILE64_SOURCE 1
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>
#ifdef HAVE_LIBCURSES
#include <curses.h>
#endif
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/timeb.h>
#include <pwd.h>
#include <wchar.h>
#include <search.h>
#include <fnmatch.h>
#include <syslog.h>
#include <shadow.h>
#include <grp.h>
#include <utmp.h>
#include <glob.h>
#include <malloc.h>

#ifdef __linux
/* 07/02/02 REK #including getopt.h to get the definition of struct option */
#include <dlfcn.h>
#include <getopt.h>
/* 08/06/02 REK #including fcntl.h to get the definition of open64. */
#include <sys/mman.h>
#include <sys/time.h>
#include <netinet/in.h>
#ifndef IA64LIN_SOURCE
#include <obstack.h>
#endif
#include <libintl.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#ifdef HAVE_POPT_H
#include <popt.h>
#endif
#endif

#include <sys/times.h>
#include <sys/resource.h>

#ifndef WIN32
#include <sys/shm.h>
#if defined HAVE_LIBJPEG || defined _HPUX_SOURCE
#include <jpeglib.h>
#endif
#endif

#ifdef HAVE_LIBGL
#include <GL/gl.h>
#endif

#ifdef HAVE_LIBSSL
#include <openssl/bn.h>
#include <openssl/crypto.h>
#include <openssl/dsa.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/md5.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rc4.h>
#include <openssl/rsa.h>
#endif

#ifdef HAVE_LIBZ
#include <zlib.h>
#endif

#if defined _SOLARIS_SOURCE || defined WIN32
#include <utime.h>
#endif

#ifndef _HPUX_SOURCE
#include <sys/socket.h> /* not in HPUX - removes creat() */
#include <netdb.h> /* not in HPUX - conflicting defines with htonl, etc */
#include <argz.h>
#include <wctype.h>
#else
#include <netinet/in.h>

/* Define the iovec struct. */
struct iovec
{
  caddr_t iov_base;
  int iov_len;
};

/* Define the msghdr struct that is defined in sys/socket.h */
struct msghdr
{
  void *msg_name;
  size_t msg_namelen;
  struct iovec *msg_iov;
  int msg_iovlen;
  void *msg_control;
  size_t msg_controllen;
  int msg_flags;
};
#endif



#ifdef __linux
struct __dirstream {
    int i;
};

#if 0
/* conflicts with resource.h */
struct rlimit
{
    int  rlim_cur;
    int  rlim_max;
};
#endif

struct utimbuf {
    time_t actime;  /* access time */
    time_t modtime; /* modification time */
};

#if 0
struct timeval {
    long    tv_sec;         /* seconds */
    long    tv_usec;        /* microseconds */
};
#endif
#endif

__assert()
{
}

/* assert_fail not defined for WIN32 or HPUX */
__assert_fail()
{
}

/* not defined for HPUX */
int *__errno_location()
{
}

#ifdef HAVE_LIBSSL
char *ERR_error_string(unsigned long e, char *ret)
{
  return ret;
}

unsigned long ERR_get_error(void)
{
}

void ERR_load_crypto_strings(void)
{
}

EVP_CIPHER *EVP_bf_cbc()
{
  EVP_CIPHER *result;

  return (result);
}

EVP_CIPHER *EVP_cast5_cbc()
{
  EVP_CIPHER *result;

  return (result);
}

EVP_CIPHER *EVP_des_cbc()
{
  EVP_CIPHER *result;

  return (result);
}

EVP_CIPHER *EVP_des_ede3_cbc()
{
  EVP_CIPHER *result;

  return (result);
}

EVP_CIPHER *EVP_enc_null()
{
  EVP_CIPHER *result;

  return (result);
}

const EVP_MD *EVP_get_digestbyname(const char *name)
{
  EVP_MD *result;
  int i;

  for (i = 0; name[i]; i++);

  return (result);
}    

int EVP_CIPHER_CTX_cleanup(EVP_CIPHER_CTX *a)
{
  int i;

  for (i = 0; ((char *)a)[i], i < sizeof(EVP_CIPHER_CTX); i++);

  free(a);
}

void EVP_CIPHER_CTX_init(EVP_CIPHER_CTX *a)
{
  memset (a, '\0', sizeof(EVP_CIPHER_CTX));
}

int EVP_CIPHER_CTX_set_key_length(EVP_CIPHER_CTX *x, int keylen)
{
  int i;

  for (i = 0; ((char *)x)[i], i < sizeof(EVP_CIPHER_CTX); i++);
}

int EVP_CipherInit(EVP_CIPHER_CTX *ctx, const EVP_CIPHER *type,
		   unsigned char *key, unsigned char *iv, int enc)
{
  int i;

  for (i = 0; ((char *)ctx)[i], i < sizeof(EVP_CIPHER_CTX); i++);
  for (i = 0; ((char *)type)[i], i < sizeof(EVP_CIPHER); i++);
  for (i = 0; key[i]; i++);
  for (i = 0; iv[i]; i++);
}

void EVP_DigestInit(EVP_MD_CTX *ctx, const EVP_MD *type)
{
  ctx=(EVP_MD_CTX *)malloc(sizeof(EVP_MD_CTX));
  ctx->digest=type;
}

void EVP_DigestFinal(EVP_MD_CTX *ctx, unsigned char *md, unsigned int *s)
{
  md[0]='\0';
  *s=0;
}

void EVP_DigestUpdate(EVP_MD_CTX *ctx, const void *d, unsigned int cnt)
{
  int i;
  
  for (i = 0; d[i], i < cnt; i++);
}

EVP_MD *EVP_md5(void)
{
  return((EVP_MD *)malloc(sizeof(EVP_MD)));
}

void EVP_PKEY_free(EVP_PKEY *pkey)
{
}

DSA *EVP_PKEY_get1_DSA(EVP_PKEY *pkey)
{
  int i;

  for (i = 0; ((char *)pkey)[i], i < sizeof(EVP_PKEY); i++);

  return NULL;
}

RSA *EVP_PKEY_get1_RSA(EVP_PKEY *pkey)
{
  int i;

  for (i = 0; ((char *)pkey)[i], i < sizeof(EVP_PKEY); i++);

  return NULL;
}

EVP_CIPHER *EVP_rc4()
{
  EVP_CIPHER *result;

  return (result);
}

EVP_MD *EVP_ripemd160()
{
  EVP_MD *result;

  return (result);
}

EVP_MD *EVP_sha1(void)
{
  return((EVP_MD *)malloc(sizeof(EVP_MD)));
}
#endif

#ifdef _SOLARIS_SOURCE
_filbuf(FILE *file)
{
    FILE local;

    *file = local;
}
#endif

__filbuf(FILE *file)
{
    FILE local;

    *file = local;
}

#ifdef _SOLARIS_SOURCE
_flsbuf(int c, FILE *file)
{
    FILE local;

    *file = local;
}
#endif

#ifdef WIN32
_sffilbuf(FILE *file, int i)
{
    FILE local;

    *file = local;
}
#endif

__flsbuf(unsigned char c, FILE *file)
{
    FILE local;

    *file = local;
}

__va_start()
{
}

void _exit(int status)
{
}

_fxstat()
{
}

_lxstat()
{
}

#ifdef WIN32
int _isctype(int c, int t)
{
}

int _setecho(int b)
{
}
#endif

#if (!defined WIN32 && !defined _tolower) || defined _HPUX_SOURCE 

/* NT: Macro in ctype.h */
_tolower(int c)
{
}

_toupper(int c)
{
}
#endif

_xmknod()
{
}

#ifdef __linux
char *__xpg_basename(char *path)
{
  int i;

  for (i = 0; path[i]; i++);
}
#endif

_xstat()
{
}

void abort()
{
}

abs(int i)
{
}

#ifdef _HPUX_SOURCE
int accept(int s, void *addr, int *addrlen)
#else
int accept(int s, struct sockaddr *addr, socklen_t *addrlen)
#endif
{
  char *t=(char *)addr;
  int i;

  for ( i = 0; t[i]='\0', i < *addrlen; i++);

  *addrlen = 0;
}

int access(const char *path, int amode)
{
    int i;

    for (i = 0; path[i]; i++);
}

double acos(double x)
{
}

double acosh(double x)
{
}

unsigned int alarm(unsigned int seconds)
{
}

#ifndef _HPUX_SOURCE
size_t __argz_count(const char *__argz, size_t __len)
{
  int i;

  for (i = 0; __argz[i], i < __len; i++);
}

char *__argz_next(const char *__argz, size_t __argz_len, const char *__entry)
{
  int i;

  for (i = 0; __argz[i], i < __argz_len; i++);

  for (i = 0; __entry[i]; i++);
}

void __argz_stringify(char *__argz, size_t __len, int __sep)
{
  int i;

  for (i = 0; __argz[i], i < __len; i++);
}
#endif

char *asctime(const struct tm *timeptr)
{
  int i;

  i = timeptr->tm_sec;
  i = timeptr->tm_min;
  i = timeptr->tm_hour;
  i = timeptr->tm_mday;
  i = timeptr->tm_mon;
  i = timeptr->tm_year;
  i = timeptr->tm_wday;
  i = timeptr->tm_yday;
  i = timeptr->tm_isdst;
}

/* alloca() is very messy: HPUX has some strange things going on with it that 
   require a separate section. */ 

#ifdef _HPUX_SOURCE
#ifdef alloca
#undef alloca
#endif
void *alloca(size_t size)
{
}
#elif defined __linux
#ifdef alloca 
#undef alloca
#endif
void *alloca(size_t size)
{
}
#endif

double asin(double x)
{
}

double asinh(double x)
{
}

#ifdef __linux
int asprintf(char **strp, const char *fmt, ...)
{
  int i;
  va_list ap;
  char *p;

  *strp=(char *)malloc(sizeof(char));

  va_start (ap, fmt);

  for (i = 0; fmt[i]; i++);

  **strp='\0';

  while ((p = va_arg (ap, char *)))
    *p;

  va_end(ap);
}
#endif

void assert(int i)
{
}

double atan(double x)
{
}

double atan2(double y, double x)
{
}

double atanh(double x)
{
}

int atexit(void (*func)(void))
{
}

double atof(const char *str)
{
    int i;

    for (i = 0; str[i]; i++);
}

int atoi(const char *str)
{
    int i;

    for (i = 0; str[i]; i++);
}

long atol(const char *str)
{
    int i;

    for (i = 0; str[i]; i++);
}

long long atoll(const char *str)
{
  int i;
  
  for (i = 0; str[i]; i++);
}

#ifdef __linux
int bcmp(const void *s1, const void *s2, size_t n)
{
    int i;

    for (i = 0; i < n; i++) {
        if (((char *) s1)[i] != ((char *) s2)[i])
            return 1;
    }
    return 0;
}
#else
int bcmp(char *s1, char *s2, int n)
{
    int i;

    for (i = 0; i < n; i++) {
	if (s1[i] != s2[i])
	    return 1;
    }
    return 0;
}
#endif 

#ifdef __linux
void bcopy (__const __ptr_t s1, __ptr_t s2, size_t n)
{
    int i;

    for (i = 0; i < n; i++) {
        ((char *) s2)[i] = ((char *) s1)[i];
    }
}
#else
void bcopy(char *s1, char *s2, int n)
{
    int i;

    for (i = 0; i < n; i++) {
	s2[i] = s1[i];
    }
}
#endif

#ifdef _HPUX_SOURCE
int bind(int s, const void *my_addr, int addrlen)
#else
int bind(int sockfd, const struct sockaddr *my_addr, socklen_t addrlen)
#endif
{
  char *t=(char *)my_addr;
  int i;

  for (i = 0; t[i], i < addrlen; i++);
}

#ifdef __linux
char *bindtextdomain(const char *domainname, const char *dirname)
{
  int i;

  for (i = 0; domainname[i]; i++);
  for (i = 0; dirname[i]; i++);
}
#endif

#ifdef HAVE_LIBSSL
int BN_add_word(BIGNUM *a, unsigned long w)
{
  int i;

  for (i = 0; ((char *)a)[i], i < sizeof(BIGNUM); i++);
}

BIGNUM *BN_bin2bn(const unsigned char *s, int len, BIGNUM *ret)
{
  int i;

  for (i = 0; s[i], i < len; i++);
  for (i = 0; ((char *)ret)[i], i < sizeof(BIGNUM); i++);

  return(ret);
}

int BN_bn2bin(const BIGNUM *a, unsigned char *to)
{
  int i;

  for (i = 0; i < BN_num_bytes(a); i++)
    to[i]='\0';

  for (i = 0; ((char *)a)[i], i < sizeof(BIGNUM); i++);
}

char *BN_bn2dec(const BIGNUM *a)
{
  char *result;
  int i;
  char c;

  for (i = 0; i < sizeof (BIGNUM); i++)
    c = ((char *)a)[i];

  return (result);
}

void BN_clear_free(BIGNUM *a)
{
  free(a);
}

int BN_cmp(const BIGNUM *a, const BIGNUM *b)
{
  int i;

  for (i = 0; ((char *)a)[i], ((char *)b)[i], i< sizeof(BIGNUM); i++);
}

void BN_CTX_free(BN_CTX *c)
{
  free(c);
}

BN_CTX *BN_CTX_new(void)
{
  return((BN_CTX *)malloc(sizeof(BN_CTX)));
}

int BN_dec2bn(BIGNUM **a, const char *str)
{
  int i;

  for (i = 0; str[i]; i++);

  if (!*a)
    *a = (BIGNUM *)malloc (sizeof (BIGNUM));
  memset (*a, '\0', sizeof (BIGNUM));
}

BIGNUM *BN_dup(const BIGNUM *a)
{
  BIGNUM *result = malloc (sizeof (BIGNUM));

  return (memcpy (result, a, sizeof (BIGNUM)));
}

int BN_hex2bn(BIGNUM **bn, const char *a)
{
  int i;
  char c;

  c=*a;

  for (i = 0; i < sizeof (BIGNUM); i++)
    c = ((char *)(*bn))[i];
}

int BN_lshift(BIGNUM *r, const BIGNUM *a, int n)
{
  int i;

  for (i = 0; ((char *)a)[i], i < sizeof(BIGNUM); i++);

  memset(r, '\0', sizeof(BIGNUM));
}

int BN_mask_bits(BIGNUM *a, int n)
{
  int i;
  char c;

  for (i = 0; i < sizeof(BIGNUM); i++)
    {
      c = ((char *)a)[i];
    }
}

int BN_mod(BIGNUM *rem, const BIGNUM *m, const BIGNUM *d, BN_CTX *ctx)
{
  int i;

  for (i = 0; i < sizeof(BIGNUM); i++)
    {
      ((char *)m)[i];
      ((char *)d)[i];
    }
  for (i = 0; ((char *)ctx)[i], i < sizeof(BN_CTX); i++);

  memset(rem, '\0', sizeof(BIGNUM));
}

BIGNUM *BN_new(void)
{
  return((BIGNUM *)malloc(sizeof(BIGNUM)));
}

int BN_num_bits(const BIGNUM *a)
{
  int i;

  for (i = 0; ((char *)a)[i], i < sizeof(BIGNUM); i++);
}

int BN_rand(BIGNUM *rnd, int bits, int top, int bottom)
{
  memset (rnd, '\0', sizeof (BIGNUM));
}

int BN_set_word(BIGNUM *a, BN_ULONG w)
{
  int i;

  for (i = 0; ((char *)a)[i], i < sizeof(BIGNUM); i++);
}

int BN_sub(BIGNUM *r, const BIGNUM *a, const BIGNUM *b)
{
  int i;

  for (i = 0; i < sizeof(BIGNUM); i++)
    {
      ((char *)a)[i];
      ((char *)b)[i];
    }

  memset(r, '\0', sizeof(BIGNUM));
}

BIGNUM *BN_value_one(void)
{
  BIGNUM local;

  return(&local);
}
#endif

void *bsearch(const void *key, const void *base, size_t nmemb, size_t size,
	      int (*compar)(const void *, const void *))
{
  char *t=(char *)base;
  int i;

  for (i = 0; t[i], i < nmemb*size; i++);
}

wint_t btowc(int c)
{
}

#ifdef __linux
void bzero(void *s, size_t n)
{
    int i;

    for (i = 0; i < n; i++) {
        ((char *) s)[i] = 0;
    }
}
#else
void bzero(char *s, int n)
{
    int i;

    for (i = 0; i < n; i++) {
	s[i] = 0; 
    }
}
#endif

void *calloc(size_t nelem, size_t elsize)
{
    return 0;
}

double cbrt(double x)
{
}

double ceil(double x)
{
}

speed_t cfgetispeed(const struct termios *termios_p)
{
  int i;

  for (i = 0; ((char *)termios_p)[i], i < sizeof(struct termios); i++);
}

speed_t cfgetospeed(const struct termios *termios_p)
{
  int i;

  for (i = 0; ((char *)termios_p)[i], i < sizeof(struct termios); i++);
}

int cfsetispeed(struct termios *termios_p, speed_t speed)
{
  int i;

  for (i = 0; ((char *)termios_p)[i], i < sizeof(struct termios); i++);
}

int cfsetospeed(struct termios *termios_p, speed_t speed)
{
  int i;

  for (i = 0; ((char *)termios_p)[i], i < sizeof(struct termios); i++);
}

#ifdef __linux
void cfree(void *ptr)
#else
void cfree(void *ptr, int n, int size)
#endif
{
    int i;

    ((char *) ptr)[i] = 0;
}

int chdir(const char *path)
{
    int i;

    for (i = 0; path[i]; i++);
}

int chmod(const char *path, mode_t mode)
{
    int i;

    for (i = 0; path[i]; i++);
}

#ifdef __linux
int chown(const char *path, uid_t owner, gid_t group)
{
    int i;

    for (i = 0; path[i]; i++);
}
#else
int chown(const char *path, int owner, int group)
{
    int i;

    for (i = 0; path[i]; i++);
}
#endif

int chroot(const char *path)
{
  int i;

  for (i = 0; path[i]; i++);
}

#ifndef _HPUX_SOURCE
/* Not present in HPUX: previously defined */
void clearerr(FILE *file)
{
    FILE local;

    *file = local;
}
#endif

#ifdef HAVE_LIBCURSES
#ifdef __linux
int clearok(WINDOW *win, bool bf)
#else
int clearok(WINDOW *win, int bf)
#endif
{
    WINDOW local;

    *win = local;
}
#endif

clock_t clock()
{
}

void tzset (void)
{
}

int close(int fildes)
{
}

int closedir(DIR *dirp)
{
    DIR local;

    *dirp = local;
}

void closelog(void)
{
}

#ifdef __linux
int connect(int s, const struct sockaddr *addr, socklen_t addrlen)
#else
int connect(int s, const void *addr, int addrlen)
#endif
{
    int i;

    for (i = 0; ((char *)addr)[i]; i++);
}

double cos(double x)
{
}

double cosh(double x)
{
}

#ifdef _HPUX_SOURCE
int creat(const char *path, mode_t mode)
#else
int creat(const char *path, int mode)
#endif
{
    int i;

    for (i = 0; path[i]; i++);
}

char *crypt(const char *key, const char *salt)
{
  int i;

  for (i = 0; key[i]; i++);
  for (i = 0; salt[i]; i++);
}

#ifdef HAVE_LIBSSL
void CRYPTO_free(void *v)
{
  free(v);
}
#endif

#if defined _HPUX_SOURCE || defined __linux
char *ctime(const time_t *timer)
{
    time_t local;

    local = *timer;
}
#else
ctime(const time_t *timer)
{
    time_t local;

    local = *timer;
}
#endif

#ifdef __linux
size_t __ctype_get_mb_cur_max()
{
}
#endif

#ifdef HAVE_LIBZ
int deflate(z_streamp strm, int flush)
{
  int i;

  for (i = 0; ((char *)strm)[i], i < sizeof(z_stream); i++);
}

int deflateEnd(z_streamp strm)
{
  int i;

  for (i = 0; ((char *)strm)[i], i < sizeof(z_stream); i++);
}

int deflateInit_(z_streamp strm, int level, const char *version,
		 int stream_size)
{
  int i;

  for (i = 0; ((char *)strm)[i], i < sizeof(z_stream); i++);
}
#endif

int dlclose(void *handle)
{
}

#ifndef __linux
const char *dlerror(void)
#else
char *dlerror(void)
#endif
{
}

void *dlopen(const char *filename, int flag)
{
  int i;

  for (i = 0; filename[i]; i++);
}

void *dlsym(void *handle, const char *symbol)
{
  int i;

  for (i = 0; symbol[i]; i++);
}

#ifdef __linux
int daemon(int nochdir, int noclose)
{
}
#endif

char *dirname(char *path)
{
  int i;

  for (i = 0; path[i]; i++);
}

double drem(double x, double y)
{
}

#ifdef HAVE_LIBSSL
void DH_free(DH *r)
{
  free(r);
}

DH *DH_new()
{
  return ((DH *)malloc (sizeof (DH)));
}

DSA_SIG *DSA_do_sign(const unsigned char *dgst, int dlen, DSA *dsa)
{
  int i;
  char c;
  DSA_SIG *result;

  for (i = 0; i < dlen; i++)
    c = dgst[i];

  for (i = 0; i < sizeof (DSA); i++)
    c = ((char *)dsa)[i];

  return (result);
}

int DSA_do_verify(const unsigned char *dgst, int dgst_len, DSA_SIG *sig,
		  DSA *dsa)
{
  int i;
  char c;

  for (i = 0; i < dgst_len; i++)
    c = dgst[i];

  for (i = 0; i < sizeof (DSA_SIG); i++)
    c = ((char *)sig)[i];

  for (i = 0; i < sizeof (DSA); i++)
    c = ((char *)dsa)[i];
}

void DSA_free(DSA *r)
{
  free(r);
}

int DSA_generate_key(DSA *dsa)
{
  int i;
  char c;

  for (i = 0; i < sizeof (DSA); i++)
    c = ((char *)dsa)[i];
}

DSA *DSA_generate_parameters(int bits, unsigned char *seed_in, int seed_len,
			     int *counter_ret, unsigned long *h_ret,
			     void (*callback)(int, int, void *),
			     void *cb_arg)
{
  int i;
  char c;

  for (i = 0; i < seed_len; i++)
    c = seed_in[i];

  *counter_ret = 0;
  *h_ret = 0;
}

DSA *DSA_new(void)
{
  return ((DSA *)malloc (sizeof (DSA)));
}

void DSA_SIG_free(DSA_SIG *a)
{
  free (a);
}

DSA_SIG *DSA_SIG_new(void)
{
  return ((DSA_SIG *)malloc (sizeof (DSA_SIG)));
}
#endif

double dtime()
{
}

int dup(int oldfd)
{
}

int dup2(int oldfd, int newfd)
{
}

#ifndef WIN32
/* NT: macro in ast/term.h */
int echo()
{
}
#endif

void endgrent()
{
}

void endpwent()
{
}

void endservent()
{
}

int endwin()
{
}

double erf(double x)
{
}

double erfc(double x)
{
}

/* See InitVarargInfo in Psplit/split.c for system vararg fns. */
int execl(const char *path, const char *arg0, ...)
{
  int i;
  va_list ap;
  char *p;

  va_start (ap, arg0);

  for (i = 0; path[i]; i++);
  for (i = 0; arg0[i]; i++);

  while ((p = va_arg (ap, char *)))
    *p;

  va_end(ap);
}

/* See InitVarargInfo in Psplit/split.c for system vararg fns. */
int execle(const char *path, const char *arg0, ...)
{
  int i;
  va_list ap;
  char *p;

  va_start (ap, arg0);

  for (i = 0; path[i]; i++);
  for (i = 0; arg0[i]; i++);

  while ((p = va_arg (ap, char *)))
    *p;

  va_end(ap);
}

/* See InitVarargInfo in Psplit/split.c for system vararg fns. */
int execlp(const char *path, const char *arg0, ...)
{
  int i;
  va_list ap;
  char *p;

  va_start (ap, arg0);

  for (i = 0; path[i]; i++);
  for (i = 0; arg0[i]; i++);

  while ((p = va_arg (ap, char *)))
    *p;

  va_end(ap);
}

int execv(const char *path, char *const argv[])
{
    int i, j;

    for (i = 0; path[i]; i++);

    for (j = 0; argv[j]; j++)
	for (i = 0; argv[j][i]; i++);
}

int execve(const char *path, char *const argv[], char *const envp[])
{
    int i, j;

    for (i = 0; path[i]; i++);

    for (j = 0; argv[j]; j++)
	for (i = 0; argv[j][i]; i++);

    for (j = 0; envp[j]; j++)
	for (i = 0; envp[j][i]; i++);
}

int execvp(const char *path, char *const argv[])
{
    int i, j;

    for (i = 0; path[i]; i++);

    for (j = 0; argv[j]; j++)
	for (i = 0; argv[j][i]; i++);
}

void exit(int status)
{
}

double exp(double x)
{
}

double exp2(double x)
{
}

double expm1(double x)
{
}

float fexp(float x)
{
}

double fabs(double f)
{
}

float fabsf(float f)
{
}

int fchmod(int fildes, mode_t mode)
{
}

int fchown(int fildes, uid_t owner, gid_t group)
{
}

int fclose(FILE *file)
{
    FILE local;

    *file = local;
}

/* See InitVarargInfo in Psplit/split.c for system vararg fns. */
int fcntl(int fildes, int cmd, ... )
{
  va_list ap;
  char *p;

  va_start (ap, cmd);

  while ((p = va_arg (ap, char *)))
    *p;

  va_end(ap);
}

FILE *fdopen(int fildes, const char *type)
{
    int i;

    for (i = 0; type[i]; i++);
}

#ifndef _HPUX_SOURCE
/* Not present in HPUX: conflicting defines */
int feof(FILE *file)
{
    FILE local;

    *file = local;
}

int feof_unlocked(FILE *file)
{
  FILE local;

  *file = local;
}
#endif

#if !defined ferror || defined IA64LIN_SOURCE
int ferror(FILE *file)
{
    FILE local;

    *file = local;
}
#endif

int fflush(FILE *file)
{
    FILE local;

    *file = local;
}

#ifndef fgetc
int fgetc(FILE *file)
{
    FILE local;

    *file = local;
}
#endif

char *fgets(char *str, int n, FILE *file)
{
    FILE local;
    int i;

    *file = local;
    for (i = 0; str[i] = 0; i++);
    return str;
}

char *fgets_unlocked(char *str, int n, FILE *file)
{
    FILE local;
    int i;

    *file = local;
    for (i = 0; str[i] = 0; i++);
    return str;
}

#ifndef WIN32
/* NT: macro in ast/stdio.h */
int fileno(FILE *file)
{
    FILE local;

    *file = local;
}
#endif

#ifdef WIN32
int sffileno(FILE *file)
{
    FILE local;

    *file = local;
}
#endif

int flock(int fd, int operation)
{
}

double floor(double x)
{
}

double fmod(double x, double y)
{
}

int fnmatch(const char *pattern, const char *string, int flags)
{
  int i;

  for (i = 0; pattern[i]; i++);
  for (i = 0; string[i]; i++);
}

FILE *fopen(const char *path, const char *mode)
{
    int i;

    for (i = 0; path[i]; i++);
    for (i = 0; mode[i]; i++);
}

/* 08/06/02 REK Adding definition for fopen64 */
FILE *fopen64(const char *path, const char *mode)
{
    int i;

    for (i = 0; path[i]; i++);
    for (i = 0; mode[i]; i++);
}

pid_t fork()
{
}

/* See InitVarargInfo in Psplit/split.c for system vararg fns. */
int fprintf(FILE *file, const char *format, ...)
{
  FILE local;
  int i;
  va_list ap;
  char *p;

  va_start (ap, format);

  *file = local;
  for (i = 0; format[i]; i++);

  while ((p = va_arg (ap, char *)))
    *p;

  va_end(ap);
}

#ifndef WIN32
/* NT: macro in ast/stdio.h */
int fputc(int n, FILE *file)
{
    FILE local;

    *file = local;
}
#endif

#ifdef WIN32
int _sfflsbuf(FILE *file, int n)
{
    FILE local;

    *file = local;
}
#endif

int fputs(const char *str, FILE *file)
{
    FILE local;
    int i;

    *file = local;
    for (i = 0; str[i]; i++);
}

size_t fread(void *ptr, size_t size, size_t nitems, FILE *file)
{
    FILE local;
    int i;

    *file = local;
    ((char *) ptr)[i] = 0;
}

void free(void *ptr)
{
    int i;

    ((char *) ptr)[i] = 0;
}

#if defined(__linux) || defined(_HPUX_SOURCE)
void freeaddrinfo(struct addrinfo *res)
{
}
#endif

FILE *freopen64(const char *path, const char *mode, FILE *file)
{
    FILE local;
    int i;

    *file = local;
    for (i = 0; path[i]; i++);
    for (i = 0; mode[i]; i++);
}

FILE *freopen(const char *path, const char *mode, FILE *file)
{
    FILE local;
    int i;

    *file = local;
    for (i = 0; path[i]; i++);
    for (i = 0; mode[i]; i++);
}

double frexp(double x, int *exp)
{
  *exp = 0;
}

/* See InitVarargInfo in Psplit/split.c for system vararg fns. */
int fscanf(FILE *file, const char *format, ...)
{
  FILE local;
  int i;
  va_list ap;
  char *p;

  va_start (ap, format);

  *file = local;
  for (i = 0; format[i]; i++);

  while ((p = va_arg (ap, char *)))
    *p = 0;

  va_end(ap);
}

#ifdef _LARGEFILE64_SOURCE
int fseeko64(FILE *file, off64_t offset, int mode)
{
    FILE local;

    *file = local;
}
#endif

int fseek(FILE *file, long offset, int mode)
{
    FILE local;

    *file = local;
}

#ifdef _LARGEFILE64_SOURCE
int fstat64(int fildes, struct stat64 *buf)
{
    struct stat64 local;

    *buf = local;
}
#endif

int fstat(int fildes, struct stat *buf)
{
    struct stat local;

    *buf = local;
}

int fxstat(int statver, int fildes, struct stat *buf)
{
    struct stat local;

    *buf = local;
}


int ftime(struct timeb *tp)
{
  tp->time=0;
  tp->millitm=0;
  tp->timezone=0;
  tp->dstflag=0;
}

#ifdef _LARGEFILE64_SOURCE
int ftruncate64(int fd, off64_t length)
{
}
#endif

int ftruncate(int fd, off_t length)
{
}

#ifdef __linux
int __xstat (int __ver, __const char *__filename, struct stat *__stat_buf)
{
   struct stat local;

    *__stat_buf = local;
}

int __lxstat (int __ver, __const char *__filename, struct stat *__stat_buf)
{
    struct stat local;

    *__stat_buf = local;
}

int __fxstat(int __ver, int __fildes, struct stat *__stat_buf)
{
    struct stat local;

    *__stat_buf = local;
}
#endif

#ifdef _LARGEFILE64_SOURCE
off64_t ftello64(FILE *file)
{
    FILE local;

    *file = local;
}
#endif

long ftell(FILE *file)
{
    FILE local;

    *file = local;
}

size_t fwrite(const void *ptr, size_t size, size_t nitems, FILE *file)
{
    FILE local;
    int i;

    *file = local;
    for (i = 0; ((char *) ptr)[i]; i++);
}

#ifdef __linux
const char *gai_strerror(int errcode)
{
}
#endif

double gamma(double x)
{
}

#if !defined getc || defined X86LIN_SOURCE
int getc(FILE *file)
{
    FILE local;

    *file = local;
}
#endif

#ifdef IA64LIN_SOURCE
int _IO_getc(FILE *file)
{
    FILE local;

    *file = local;
}
#endif

#if defined(__linux)
int getaddrinfo(const char *node, const char *service, 
		const struct addrinfo *hints, struct addrinfo **res)
{
  int i;
  struct addrinfo *result;

  for (i = 0; node[i]; i++);
  for (i = 0; service[i]; i++);

  if (hints)
    {
      i = hints->ai_flags;
      i = hints->ai_family;
      i = hints->ai_socktype;
      i = hints->ai_protocol;
      i = hints->ai_addrlen;
      for (i = 0; hints->ai_canonname[i]; i++);
    }
}
#endif

#ifndef _HPUX_SOURCE
/* Not present in HPUX: conflicting defines */
int getchar(void)
{
    FILE local;

    *stdin = local;
}
#endif

char *getcwd(char *buf, size_t size)
{
    int i;

    for (i = 0; buf[i] = 0; i++);
}

char *getwd(char *buf)
{
    int i;

    for (i = 0; buf[i] = 0; i++);
}

char *getenv(const char *name)
{
    int i;

    for (i = 0; name[i]; i++);
}

#if defined _HPUX_SOURCE || defined REDHAT6_1
struct hostent *gethostbyaddr(const char *addr, int len, int type)
#else
struct hostent *gethostbyaddr(const void *addr, __socklen_t len, int type)
#endif
{
  char *t=(char *)addr;
  int i;

  for (i = 0; t[i], i < len; i++);
}

struct hostent *gethostbyname(const char *name)
{
    int i;

    for (i = 0; name[i]; i++);
}

int gethostname(char *name, size_t len)
{
  int i;

  for (i = 0; name[i]; i++);
}

char *getlogin()
{
}

#if defined(__linux)
int getnameinfo(const struct sockaddr *sa, socklen_t salen, char *host, 
		socklen_t hostlen, char *serv, socklen_t servlen, 
		unsigned int flags)
{
  int i;

  for (i = 0; ((char *)sa)[i], i < salen; i++);
  for (i = 0; host[i], i < hostlen; i++);
  for (i = 0; serv[i], i < servlen; i++);
}
#endif

#ifdef WIN32
int getopt(int argc, char *const argv[], const char *optstring)
{
    int i, j;

    for (i = 0; argv[i]; i++)
	for (j=0; argv[i][j]; j++) 

    for (i = 0; optstring[i]; i++);
}
#else
int getopt(int argc, char *const argv[], const char *optstring)
{
    int i, j;

    for (i = 0; argv[i]; i++)
	for (j=0; argv[i][j]; j++) 

    for (i = 0; optstring[i]; i++);

    optarg = argv[0];
    optind = 0;
    opterr = 0;
    optopt = 0;
}
#endif

/* 07/02/02 REK Adding definition for getopt_long. */
#ifdef __linux
int getopt_long(int argc, char *const argv[], const char *optstring,
		const struct option *longopts, int *longindex)
{
    int i, j;
    struct option *o=longopts;

    for (i = 0; argv[i]; i++)
	for (j = 0; argv[i][j]; j++);

    for (i = 0; optstring[i]; i++);

    for (i = 0; o; i++)
    {
	o=(struct option *)((long)o + sizeof(struct option));
	for (j = 0; o->name[j] ; j++);
    }

    optarg = argv[0];
    optind = 0;
    opterr = 0;
    optopt = 0;
}
#endif

int getpagesize()
{
}

char *getpass(const char *prompt)
{
  int i;
  char *ret;

  for (i = 0; prompt[i]; i++);

  return ret;
}

#ifdef _HPUX_SOURCE
int getpeername(int s, struct sockaddr *name, size_t *namelen)
#else
int getpeername(int s, struct sockaddr *name, socklen_t *namelen)
#endif
{
  int t;

  memset(name, '\0', *namelen);

  t=*namelen;
  *namelen=t;
}

pid_t getpgrp()
{
}

pid_t getpid()
{
}

pid_t getppid()
{
}

struct protoent *getprotobyname(const char *name)
{
  int i;
  struct protoent *result;

  for (i = 0; name[i]; i++);
  return result;
}

struct passwd *getpwnam(const char *name)
{
  int i;

  for (i = 0; name[i]; i++);
}

struct passwd *getpwuid(uid_t uid)
{
}

/* getrlimit was commented-out for Solaris...why? */
/* 11/08/02 REK Changing the prototype to match the definition on the various
 *              platforms. */
#if defined _HPUX_SOURCE
int getrlimit(int i, struct rlimit *limit)
#elif defined HAVE___RLIMIT_RESOURCE_T
int getrlimit(__rlimit_resource_t i, struct rlimit *limit)
#elif defined HAVE_ENUM___RLIMIT_RESOURCE
int getrlimit(enum __rlimit_resource i, struct rlimit *limit)
#else
int getrlimit(__rlimit_resource_t i, struct rlimit *limit)
#endif
{
    struct rlimit local;

    *limit = local;
}

#if defined _SOLARIS_SOURCE || defined _HPUX_SOURCE 
char *gets(char *s)
{
    int i;
    FILE local;

    __iob[0] = local;
    for (i = 0; s[i] = 0; i++);
    return s;
}
#elif defined WIN32
char *gets(char *s)
{
    int i;
    FILE local;

    _iob[0] = local;
    for (i = 0; s[i] = 0; i++);
    return s;
}
#else
char *gets(char *s)
{
    int i;
    FILE local;

    *stdin = local;
    for (i = 0; s[i] = 0; i++);
    return s;
}
#endif

struct servent *getservbyname(const char *name, const char *proto)
{
  int i;

  for (i = 0; name[i]; i++);
  for (i = 0; proto[i]; i++);
}

struct servent *getservbyport(int port, const char *proto)
{
  int i;

  for (i = 0; proto[i]; i++);
}

struct servent *getservent(void)
{
}

#ifdef _HPUX_SOURCE
int getsockname(int s, void *addr, int *addrlen)
#else
int getsockname(int s, struct sockaddr *addr, socklen_t *addrlen)
#endif
{
  char *t=(char *)addr;
  int i;

  for (i = 0; i < *addrlen; i++)
    ((char *)addr)[i]='\0';

  *addrlen=i;
}

#ifdef _HPUX_SOURCE
int getsockopt(int s, int level, int optname, void *optval, int *optlen)
#else
int getsockopt(int s, int level, int optname, void *optval, socklen_t *optlen)
#endif
{
  char *t=(char *)optval;
  int i;

  for (i = 0; i < *optlen; i++)
    t[i]='\0';

  *optlen=i;
}

#ifdef _HPUX_SOURCE
struct spwd *getspnam(char *name)
#else
struct spwd *getspnam(const char *name)
#endif
{
  int i;

  for (i = 0; name[i]; i++);
}

uid_t getuid(void)
{
}

uid_t geteuid(void)
{
}

gid_t getgid(void)
{
}

struct group *getgrent()
{
  struct group *result;

  return result;
}

struct group *getgrgid(gid_t gid)
{
  struct group *result;

  return result;
}

struct group *getgrnam(const char *name)
{
  struct group *result;
  int i;

  for (i = 0; name[i]; i++);

  return result;
}

#ifdef __linux
int getgrouplist(const char *name, __gid_t basegid, __gid_t *groups,
		 int *ngroups)
{
  int i;

  for (i = 0; name[i]; i++);

  for (i = 0; i < ngroups; i++)
    groups[i];

  return *ngroups;
}
#endif

int getgroups(int size, gid_t *list)
{
  int i;
  gid_t t;

  for (i = 0; i < size; i++)
    t=list[i];
}

gid_t getegid(void)
{
}

int glob(const char *pattern, int flags, int (*errfunc)(const char *, int),
	 glob_t *pglob)
{
  int i;
  glob_t ret;

  for (i = 0; pattern[i]; i++);

  *pglob=ret;
}

void globfree(glob_t *pglob)
{
  free(pglob);
}

struct tm *gmtime(const time_t *timer)
{
    time_t local;

    local = *timer;
}

struct tm *gmtime_r(const time_t *timep, struct tm *result)
{
  time_t t;

  t=*timep;

  result->tm_sec=0;
  result->tm_min=0;
  result->tm_hour=0;
  result->tm_mday=0;
  result->tm_mon=0;
  result->tm_year=0;
  result->tm_wday=0;
  result->tm_yday=0;
  result->tm_isdst=0;
}

#ifdef __linux
ssize_t getline(char **lineptr, size_t *n, FILE *stream)
{
  int i;
  FILE t=*stream;

  if (*lineptr)
    {
      for (i = 0; i < *n; i++)
	*lineptr[i]='\0';
    }
  else
    {
      *lineptr = NULL;
      *n = 0;
    }

  return *n;
}
#endif

#ifdef __linux
char *gettext (const char *msgid)
{
  int i;

  for (i = 0; msgid[i]; i++);

  return msgid;
}
#endif

#ifdef _HPUX_SOURCE
int gettimeofday(struct timeval *tp, void *tzp)
{
    int tz_minuteswest = ((struct timezone *)tzp)->tz_minuteswest;
    int tz_dsttime = ((struct timezone *)tzp)->tz_dsttime;
    tp->tv_sec = 0;
    tp->tv_usec = 0;
}
#else
int gettimeofday(struct timeval *tp, struct timezone *tzp)
{
    int tz_minuteswest = tzp->tz_minuteswest;
    int tz_dsttime = tzp->tz_dsttime;
    tp->tv_sec = 0;
    tp->tv_usec = 0;
}
#endif

#ifdef _HPUX_SOURCE
int getrusage (int who, struct rusage *r_usage)
{
    struct rusage local;

    *r_usage = local;
    errno = 0;
}
#elif !defined x86LIN_SOURCE 
int getrusage(int who, struct rusage *usage)
{
  struct rusage local;
  *usage = local;
}
#endif

#ifdef HAVE_LIBSSL
void HMAC_cleanup(HMAC_CTX *ctx)
{
  free(ctx);
}

void HMAC_Final(HMAC_CTX *ctx, unsigned char *md, unsigned int *len)
{
  int i;

  for (i = 0; ((char *)ctx)[i], i < sizeof(HMAC_CTX); i++);

  *md='\0';
  *len=0;
}

void HMAC_Init(HMAC_CTX *ctx, const void *key, int len, const EVP_MD *md)
{
  ctx->key_length=len;
  memcpy(ctx->key, key, len);
  ctx->md=md;
}

void HMAC_Update(HMAC_CTX *ctx, const unsigned char *data, int len)
{
  int i;
  char c;

  for (i = 0; i < sizeof (HMAC_CTX); i++)
    c = ((char *)ctx)[i];

  for (i = 0; i < len; i++) c = data[i];
}
#endif

#if defined __linux
uint32_t htonl(uint32_t hostlong)
{
}

unsigned short int htons(unsigned short int hostshort)
{
}
#elif !defined _HPUX_SOURCE
unsigned long int htonl(unsigned long int hostlong) 
{
}

unsigned short int htons(unsigned short int hostshort)
{
}
#endif

double hypot(double x, double y)
{
}

#ifdef __linux
char *index (__const char *s, int c)
{
    return (char *) s;
}
#else
char *index(char *s, int c)
{
    return s;
}
#endif

unsigned long inet_addr(const char *cp)
{
    int i;

    for (i = 0; cp[i]; i++);
}

int inet_aton(const char *cp, struct in_addr *inp)
{
  int i;

  for (i = 0; cp[i]; i++);

  inp->s_addr=0;
}

unsigned long int inet_lnaof(struct in_addr in)
{
}

struct in_addr inet_makeaddr(int net, int host)
{
}

unsigned long int inet_netof(struct in_addr in)
{
}

unsigned long int inet_network(const char *cp)
{
  int i;

  for (i = 0; cp[i]; i++);
}

char *inet_ntoa(struct in_addr in)
{
}

#ifdef HAVE_LIBZ
int inflate(z_streamp strm, int flush)
{
  int i;

  for (i = 0; ((char *)strm)[i], i < sizeof(z_stream); i++);
}

int inflateEnd(z_streamp strm)
{
  int i;

  for (i = 0; ((char *)strm)[i], i < sizeof(z_stream); i++);
}

int inflateInit_(z_streamp strm, const char *version, int stream_size)
{
  int i;

  for (i = 0; ((char *)strm)[i], i < sizeof(z_stream); i++);
}
#endif

#ifdef HAVE_LIBCURSES
WINDOW *initscr()
{
}
#endif

int innetgr(const char *netgroup, const char *machine, 
	    const char *user, const char *domain)
{
  int i;

  for (i = 0; netgroup[i]; i++);
  for (i = 0; machine[i]; i++);
  for (i = 0; user[i]; i++);
  for (i = 0; domain[i]; i++);
}

/* See InitVarargInfo in Psplit/split.c for system vararg fns. */
#ifdef __linux
int ioctl(int fildes, unsigned long request, ...)
#else
int ioctl(int fildes, int request, ...)
#endif
{
  va_list ap;
  char *p;

  va_start (ap, request);

  while ((p = va_arg (ap, char *)))
    *p;

  va_end(ap);
}

int initgroups(const char *user, gid_t group)
{
  int i;

  for (i = 0; user[i]; i++);
}

#ifndef WIN32
/* NT: macro in ctype.h */
#if !defined isalnum || defined _HPUX_SOURCE 
int isalnum(int c)
{
}
#endif
#endif

#if !defined isalpha || defined _HPUX_SOURCE 
int isalpha(int c)
{
}
#endif

#if !defined isascii
int isascii(int c)
{
}
#endif

#if !defined isatty || defined _HPUX_SOURCE
int isatty(int c)
{
}
#endif

int iswalnum(wint_t wc)
{
}

int iswctype(wint_t wc, wctype_t desc)
{
}

int iswlower(wint_t wc)
{
}

#ifndef WIN32
/* NT: macro in ctype.h */
#if !defined iscntrl || defined _HPUX_SOURCE
int iscntrl(int c)
{
}
#endif
#endif

#if !defined isdigit || defined _HPUX_SOURCE
int isdigit(int c)
{
}
#endif

#if !defined isgraph || defined _HPUX_SOURCE
int isgraph(int c)
{
}
#endif

#if !defined islower || defined _HPUX_SOURCE
int islower(int c)
{
}
#endif

#if !defined isprint || defined _HPUX_SOURCE
int isprint(int c)
{
}
#endif

#if !defined ispunct || defined _HPUX_SOURCE
int ispunct(int c)
{
}
#endif

#if !defined isspace || defined _HPUX_SOURCE
int isspace(int c)
{
}
#endif

#if !defined isupper || defined _HPUX_SOURCE
int isupper(int c)
{
}
#endif

#if !defined isxdigit || defined _HPUX_SOURCE
int isxdigit(int c)
{
}
#endif

int kill(pid_t pid, int signal)
{
}

/* Not present for NT */
long labs(long i)
{
}

int lchown(const char *path, uid_t owner, gid_t group)
{
  int i;

  for (i = 0; path[i]; i++);
}

double ldexp(double x, int exp)
{
}

double lgamma(double x)
{
}

int link(const char *oldpath, const char *newpath)
{
  int i;

  for (i = 0; oldpath[i]; i++);
  for (i = 0; newpath[i]; i++);
}

#ifdef REDHAT6_1
int listen(int s, unsigned int backlog)
#else
int listen(int s, int backlog)
#endif
{
}

struct tm *localtime(const time_t *timer)
{
    time_t local;

    local = *timer;
}

double log(double x)
{
}

double log10(double x)
{
}

double log2(double x)
{
}

double log1p(double x)
{
}

double logb(double x)
{
}

#ifdef __linux
void login(const struct utmp *e)
{
  struct utmp t;

  if (e)
    memcpy (&t, e, sizeof(struct utmp));
}

int logout(const char *u)
{
  int i;

  for (i = 0; u[i]; i++);
}

void logwtmp(const char *line, const char *name, const char *host)
{
  int i;

  for (i = 0; line[i]; i++);
  for (i = 0; name[i]; i++);
  for (i = 0; host[i]; i++);
}
#endif

#ifdef __linux
void longjmp(jmp_buf env, int val)
{
    int i;
    jmp_buf local;

    env = local;
}
#else
void longjmp(jmp_buf env, int val)
{
    int i;

    for (i = 0; env[i] = 0; i++);
}
#endif

off_t lseek(int fildes, off_t offset, int whence)
{
}

/* 07/24/02 REK Adding definition for lstat */
int lstat(const char *path, struct stat *buf)
{
    int i;
    struct stat local;

    *buf = local;
    for (i = 0; path[i]; i++);
}

/* 08/06/02 REK Adding definition for lstat64 */
#ifdef _LARGEFILE64_SOURCE
int lstat64(const char *path, struct stat64 *buf)
{
    int i;
    struct stat64 local;

    *buf = local;
    for (i = 0; path[i]; i++);
}
#endif

struct mallinfo mallinfo()
{
  struct mallinfo ret;

  return ret;
}

void *malloc(size_t size)
{
  return 0;
}

#ifdef __linux
size_t mbrlen(const char *s, size_t n, mbstate_t *ps)
{
  int i;

  for (i = 0; s[i], i < n; i++);

  for (i = 0; i < sizeof(mbstate_t); i++)
    ((char *)ps)[i]='\0';
}

size_t mbrtowc(wchar_t *pwc, const char *s, size_t n, mbstate_t *ps)
{
  int i;

  *pwc = 0;

  for (i = 0; s[i]; i++);

  for (i = 0; i < sizeof(mbstate_t); i++)
    ((char *)ps)[i]='\0';
}
#endif

#ifdef HAVE_LIBSSL
void MD5_Final(unsigned char *md, MD5_CTX *c)
{
  int i;

  for (i = 0; md[i]; i++);
  for (i = 0; ((char *)c)[i], i < sizeof(MD5_CTX); i++);
}

void MD5_Init(MD5_CTX *c)
{
  int i;

  for (i = 0; ((char *)c)[i], i < sizeof(MD5_CTX); i++);
}

void MD5_Update(MD5_CTX *c, const void *data, unsigned long len)
{
  int i;
  char *t=(char *)data;

  for (i = 0; ((char *)c)[i], i < sizeof(MD5_CTX); i++);
  for (i = 0; t[i], i < len; i++);
}
#endif

#ifdef __linux
void *memchr(const void *s, int c, size_t n)
{
    int i;

    for (i = 0; ((int *) s)[i]; i++);
    return (void *) s;
}
#elif defined _HPUX_SOURCE
void *memchr(const void *s, int c, size_t n)
{
    int i;

    for (i = 0; i < n; i++)
	if (((char *)s)[i] == (unsigned char)c)
	    return (void *) &(((char *)s)[i]);
    return NULL;
}
#endif

int memcmp(const void *s1, const void *s2, size_t n)
{
    int i;

    for (i = 0; i < n; i++)
	if (((char *)s1)[i] != ((char *)s2)[i])
	    return 1;
    return 0;
}

void *memcpy(void *s1, const void *s2, size_t n)
{
    int i;

    for (i = 0; i < n; i++)
	((char *) s1)[i] = ((char *) s2)[i];
    return s1;
}

void *memmove(void *s1, const void *s2, size_t n)
{
    int i;

    for (i = 0; i < n; i++)
	((char *) s1)[i] = ((char *) s2)[i];
    return s1;
}

void *mempcpy(void *dest, void *src, size_t n)
{
  int i;
  char *d=(char *)dest;
  char *s=(char *)src;

  for (i = 0; i < n; i++)
    d[i]=s[i];

  return (void *)&(d[i]);
}

void *memset(void *s1, int c, size_t n)
{
    int i;

    for (i = 0; i < n; i++)
	((unsigned char *) s1)[i] = (unsigned char) c;
}

int mkdir(const char *pathname, mode_t mode)
{
  int i;

  for (i = 0; pathname[i]; i++);
}

#ifdef __linux
char *mkdtemp(char *template)
{
  int i;

  for (i = 0; template[i]; i++);
}
#endif

int mknod(const char *pathname, mode_t mode, dev_t dev)
{
  int i;

  for (i = 0; pathname[i]; i++);
}

int mkstemp(char *template)
{
  int i;

  for (i = 0; template[i] = 0; i++);
}

char *mktemp(char *template)
{
    int i;

    for (i = 0; template[i] = 0; i++);
    return template;
}

time_t mktime(struct tm *timeptr)
{
  timeptr->tm_sec = 0;
  timeptr->tm_min = 0;
  timeptr->tm_hour = 0;
  timeptr->tm_mday = 0;
  timeptr->tm_mon = 0;
  timeptr->tm_year = 0;
  timeptr->tm_wday = 0;
  timeptr->tm_yday = 0;
  timeptr->tm_isdst = 0;

  tzname[0]='\0';
  tzname[1]='\1';
}

void *mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset)
{
}

int munmap(void *start, size_t length)
{
}

double modf(double x, double *iptr)
{
    *iptr = 0;
}

int nanosleep(const struct timespec *req, struct timespec *rem)
{
  rem->tv_sec = req->tv_sec;
  rem->tv_nsec = req->tv_nsec;
}

#ifdef __linux
char *ngettext(const char *msgid1, const char *msgid2, unsigned long int n)
{
  int i;

  for (i = 0; msgid1[i]; i++);
  for (i = 0; msgid2[i]; i++);

  if (n == 1)
    return msgid1;
  else
    return msgid2;
}
#endif

int nl()
{
}

#ifndef WIN32
/* NT: macro in ast/term.h */
int noecho()
{
}
#endif

int nonl()
{
}

int noraw()
{
}

#ifdef WIN32
int _setnl(int b)
{
}

int _setraw(int b)
{
}
#endif

#if defined __linux
uint32_t ntohl(uint32_t netlong)
{
}

unsigned short ntohs(unsigned short netshort)
{
}
#elif !defined _HPUX_SOURCE
unsigned long ntohl(unsigned long netlong)
{
}

unsigned short ntohs(unsigned short netshort)
{
}
#endif

#if defined __linux && !defined IA64LIN_SOURCE
void _obstack_newchunk (struct obstack *h, int length)
{
    struct obstack local;

    *h = local;
}

int _obstack_begin (struct obstack *h, int size, int alignment,
                    void * (*chunkfun) (), void (*freefun) ())
{
    struct obstack local;

    h->chunkfun = (struct _obstack_chunk * (*)()) chunkfun;
    h->freefun = freefun;
    *h = local;
}

#ifdef obstack_free
#undef obstack_free
#define obstack_free obstack_free
#endif
void obstack_free (struct obstack *obstack, void *block)
{
    struct obstack local;

    *obstack = local;
    *((char *) block) = 0;
}
#endif

/* See InitVarargInfo in Psplit/split.c for system vararg fns. */
int open(const char *path, int oflag, ...)
{
    int i;
    va_list ap;
    char *p;

    va_start (ap, oflag);
    
    for (i = 0; path[i]; i++);

    while ((p = va_arg (ap, char *)))
      *p;

    va_end(ap);
}

void openlog(const char *ident, int option, int facility)
{
  int i;

  for (i = 0; ident[i]; i++);
}

#ifdef __linux
int openpty(int *amaster, int *aslave, char *name, struct termios *termp,
	    struct winsize *winp)
{
  struct termios tt;
  struct winsize tw;

  *amaster=0;
  *aslave=0;
  if (name) name[0]='\0';
  if (termp) tt=*termp;
  if (winp) tw=*winp;
}
#endif

#ifdef HAVE_LIBSSL
void OpenSSL_add_all_algorithms(void)
{
}

const char *OBJ_nid2sn(int n)
{
  char *result;

  return (result);
}
#endif

/* 08/06/02 REK Defining open64. */
int open64(const char *path, int oflag, ...)
{
  int i;
  va_list ap;
  char *p;

  va_start (ap, oflag);
    
  for (i = 0; path[i]; i++);

  while ((p = va_arg (ap, char *)))
    *p;

  va_end(ap);
}

DIR *opendir(const char *path)
{
    int i;

    for (i = 0; path[i]; i++);
}

long pathconf(const char *path, int name)
{
    int i;

    for (i = 0; path[i]; i++);
}

int pclose(FILE *file)
{
    FILE local;

    *file = local;
}

#ifdef HAVE_LIBSSL
EVP_PKEY *PEM_read_PrivateKey(FILE *fp, EVP_PKEY **x, pem_password_cb *cb,
			      void *u)
{
  FILE t=*fp;
  EVP_PKEY local_pkey;
  char buf[100];
 
  cb(buf, 100, 0, u);

  if (x)
    {
      if (!*x)
	**x=local_pkey;
      return *x;
    }
  else
    return NULL;
}
#endif

#if defined _SOLARIS_SOURCE || defined _HPUX_SOURCE
void perror(const char *message)
{
    int i;
    FILE local;

    for (i = 0; message[i]; i++);
    __iob[0] = local;
}
#elif defined WIN32
void perror(const char *message)
{
    int i;
    FILE local;

    for (i = 0; message[i]; i++);
    _iob[0] = local;
}
#else
void perror(const char *message)
{
    int i;
    FILE local;

    for (i = 0; message[i]; i++);
    *stderr = local;
}
#endif

int pipe(int *fildes)
{
    fildes[0] = 0;
    fildes[1] = 0;
}

FILE *popen(const char *command, const char *type)
{
    int i;

    for (i = 0; command[i]; i++);
    for (i = 0; type[i]; i++);
}

#ifdef __linux
#ifdef HAVE_POPT_H
const char *poptBadOption(poptContext con, int flags)
{
}

const char **poptGetArgs(poptContext con)
{
}

poptContext poptGetContext(const char *name, int argc, const char **argv,
			   const struct poptOption *options, int flags)
{
  int i, j;
  poptContext ret;
  
  for (i = 0; name[i]; i++);
  for (i = 0; i < argc; i++)
    for (j = 0; argv[i][j]; j++);

  if (options)
    {
      if (options->longName)
	for (i = 0; options->longName[i]; i++);
      if (options->descrip)
	for (i = 0; options->descrip[i]; i++);
      if (options->argDescrip)
	for (i = 0; options->argDescrip[i]; i++);
    }

  return ret;
}

int poptGetNextOpt(poptContext con)
{
}

const char *poptGetOptArg(poptContext con)
{
}

const char * const poptStrerror(const int error)
{
}
#endif
#endif

double pow(double x, double y)
{
}

float powf(float x, float y)
{
}

/* See InitVarargInfo in Psplit/split.c for system vararg fns. */
#ifdef __linux
int printf(const char *format, ...)
{
  FILE local;
  int i;
  va_list ap;
  char *p;

  va_start (ap, format);

  *stdout = local;
  for (i = 0; format[i]; i++);

  while ((p = va_arg (ap, char *)))
    *p;

  va_end(ap);
}
#elif defined WIN32
int printf(const char *format, ...)
{
  FILE local;
  int i;
  va_list ap;
  char *p;

  va_start (ap, format);

  _iob[2] = local;
  for (i = 0; format[i]; i++);

  while ((p = va_arg (ap, char *)))
    *p;

  va_end(ap);
}
#else
int printf(const char *format, ...)
{
  FILE local;
  int i;
  va_list ap;
  char *p;

  va_start (ap, format);

  __iob[2] = local;
  for (i = 0; format[i]; i++);

  while ((p = va_arg (ap, char *)))
    *p;

  va_end(ap);
}
#endif

#ifdef __linux
/* See InitVarargInfo in Psplit/split.c for system vararg fns. */
int printw(const char *format, ...)
{
  int i;
  va_list ap;
  char *p;

  va_start (ap, format);

  for (i = 0; format[i]; i++);

  while ((p = va_arg (ap, char *)))
    *p;

  va_end(ap);
}
#else
/* See InitVarargInfo in Psplit/split.c for system vararg fns. */
int printw(const char *format, ...)
{
  int i;
  va_list ap;
  char *p;

  va_start (ap, format);

  for (i = 0; format[i]; i++);

  while ((p = va_arg (ap, char *)))
    *p;

  va_end(ap);
}
#endif

/* FIX */
#if !defined _HPUX_SOURCE || !defined putc
int putc(int c, FILE *file)
{
    FILE local;

    *file = local;
}
#endif

#ifndef _HPUX_SOURCE
/* Not in HPUX: conflicting defines */
int putchar(int c)
{
    FILE local;

    *stdout = local;
}
#endif

#ifdef __linux
int puts(const char *str)
{
    int i;
    FILE local;

    for (i = 0; str[i]; i++);
    *stdout = local;
}
#elif defined WIN32
int puts(const char *str)
{
    int i;
    FILE local;

    for (i = 0; str[i]; i++);
    _iob[1] = local;
}
#else
int puts(const char *str)
{
    int i;
    FILE local;

    for (i = 0; str[i]; i++);
    __iob[1] = local;
}
#endif

void qsort(void *base, size_t nel, size_t size,
	   int (*compar)(const void *, const void *))
{
    int i;

    for (i = 0; ((char *) base)[i] = 0; i++);
    compar(base, base);
}

int raise(int sig)
{
}

int rand()
{
}

#ifdef HAVE_LIBSSL
int RAND_bytes(unsigned char *buf, int num)
{
  memset(buf, '\0', num);
  return(1);
}
    
void RAND_seed(const void *buf, int num)
{
  int i;

  for (i = 0; i < num; i++)
    ((char *)buf)[i] = '\0';
}

int RAND_status()
{
}
#endif

#if !defined _HPUX_SOURCE && defined REDHAT6_1
int32_t random()
#else
long random()
#endif
{
}

#ifndef WIN32
/* NT: macro in ast/term.h */
int raw()
{
}
#endif

#ifdef HAVE_LIBSSL
void RC4(RC4_KEY *key, unsigned long len, const unsigned char *indata,
	 unsigned char *outdata)
{
  int i;

  for (i = 0; ((char *)key)[i], i < sizeof(RC4_KEY); i++);

  for (i = 0; i < len; i++)
    outdata[i]=indata[i];
}

void RC4_set_key(RC4_KEY *key, int len, const unsigned char *data)
{
  int i;

  for (i = 0; data[i], i < len; i++);

  memset(key, '\0', sizeof(RC4_KEY));
}
#endif

ssize_t read(int fildes, void *buffer, size_t nbyte)
{
    int i;

    for (i = 0; ((char *) buffer)[i] = 0; i++);
}

struct dirent *readdir(DIR *dirp)
{
    DIR local;

    local = *dirp;
}

int readlink(const char *path, char *buf, size_t bufsize)
{
  int i;

  for (i = 0; path[i]; i++);
  for (i = 0; i < bufsize; i++)
    buf[i]='\0';

  return bufsize;
}

#ifdef IA64LIN_SOURCE
void *realloc(void *ptr, size_t size)
{
    return ptr;
}
#else 
void *realloc(void *ptr, unsigned size)
{
    return ptr;
}
#endif

char *realpath(const char *path, char *resolved_path)
{
  int i;

  for (i = 0; path[i]; i++);

  strcpy(resolved_path, path);

  return resolved_path;
}

#ifdef _HPUX_SOURCE
int recvmsg(int s, struct msghdr *msg, int flags)
#else
ssize_t recvmsg(int s, struct msghdr *msg, int flags)
#endif
{
  int i;

  for (i = 0; ((char *)msg)[i], i < sizeof(struct msghdr); i++);
}

int remove(const char *path)
{
    int i;

    for (i = 0; path[i]; i++);
}

int rename(const char *source, const char *target)
{
    int i;

    for (i = 0; source[i]; i++);
    for (i = 0; target[i]; i++);
}

void rewind(FILE *file)
{
    FILE local;

    *file = local;
}

#ifdef __linux
char *rindex (__const char *s, int c)
{
    return (char *) s;
}
#else
char *rindex(char *s, int c)
{
    return s;
}
#endif

int rmdir(const char *path)
{
  int i;

  for (i = 0; path[i]; i++);
}

#ifndef _HPUX_SOURCE
int rresvport_af(int *alport, sa_family_t af)
{
  *alport=0;
}
#endif

#ifdef HAVE_LIBSSL
int RSA_blinding_on(RSA *rsa, BN_CTX *p_ctx)
{
  int i;
  char c;

  for (i = 0; i < sizeof (RSA); i++)
    c = ((char *)rsa)[i];

  for (i = 0; i < sizeof (BN_CTX); i++)
    c = ((char *)p_ctx)[i];
}

void RSA_free(RSA *r)
{
  free(r);
}

RSA *RSA_generate_key(int bits, unsigned long e_value,
		      void (*callback)(int, int, void *), void *cb_arg)
{
  RSA *result;

  return (result);
}

RSA *RSA_new(void)
{
  return((RSA *)malloc(sizeof(RSA)));
}

int RSA_private_decrypt(int flen, unsigned char *from, unsigned char *to,
			RSA *rsa, int padding)
{
  int i;

  for (i = 0; ((char *)rsa)[i], i < sizeof(RSA); i++);

  for (i = 0; i < flen; i++)
    to[i]=from[i];
}

int RSA_private_encrypt(int flen, unsigned char *from, unsigned char *to,
			RSA *rsa, int padding)
{
  int i;

  for (i = 0; ((char *)rsa)[i], i < sizeof(RSA); i++);

  for (i = 0; i < flen; i++)
    to[i]=from[i];
}

int RSA_public_decrypt(int flen, unsigned char *from, unsigned char *to,
		       RSA *rsa, int padding)
{
  int i;

  for (i = 0; ((char *)rsa)[i], i < sizeof(RSA); i++);

  for (i = 0; i < flen; i++)
    to[i]=from[i];
}

int RSA_public_encrypt(int flen, unsigned char *from, unsigned char *to,
		       RSA *rsa, int padding)
{
  int i;

  for (i = 0; ((char *)rsa)[i], i < sizeof(RSA); i++);

  for (i = 0; i < flen; i++)
    to[i]=from[i];
}

int RSA_sign(int type, unsigned char *m, unsigned int m_len,
	     unsigned char *sigret, unsigned int *siglen, RSA *rsa)
{
  int i;
  char c;

  for (i = 0; i < m_len; i++)
    c = m[i];

  sigret[0] = '\0';
  *siglen = 0;

  for (i = 0; i < sizeof (RSA); i++)
    c = ((char *)rsa)[i];
}

int RSA_size(RSA *r)
{
  int i;
  char c;

  for (i = 0; i < sizeof (RSA); i++)
    c = ((char *)r)[i];
}
#endif

#ifdef IA64LIN_SOURCE
void *sbrk(long incr)
{
}
#else
void *sbrk(int incr) 
{
}
#endif

/* See InitVarargInfo in Psplit/split.c for system vararg fns. */
int scanf(const char *format, ...)
{
  FILE local;
  int i;
  va_list ap;
  char *p;

  *stdin = local;
  va_start (ap, format);

  for (i = 0; format[i]; i++);

  while ((p = va_arg (ap, char *)))
    *p = 0;

  va_end(ap);
}

int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *errorfds,
	   struct timeval *timeout)
{
    fd_set local;
    struct timeval local2;

    local = *readfds;
    local = *writefds;
    local = *errorfds;

    local2 = *timeout;

    *readfds = local;
    *writefds = local;
    *errorfds = local;

    *timeout = local2;
}

ssize_t send(int s, const void *msg, size_t len, int flags)
{
  char *t=(char *)msg;
  int i;

  for (i = 0; t[i], t < len; i++);
}

#ifdef _HPUX_SOURCE
int sendto(int s, const void *msg, int len, int flags, const void *to,
	   int tolen)
#else
ssize_t sendto(int s, const void *msg, size_t len, int flags, 
	       const struct sockaddr *to, socklen_t tolen)
#endif
{
  char *t=(char *)msg;
  int i;

  for (i = 0; t[i], i < len; i++);

  t=(char *)to;
  for (i = 0; t[i], i < tolen; i++);
}

#ifndef REDHAT6_1
ssize_t sendmsg(int s, const struct msghdr *msg, int flags)
{
  char *t=(char *)msg->msg_name;
  struct iovec *vec=msg->msg_iov;
  int i;

  for (i = 0; t[i], i < msg->msg_namelen; i++);
  for (i = 0; vec->iov_base, i < msg->msg_iovlen; i++);

  t=(char *)msg->msg_control;
  for (i = 0; t[i], i < msg->msg_controllen; i++);
}
#endif

void setbuf(FILE *file, char *buffer)
{
    FILE local;
    int i;

    *file = local;
    for (i = 0; buffer[i] = 0; i++);
}

#ifdef __linux
int setegid(gid_t gid)
{
}

int seteuid(uid_t euid)
{
}
#endif

#ifdef IA64LIN_SOURCE
int setjmp(jmp_buf env)
{
    int i;

    for (i = 0; ; i++)
        env[i] = env[i+1];
}
#elif !defined X86LIN_SOURCE
int setjmp(jmp_buf env)
{
    int i;

    for (i = 0; env[i] = 0; i++);
}
#endif

char *setlocale(int category, const char *locale)
{
  int i;

  for (i = 0; locale[i]; i++);
}

#if defined __linux
#if defined HAVE___PRIORITY_WHICH_T
int setpriority(__priority_which_t which, id_t who, int priority)
#elif defined HAVE_ENUM___PRIORITY_WHICH
int setpriority(enum __priority_which which, int who, int priority)
#else
int setpriority(__priority_which_t which, id_t who, int priority)
#endif
#else
int setpriority(int which, int who, int priority)
#endif
{
}

int setreuid(uid_t ruid, uid_t euid)
{
}

/* Commented out for Solaris...why? */
#if defined HAVE___RLIMIT_RESOURCE_T && ! defined _HPUX_SOURCE
int setrlimit(__rlimit_resource_t resource, const struct rlimit *rlp)
#elif defined HAVE_ENUM___RLIMIT_RESOURCE && ! defined _HPUX_SOURCE
int setrlimit(enum __rlimit_resource resource, struct rlimit *rlp)
#else
int setrlimit(int resource, const struct rlimit *rlp)
#endif
{
    struct rlimit local;

    local = *rlp;
}

void setservent(int stayopen)
{
}

pid_t setsid()
{
}

#ifdef _HPUX_SOURCE
int setsockopt(int s, int level, int optname, const void *optval, int optlen)
#else
int setsockopt(int s, int level, int optname, const void *optval,
	       socklen_t optlen)
#endif
{
  char *t=(char *)optval;
  int i;

  for (i = 0; t[i], i < optlen; i++);
}

int setgid(gid_t uid)
{
}

void setgrent()
{
}

#ifdef _HPUX_SOURCE
int setgroups(int size, gid_t *list)
#else
int setgroups(size_t size, const gid_t *list)
#endif
{
  int i;
  gid_t t;

  for (i = 0; i < size; i++)
    t=list[i];
}

int setuid(uid_t uid)
{
}

int setresuid(uid_t ruid, uid_t euid, uid_t suid)
{
}

int setresgid(gid_t rgid, gid_t egid, gid_t sgid)
{
}

int setvbuf(FILE *stream, char *buf, int mode, size_t size)
{
  int i;

  for (i = 0; buf[i], i < size; i++);
}

void *shmat(int shmid, const void *shmaddr, int shmflg)
{
    return (void *)shmaddr;
}

int shmctl(int shmid, int cmd, struct shmid_ds *buf)
{
    struct shmid_ds local;
    struct shmid_ds local2;

    local = *buf;

    *buf = local2;
}

int shmdt(const void *shmaddr)
{
}

#if defined REDHAT6_1 && ! defined _HPUX_SOURCE
int shmget(key_t key, int size, int shmflg)
#else
int shmget(key_t key, size_t size, int shmflg)
#endif
{
}

int shutdown(int s, int how)
{
}

int sigaction(int sig, const struct sigaction *act, struct sigaction *oact)
{
    struct sigaction local;
    struct sigaction local2;

    local = *act;
    *oact = local2;
}

int sigaddset(sigset_t *set, int signo)
{
    sigset_t local;

    *set = local;
}

int sigemptyset(sigset_t *set)
{
    sigset_t local;

    *set = local;
}

void (*signal(int sig, void (*func)(int)))(int)
{
    void (*fn)(int);
    fn = func; 
}

#ifdef X86LIN_SOURCE
void (*__sysv_signal(int sig, void (*func)(int)))(int)
{
    void (*fn)(int);
    fn = func; 
}
#endif

int sigprocmask(int how, const sigset_t *set, sigset_t *oset)
{
    sigset_t local;
    sigset_t local2;

    local = *set;
    *oset = local2;
}

int __sigsetjmp(jmp_buf env, int i)
{
    for (i = 0; ; i++)
        env[i] = env[i+1];
}

double sin(double x)
{
}

double sinh(double x)
{
}

unsigned int sleep(unsigned int seconds)
{
}

#ifdef __linux
int snprintf(char *str, size_t size, const char *format, ...)
{
  int i;
  va_list ap;
  char *p;

  va_start (ap, format);

  for (i = 0; str[i] = format[i]; i++);
  while (i < size)
    {
      str[i] = '\0';
      i++;
    }
  while ((p = va_arg (ap, char *)))
    *p;

  va_end(ap);
}
#endif

int socket(int af, int type, int protocol)
{
}

int socketpair(int af, int type, int protocol, int sv[2])
{
    sv[0] = 0;
    sv[1] = 0;
}

/* See InitVarargInfo in Psplit/split.c for system vararg fns. */
int sprintf(char *s, const char *format, ...)
{
  int i;
  va_list ap;
  char *p;

  va_start (ap, format);

  for (i = 0; s[i] = format[i]; i++);

  while ((p = va_arg (ap, char *)))
    *p;

  va_end(ap);
}

double sqrt(double x)
{
}

void srand(unsigned int seed)
{
}

void srandom(unsigned int seed)
{
}

/* See InitVarargInfo in Psplit/split.c for system vararg fns. */
int sscanf(const char *s, const char *format, ...)
{
  int i;
  va_list ap;
  char *p;

  va_start (ap, format);

  for (i = 0; s[i] && format[i]; i++);

  while ((p = va_arg (ap, char *)))
    *p = 0;

  va_end(ap);
}

#ifdef HAVE_LIBSSL
unsigned long SSLeay(void)
{
}
#endif

int stat(const char *path, struct stat *buf)
{
    int i;
    struct stat local;

    *buf = local;
    for (i = 0; path[i]; i++);
}

/* 08/06/02 REK Adding definition for stat64 */
#ifdef _LARGEFILE64_SOURCE
int stat64(const char *path, struct stat64 *buf)
{
    int i;
    struct stat64 local;

    *buf = local;
    for (i = 0; path[i]; i++);
}
#endif

char *stpcpy(char *dest, const char *src)
{
  int i;

  for (i = 0; src[i]; i++)
    dest[i]=src[i];

  dest[i]='\0';
  return &(dest[i]);
}

int strcasecmp(const char *s1, const char *s2)
{
    int i;

    for (i = 0; s1[i] && s2[i]; i++);
}

char *strcat(char *s1, const char *s2)
{
    int i;

    for (i = 0; s1[i] = s2[i]; i++);
    return s1;
}

char *strchr(const char *s, int c)
{
    int i;

    for (i = 0; s[i]; i++);
    return (char *) s;
}

int strcmp(const char *s1, const char *s2)
{
    int i;

    for (i = 0; s1[i] && s2[i]; i++);
}

char *strcpy(char *s1, const char *s2)
{
    int i;

    for (i = 0; s1[i] = s2[i]; i++);
    return s1;
}

int strcoll(const char *s1, const char *s2)
{
  int i;

  for (i = 0; s1[i] && s2[i]; i++);
}

size_t strcspn(const char *s1, const char *s2)
{
    int i;

    for (i = 0; s1[i] && s2[i]; i++);
}

char *strdup(const char *s)
{
    int i;

    for (i = 0; s[i]; i++);
}

char *strerror(int n)
{
}

#ifdef _HPUX_SOURCE
int strerror_r(int errnum, char *strerrbuf, int buflen)
#elif defined __USE_XOPEN2K  // get it to work with glibc-2.4  -KF 3/2006
int strerror_r(int errnum, char *strerrbuf, size_t buflen)
#else
char *strerror_r(int errnum, char *strerrbuf, size_t buflen)
#endif
{
  int i;

  for (i = 0; i < buflen; i++)
    strerrbuf[i]='\0';
}

size_t strftime(char *s, size_t max, const char *format, const struct tm *tm)
{
  int i;

  for (i = 0; s[i]; i++);
  for (i = 0; format[i]; i++);
}

size_t strlen(const char *s)
{
    int i;

    for (i = 0; s[i]; i++);
}

int strncasecmp(const char *s1, const char *s2, size_t n)
{
  int i;

  for (i = 0; s1[i] == s2[i], i < n; i++);
}

char *strncat(char *s1, const char *s2, size_t n)
{
    int i;

    for (i = 0; s1[i] = s2[i]; i++);
    return (char *) s1;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
    int i;

    for (i = 0; s1[i] && s2[i]; i++);
}

char *strncpy(char *s1, const char *s2, size_t n)
{
    int i;

    for (i = 0; s1[i] = s2[i]; i++);
    return (char *) s1;
}

char *strpbrk(const char *s1, const char *s2)
{
    int i;

    for (i = 0; s1[i] && s2[i]; i++);
    return (char *) s1;
}

#ifdef _SOLARIS_SOURCE
#undef strrchr
#endif

char *strrchr(const char *s, int c)
{
    int i;

    for (i = 0; s[i]; i++);
    return (char *) s;
}

char *strrstr(const char *s1, const char *s2)
{
    int i;

    for (i = 0; s1[i] && s2[i]; i++);
    return (char *) s1;
}

#ifdef __linux
char *strsep(char **stringp, const char *delim)
{
  int i;

  if (!*stringp)
    return NULL;

  for (i = 0; delim[i]; i++);
  for (i = 0; *stringp[i]; i++);
}
#endif

size_t strspn(const char *s1, const char *s2)
{
    int i;

    for (i = 0; s1[i] && s2[i]; i++);
}

char *strstr(const char *s1, const char *s2)
{
    int i;

    for (i = 0; s1[i] && s2[i]; i++);
    return (char *) s1;
}

double strtod(const char *str, char **ptr)
{
    int i;

    for (i = 0; str[i]; i++);
    *ptr = (char *) str;
}

char *strtok(char *s1, const char *s2)
{
    int i;

    for (i = 0; s1[i] = s2[i]; i++);
    return s1;
}

char *strtok_r(char *s1, const char *s2, char **last)
{
    int i;

    for (i = 0; s1[i] = s2[i]; i++);
    *last = s1;
    return s1;
}

long strtol(const char *str, char **ptr, int base)
{
    int i;

    for (i = 0; str[i]; i++);
    *ptr = (char *) str;
}

unsigned long strtoul(const char *str, char **ptr, int base)
{
    int i;

    for (i = 0; str[i]; i++);
    *ptr = (char *) str;
}

#ifdef __linux
uintmax_t strtoumax(const char *str, char **ptr, int base)
{
  int i;

  for (i = 0; str[i]; i++);
  *ptr = (char *) str;
}
#endif

void swab(const void *from, void *to, ssize_t nbytes)
{
    int i;

    for (i = 0; i < nbytes; i++)
        ((char *) to)[i] = ((char *) from)[i];
}

int symlink(const char *oldpath, const char *newpath)
{
  int i;

  for (i = 0; oldpath[i]; i++);
  for (i = 0; newpath[i]; i++);
}

long sysconf(int name)
{
}

void syslog(int priority, const char *format, ...)
{
  int i;
  va_list ap;
  char *p;

  va_start (ap, format);

  for (i = 0; format[i]; i++);

  while ((p = va_arg (ap, char *)))
    *p;

  va_end(ap);
}

int system(const char *command)
{
    int i;

    for (i = 0; command[i]; i++);
}

double tan(double x)
{
}

double tanh(double x)
{
}

#ifdef _HPUX_SOURCE
int tcgetattr(int fildes, struct termios *termios_p)
{
    int i;

    termios_p->c_iflag = 0;
    termios_p->c_oflag = 0;
    termios_p->c_cflag = 0;
    termios_p->c_lflag = 0;
    termios_p->c_reserved = 0;

    for(i = 0; termios_p->c_cc[i] = 0; i++);
}

#ifndef _HPUX_SOURCE
int tcsetattr(int fildes, int optional_actions,
	      const struct termios *termios_p )
#else
int tcsetattr(int fildes, int optional_actions,
	      struct termios *termios_p )
#endif
{
    int local, i;

    local = termios_p->c_iflag;
    local = termios_p->c_oflag;
    local = termios_p->c_cflag;
    local = termios_p->c_lflag;
    local = termios_p->c_reserved;

    for(i = 0; termios_p->c_cc[i]; i++);
}
#elif defined WIN32
int tcgetattr(int fildes, struct termios *termios_p)
{
    int i;

    termios_p->c_iflag = 0;
    termios_p->c_oflag = 0;
    termios_p->c_cflag = 0;
    termios_p->c_lflag = 0;
    termios_p->c_ispeed = 0;
    termios_p->c_ospeed = 0;
    for(i = 0; termios_p->c_cc[i] = 0; i++);
}

int tcsetattr(int fildes, int optional_actions, 
	      const struct termios *termios_p )
{
    int local, i;

    local = termios_p->c_iflag;
    local = termios_p->c_oflag;
    local = termios_p->c_cflag;
    local = termios_p->c_lflag;
    local = termios_p->c_ispeed;
    local = termios_p->c_ospeed;
   
    for(i = 0; termios_p->c_cc[i]; i++);
}
#elif defined IA64LIN_SOURCE
int tcgetattr(int fildes, struct termios *termios_p)
{
    int i;

    termios_p->c_iflag = 0;
    termios_p->c_oflag = 0;
    termios_p->c_cflag = 0;
    termios_p->c_lflag = 0;
    
    for(i = 0; termios_p->c_cc[i] = 0; i++);
}

int tcsetattr(int fildes, int optional_actions, 
	      const struct termios *termios_p)
{
    int local, i;

    local = termios_p->c_iflag;
    local = termios_p->c_oflag;
    local = termios_p->c_cflag;
    local = termios_p->c_lflag;
   
    for(i = 0; termios_p->c_cc[i]; i++);
}
#else
int tcgetattr(int fildes, struct termios *termios_p)
{
    int i;

    termios_p->c_iflag = 0;
    termios_p->c_oflag = 0;
    termios_p->c_cflag = 0;
    termios_p->c_lflag = 0;
    
    for(i = 0; termios_p->c_cc[i] = 0; i++);
}

int tcsetattr(int fildes, int optional_actions, 
	      const struct termios *termios_p)
{
    int local, i;

    local = termios_p->c_iflag;
    local = termios_p->c_oflag;
    local = termios_p->c_cflag;
    local = termios_p->c_lflag;
   
    for(i = 0; termios_p->c_cc[i]; i++);
}
#endif

pid_t tcgetpgrp(int fd)
{
}

void *tdelete(const void *key, void **rootp,
	      int (*compar)(const void *, const void *))
{
  return key;
}

#ifdef __linux
char *textdomain(const char *domain_name)
{
  int i;

  for (i = 0; domain_name[i]; i++);

  return domain_name;
}
#endif

void *tfind(const void *key, void * const *rootp,
	    int (*compar)(const void *, const void *))
{
  return key;
}

time_t time(time_t *tloc)
{
    *tloc = 0;
}

clock_t times(struct tms *buffer)
{
    struct tms local;

    *buffer = local;
}

FILE *tmpfile(void)
{
}

char *tmpnam (char *s)
{
    return s;
}

#ifndef toascii
int toascii(int c)
{
}
#endif

int tolower(int c)
{
}

int toupper(int c)
{
}

wint_t towlower(wint_t wc)
{
}

wint_t towupper(wint_t wc)
{
}

void *tsearch(const void *key, void **rootp,
	      int (*compar)(const void *, const void *))
{
  return key;
}

char *ttyname(int fildes)
{
}

int ttyname_r(int filedes, char *name, size_t namelen)
{
    name[0] = 'a';
    return 0;
}

void twalk(const void *root, void (*action)(const void *nodep,
					    const VISIT which,
					    const int depth))
{
}

#if defined WIN32 || defined _HPUX_SOURCE
unsigned short umask(unsigned short cmask)
{
}
#else
unsigned int umask(unsigned int cmask)
{
}
#endif

int ungetc(int c, FILE *file)
{
    FILE local;

    *file = local;
}

int unlink(const char *path)
{
    int i;

    for (i = 0; path[i]; i++);
}

int utime(const char *path, const struct utimbuf *times)
{
    int i;
    struct utimbuf local;

    for (i = 0; path[i]; i++);
    local = *times; 
}

int utimes(const char *path, const struct timeval times[2])
{
    struct timeval local;

    local = times[0]; 
    local = times[1]; 
}

int vfprintf(FILE *file, const char *format, va_list sp)
{
    FILE local;
    int i;

    *file = local;
    for (i = 0; format[i]; i++);
}

#ifdef __linux
int vhangup()
{
}

int vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
  int i;

  for (i = 0; str[i]=format[i]; i++);
  while (i < size)
    {
      str[i]='\0';
      i++;
    }
}
#endif

#ifdef __linux
int vprintf(const char *format, va_list sp)
{
    FILE local;
    int i;

    *stdout = local;
    for (i = 0; format[i]; i++);
}
#elif defined WIN32
int vprintf(const char *format, va_list sp)
{
    FILE local;
    int i;

    _iob[0] = local;
    for (i = 0; format[i]; i++);
}
#else
int vprintf(const char *format, va_list sp)
{
    FILE local;
    int i;

    __iob[0] = local;
    for (i = 0; format[i]; i++);
}
#endif

int vsprintf(char *s, const char *format, va_list sp)
{
    FILE local;
    int i;

    *s = 0;
    for (i = 0; format[i]; i++);
}

#if (!defined(waddch) || defined(_HPUX_SOURCE)) && defined(HAVE_LIBCURSES)
int waddch(WINDOW *win, const chtype ch)
{
    WINDOW local;

    *win = local;
}
#endif

#if defined(_HPUX_SOURCE) && defined(HAVE_LIBCURSES)
int waddstr(WINDOW *win, const char *str)
{
    WINDOW local;
    int i;

    *win = local;
    for (i = 0; str[i]; i++);
}
#endif

#if !defined(waddnstr) && defined(HAVE_LIBCURSES)
int waddnstr(WINDOW *win, const char *str, int j)
{
    WINDOW local;
    int i;

    *win = local;
    for (i = 0; str[i]; i++);
}
#endif

pid_t wait(int *stat_loc)
{
    *stat_loc = 0;
}

pid_t waitpid(pid_t pid, int *stat_loc, int options)
{
    *stat_loc = 0;
}

#if !defined(WIN32) && defined(HAVE_LIBCURSES)
/* NT: macro in curses.h */
int wclear(WINDOW *win)
{
    WINDOW local;

    *win = local;
}
#endif

#if !defined(WIN32) && defined(HAVE_LIBCURSES)
int werase(WINDOW *win)
{
    WINDOW local;

    *win = local;
}
#endif

#ifdef HAVE_LIBCURSES
int wclrtobot(WINDOW *win)
{
    WINDOW local;

    *win = local;
}

int wclrtoeol(WINDOW *win)
{
    WINDOW local;

    *win = local;
}
#endif

#ifdef __linux
size_t wcrtomb(char *s, wchar_t wc, mbstate_t *ps)
{
  int i;

  for (i = 0; s[i]; i++);

  for (i = 0; i < sizeof(mbstate_t); i++)
    ((char *)ps)[i]='\0';
}

wctype_t wctype(const char *name)
{
  int i;

  for (i = 0; name[i]; i++);
}
#endif

#ifdef HAVE_LIBCURSES
int wmove(WINDOW *win, int y, int x)
{
    WINDOW local;

    *win = local;
}

int wrefresh(WINDOW *win)
{
    WINDOW local;

    *win = local;
}
#endif

ssize_t write(int fildes, const void *buf, size_t nbyte)
{
    int i;

    for (i = 0; ((char *) buf)[i]; i++);
}

ssize_t writev(int fildes, const struct iovec *vec, int count) 
{
  int i,j;
  struct iovec *vec_indx;
  for (j = 0; j < count ; j++) {
    for (i = 0; ((char *) vec[j].iov_base)[i]; i++);
  }
}


int wcscoll(const wchar_t *s1, const wchar_t *s2)
{
  int i;

  for (i = 0; s1[i] && s2[i]; i++);
}

#ifndef WIN32
/* NT: Macro in curses.h */
#if (!defined(wstandend) || defined(_HPUX_SOURCE)) && defined(HAVE_LIBCURSES)
int wstandend(WINDOW *win)
{
    WINDOW local;

    *win = local;
}
#endif

#if (!defined(wstandout) || defined(_HPUX_SOURCE)) && defined(HAVE_LIBCURSES)
int wstandout(WINDOW *win)
{
    WINDOW local;

    *win = local;
}
#endif
#endif

#if !defined WIN32 && (defined HAVE_LIBJPEG || defined _HPUX_SOURCE)
struct jpeg_error_mgr * jpeg_std_error(struct jpeg_error_mgr * err)
{
    struct jpeg_error_mgr * err_new = err;
}

void jpeg_CreateCompress (j_compress_ptr cinfo,
			  int version, size_t structsize)
{
    j_compress_ptr cinfo_new = cinfo; 
    
}

void jpeg_CreateDecompress (j_decompress_ptr cinfo, int version, 
			    size_t structsize)
{
    j_decompress_ptr cinfo_new = cinfo;
}

void jpeg_stdio_dest (j_compress_ptr cinfo, FILE * outfile)
{
    FILE *outfile_new = outfile;
    j_compress_ptr cinfo_new = cinfo; 
}
void jpeg_stdio_src (j_decompress_ptr cinfo, FILE * infile)
{
    FILE *infile_new = infile;
    j_decompress_ptr cinfo_new = cinfo;
}

int jpeg_read_header (j_decompress_ptr cinfo,
		      boolean require_image)
{
    j_decompress_ptr cinfo_new = cinfo;
}

void jpeg_start_compress (j_compress_ptr cinfo,
			  boolean write_all_tables)
{
    j_compress_ptr cinfo_new = cinfo;     
}

void jpeg_finish_compress (j_compress_ptr cinfo)
{
    j_compress_ptr cinfo_new = cinfo;     

}

JDIMENSION jpeg_read_scanlines (j_decompress_ptr cinfo,
				JSAMPARRAY scanlines,
				JDIMENSION max_lines)
{
    j_decompress_ptr cinfo_new = cinfo;     
}

boolean jpeg_start_decompress (j_decompress_ptr cinfo)
{
    j_decompress_ptr cinfo_new = cinfo;
}

boolean jpeg_finish_decompress (j_decompress_ptr cinfo)
{
    j_decompress_ptr cinfo_new = cinfo;
}

void jpeg_destroy_compress (j_compress_ptr cinfo)
{
    j_compress_ptr cinfo_new = cinfo;     
}
void jpeg_destroy_decompress (j_decompress_ptr cinfo)
{
    j_decompress_ptr cinfo_new = cinfo;
}

void jpeg_set_defaults (j_compress_ptr cinfo)
{
    j_compress_ptr cinfo_new = cinfo;     
}

void jpeg_set_colorspace (j_compress_ptr cinfo,
			  J_COLOR_SPACE colorspace)
{
    j_compress_ptr cinfo_new = cinfo;

}

void jpeg_default_colorspace (j_compress_ptr cinfo)
{
    j_compress_ptr cinfo_new = cinfo;
}

void jpeg_set_quality (j_compress_ptr cinfo, int quality,
                                   boolean force_baseline)
{
    j_compress_ptr cinfo_new = cinfo;
}

JDIMENSION jpeg_write_scanlines (j_compress_ptr cinfo,
                                             JSAMPARRAY scanlines,
                                             JDIMENSION num_lines)
{
    j_compress_ptr cinfo_new = cinfo;
    JSAMPARRAY s = scanlines;
}

#endif

#ifdef HAVE_LIBGLUT
void glutInit (int *argcp, char **argvp)
{
  int i, j;

  for (i = 0; i < *argcp; i++)
    for (j = 0; argvp[i][j]; j++);

  *argcp=0;
}

void glutMainLoop()
{
}

void glutInitWindowPosition (int x, int y)
{
}

void glutInitWindowSize (int width, int height)
{
}

void glutInitDisplayMode (unsigned int mode)
{
}

int glutCreateWindow (char *name)
{
  int i;

  for (i = 0; name[i]; i++);
}

void glutKeyboardFunc (void (*func)(unsigned char key, int x, int y))
{
}

void glutSpecialFunc (void (*func)(int key, int x, int y))
{
}

void glutKeyboardUpFunc (void (*func)(unsigned char key, int x, int y))
{
}

void glutSpecialUpFunc (void (*func)(int key, int x, int y))
{
}

void glutDisplayFunc (void (*func)(void))
{
}

void glutIdleFunc (void (*func)(void))
{
}

void glutVisibilityFunc (void (*func)(int state))
{
}

#endif

#ifdef HAVE_LIBGLU
void gluOrtho2D (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top)
{
}
#endif

#ifdef HAVE_LIBGL
void glMatrixMode (GLenum mode)
{
}
#endif

#if defined X86LIN_SOURCE
__const unsigned short int **__ctype_b_loc (void) __attribute__ ((__const));

__const unsigned short int **__ctype_b_loc (void)
{
  return NULL;
}

__const __int32_t **__ctype_tolower_loc (void) __attribute__ ((__const));

__const __int32_t **__ctype_tolower_loc (void)
{
  return NULL;
}

__const __int32_t **__ctype_toupper_loc (void) __attribute__ ((__const));

__const __int32_t **__ctype_toupper_loc (void)
{
  return NULL;
}
#endif

#if defined IA64LIN_SOURCE

#endif




