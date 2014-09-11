###############################################################################
##
##                    Illinois Open Source License
##                     University of Illinois/NCSA
##                         Open Source License
##
## Copyright (c) 2004, The University of Illinois at Urbana-Champaign.
## All rights reserved.
##
## Developed by:
##
##              IMPACT Research Group
##
##              University of Illinois at Urbana-Champaign
##
##              http://www.crhc.uiuc.edu/IMPACT
##              http://www.gelato.org
##
## Permission is hereby granted, free of charge, to any person
## obtaining a copy of this software and associated documentation
## files (the "Software"), to deal with the Software without
## restriction, including without limitation the rights to use, copy,
## modify, merge, publish, distribute, sublicense, and/or sell copies
## of the Software, and to permit persons to whom the Software is
## furnished to do so, subject to the following conditions:
##
## Redistributions of source code must retain the above copyright
## notice, this list of conditions and the following disclaimers.
##
## Redistributions in binary form must reproduce the above copyright
## notice, this list of conditions and the following disclaimers in
## the documentation and/or other materials provided with the
## distribution.
##
## Neither the names of the IMPACT Research Group, the University of
## Illinois, nor the names of its contributors may be used to endorse
## or promote products derived from this Software without specific
## prior written permission.  THE SOFTWARE IS PROVIDED "AS IS",
## WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
## LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
## PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
## CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
## OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
## OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
## OR THE USE OR OTHER DEALINGS WITH THE SOFTWARE.
##
###############################################################################

# Process this file with autoconf to produce a configure script.
AC_INIT([openimpact], [1.0rc4],  [rkidd@crhc.uiuc.edu])
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([1.6])
AC_CONFIG_SRCDIR([src/Lcode/Lcode/l_code.c])
AM_CONFIG_HEADER([include/config.h])

AC_ARG_ENABLE([cxx], AC_HELP_STRING([--enable-cxx],
			            [compile experimental C++ support]),
              AM_CONDITIONAL([USE_CXX],
			     [test "x${enable_cxx}" = "xyes"]),
              AM_CONDITIONAL([USE_CXX], [false]))
AC_ARG_ENABLE([libcurses-pip],
	      AC_HELP_STRING([--enable-libcurses-pip],
			     [compile PIP summaries for libcurses]),
	      AM_CONDITIONAL([HAVE_LIBCURSES],
                             [test "x${enable_libcurses_pip}" = "xyes"]),
	      AM_CONDITIONAL([HAVE_LIBCURSES], [false]))
AC_ARG_ENABLE([libjpeg-pip],
	      AC_HELP_STRING([--disable-libjpeg-pip],
		             [do not compile PIP summaries for libjpeg]),
	      AM_CONDITIONAL([HAVE_LIBJPEG],
	                     [test "x${enable_libjpeg_pip}" = "xyes"]),
              enable_libjpeg_pip="yes"
	      AM_CONDITIONAL([HAVE_LIBJPEG], [true]))
AC_ARG_ENABLE([libGL-pip],
	      AC_HELP_STRING([--disable-libGL-pip],
		             [do not compile PIP summaries for libGL]),
	      AM_CONDITIONAL([HAVE_LIBGL],
			     [test "x${enable_libGL_pip}" = "xyes"]),
              enable_libGL_pip="yes"
	      AM_CONDITIONAL([HAVE_LIBGL], [true]))
AC_ARG_ENABLE([libGLU-pip],
	      AC_HELP_STRING([--disable-libGLU-pip],
		             [do not compile PIP summaries for libGLU]),
	      AM_CONDITIONAL([HAVE_LIBGLU],
			     [test "x${enable_libGLU_pip}" = "xyes"]),
              enable_libGLU_pip="yes"
	      AM_CONDITIONAL([HAVE_LIBGLU], [true]))
AC_ARG_ENABLE([libglut-pip],
	      AC_HELP_STRING([--disable-libglut-pip],
		             [do not compile PIP summaries for libglut]),
	      AM_CONDITIONAL([HAVE_LIBGLUT],
			     [test "x${enable_libglut_pip}" = "xyes"]),
              enable_libglut_pip="yes"
	      AM_CONDITIONAL([HAVE_LIBGLUT], [true]))
AC_ARG_ENABLE([openssl-pip],
	      AC_HELP_STRING([--enable-openssl-pip],
		             [compile PIP summaries for OpenSSL]),
	      AM_CONDITIONAL([HAVE_LIBSSL],
			     [test "x${enable_openssl_pip}" = "xyes"]),
	      AM_CONDITIONAL([HAVE_LIBSSL], [false]))
AC_ARG_ENABLE([libz-pip],
	      AC_HELP_STRING([--disable-libz-pip],
		             [do not compile PIP summaries for libz]),
	      AM_CONDITIONAL([HAVE_LIBZ],
			     [test "x${enable_libz_pip}" = "xyes"]),
              enable_libz_pip="yes"
	      AM_CONDITIONAL([HAVE_LIBZ], [true]))

# Checks for programs.
AC_PROG_AWK
#AC_PROG_CXX
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL
AM_PROG_LEX
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_YACC
AC_PROG_RANLIB

# Checks for libraries.
AC_CHECK_LIB([elf], [elf_begin],
	     [AC_DEFINE([HAVE_LIBELF], 1,
			[Define to 1 if you have the `elf' library (-lelf).])
	      AM_CONDITIONAL([HAVE_LIBELF], [true])],
             [if test `echo ${host} | sed -n -e '/^ia64-.*-linux.*/p'`; then
                  AC_MSG_ERROR([[OpenIMPACT requires libelf.  If you have
                                 libelf installed in a non-standard location,
                                 you may need to specify the -I and -L
                                 options to your compiler.]])
	      else
		  AM_CONDITIONAL([HAVE_LIBELF], [false])
              fi], [])

if test "x${enable_libcurses_pip}" = "xyes"; then
  AC_CHECK_LIB([curses], [initscr], 
	       [AC_DEFINE([HAVE_LIBCURSES], 1, 
	                  [Define to 1 if you have the `curses' library
		  	   (-lcurses).])
	        AM_CONDITIONAL([HAVE_LIBCURSES], [true])],
	       [AM_CONDITIONAL([HAVE_LIBCURSES], [false])], [])
fi
if test "x${enable_libjpeg_pip}" = "xyes"; then
  AC_CHECK_LIB([jpeg], [jpeg_read_header],
	       [AC_DEFINE([HAVE_LIBJPEG], 1,
			  [Define to 1 if you have the `jpeg' library
                           (-ljpeg).])
	        AM_CONDITIONAL([HAVE_LIBJPEG], [true])], 
	       [AM_CONDITIONAL([HAVE_LIBJPEG], [false])], [])
fi
if test "x${enable_libGL_pip}" = "xyes"; then
  AC_CHECK_LIB([GL], [glMatrixMode],
	       [AC_DEFINE([HAVE_LIBGL], 1,
			  [Define to 1 if you have the `GL' library (-lGL).])
	        AM_CONDITIONAL([HAVE_LIBGL], [true])], 
	       [AM_CONDITIONAL([HAVE_LIBGL], [false])], [-lm])
fi
if test "x${enable_libGLU_pip}" = "xyes"; then
  AC_CHECK_LIB([GLU], [gluOrtho2D],
	       [AC_DEFINE([HAVE_LIBGLU], 1,
			  [Define to 1 if you have the `GLU' library (-lGLU).])
	        AM_CONDITIONAL([HAVE_LIBGLU], [true])], 
	       [AM_CONDITIONAL([HAVE_LIBGLU], [false])], [-lGL -lm])
fi
if test "x${enable_libglut_pip}" = "xyes"; then
  AC_CHECK_LIB([glut], [glutInit], 
	       [AC_DEFINE([HAVE_LIBGLUT], 1,
			  [Define to 1 if you have the `glut' library
                           (-lglut).])
	        AM_CONDITIONAL([HAVE_LIBGLUT], [true])], 
	       [AM_CONDITIONAL([HAVE_LIBGLUT], [false])], [-lGL -lGLU -lm])
fi
if test "x${enable_openssl_pip}" = "xyes"; then
  AC_CHECK_LIB([ssl], [SSLeay],
               [AC_DEFINE([HAVE_LIBSSL], 1,
			  [Define to 1 if you have the `ssl' library (-lssl).])
	        AM_CONDITIONAL([HAVE_LIBSSL], [true])],
	       [AM_CONDITIONAL([HAVE_LIBSSL], [false])], [])
fi
if test "x${enable_libz_pip}" = "xyes"; then
  AC_CHECK_LIB([z], [zlibVersion],
	       [AC_DEFINE([HAVE_LIBZ], 1,
			  [Define to 1 if you have the `z` library (-lz).])
	        AM_CONDITIONAL([HAVE_LIBZ], [true])],
	       [AM_CONDITIONAL([HAVE_LIBZ], [false])], [])
fi

# Checks for header files.
AC_FUNC_ALLOCA
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([arpa/inet.h fcntl.h float.h limits.h locale.h malloc.h \
                  memory.h netdb.h netinet/in.h nlist.h sgtty.h stddef.h \
                  stdlib.h string.h strings.h sys/file.h sys/ioctl.h \
		  sys/ipc.h sys/param.h sys/shm.h sys/socket.h sys/time.h \
		  sys/timeb.h termio.h termios.h unistd.h utime.h values.h \
		  wchar.h])
AC_CHECK_HEADERS([popt.h],
		 [AM_CONDITIONAL([HAVE_POPT_H], [true])],
		 [AM_CONDITIONAL([HAVE_POPT_H], [false])])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STAT
AC_C_CONST
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_MODE_T
AC_FUNC_OBSTACK
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_CHECK_MEMBERS([struct stat.st_blksize])
AC_STRUCT_ST_BLOCKS
AC_CHECK_MEMBERS([struct stat.st_rdev])
AC_HEADER_TIME
AC_STRUCT_TM
AC_CHECK_HEADERS([sys/resource.h])
# Determine if libelf.h is in libelf/libelf.h or just libelf.h
AC_CHECK_HEADERS([libelf/libelf.h])
AC_CHECK_SIZEOF([void *])
AC_CHECK_SIZEOF([long])
AC_CHECK_SIZEOF([int])

AC_CHECK_TYPES([ptrdiff_t, __rlimit_resource_t, enum __rlimit_resource,
                __priority_which_t, enum __priority_which], [], [],
               [#ifdef HAVE_SYS_RESOURCE_H
                #include <sys/resource.h>
                #endif])
AC_CHECK_TYPE([enum __rlimit_resource], [enum___rlimit_resource_defined=1],
	      [enum___rlimit_resource_defined=0],
	      [#ifdef HAVE_SYS_RESOURCE_H
	       #include <sys/resource.h>
	       #endif])
AC_CHECK_TYPE([enum __priority_which], [enum___priority_which_defined=1],
	      [enum___priority_which_defined=0],
	      [#ifdef HAVE_SYS_RESOURCE_H
	       #include <sys/resource.h>
	       #endif])

# See if we're compiling on RedHat 6.1
if test -f /etc/redhat-release && \
   test -n "`grep '6\.1' /etc/redhat-release`" ; then
	AC_DEFINE([REDHAT6_1], 1, [Set to 1 when compiling on RedHat 6.1])
fi

# See if we're trying to compile impact with impact
AC_MSG_CHECKING([[if compiler is oicc]])
AC_RUN_IFELSE(AC_LANG_PROGRAM([[]],
			      [[#ifdef __OICC__
					return(0);
				#else
					return(1);
				#endif]]),
	      AM_CONDITIONAL([COMPILER_IS_OICC], [true])
	      AC_MSG_RESULT([[yes]]),
	      AM_CONDITIONAL([COMPILER_IS_OICC], [false])
	      AC_MSG_RESULT([[no]]))
	
# Checks for library functions.
AC_FUNC_CHOWN
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_GETGROUPS
AC_FUNC_GETPGRP
AC_PROG_GCC_TRADITIONAL
AC_FUNC_LSTAT
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_MKTIME
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_FUNC_SETPGRP
AC_FUNC_SETVBUF_REVERSED
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_STRCOLL
AC_FUNC_STRFTIME
AC_FUNC_STRTOD
AC_FUNC_UTIME_NULL
AC_FUNC_VPRINTF
AC_FUNC_WAIT3
AC_CHECK_FUNCS([alarm atexit bzero dup2 euidaccess fchdir fdatasync floor \
                ftime ftruncate getcwd gethostbyname gethostname getpagesize \
                getpass gettimeofday getusershell getwd isascii lchown \
                localeconv mblen memchr memmove memset modf munmap pathconf \
                pow regcomp rint rmdir select sethostname setlocale socket \
                sqrt stpcpy strcasecmp strchr strcspn strdup strerror \
                strncasecmp strpbrk strrchr strspn strstr strtol strtoul \
                uname utime])

# TODO: Change all scripts and programs so that autoconf gives them the needed
#       values and these environment variables can be dropped altogether.
AM_CONDITIONAL([TARGET_PLATFORM_IA64LIN_GCC], [false])
AM_CONDITIONAL([TARGET_PLATFORM_X86LIN_GCC], [false])
AM_CONDITIONAL([TARGET_PLATFORM_HP_CC], [false])
AM_CONDITIONAL([TARGET_PLATFORM_HP_GCC], [false])
AM_CONDITIONAL([HOST_PLATFORM_IA64LIN_GCC], [false])
AM_CONDITIONAL([HOST_PLATFORM_X86LIN_GCC], [false])
AM_CONDITIONAL([HOST_PLATFORM_HP_CC], [false])
AM_CONDITIONAL([HOST_PLATFORM_HP_GCC], [false])
AM_CONDITIONAL([LP64_ARCHITECTURE], [false])

# Set an extra automake flag if we're on x86_64.  This is more or less
# the same as x86, but we may need to specify special flags to force
# 32 bitness.
if [ test -z ${host##*86_64-*-linux*} ]; then
	AM_CONDITIONAL([HOST_X86_64], [true])
else
	AM_CONDITIONAL([HOST_X86_64], [false])
fi

# Set the appropriate preprocessor symbols for our host architecture
case $host in
ia64-*-linux*)
	AC_DEFINE([IT64BIT], 1, 
                  [Set to 1 when compiling on a 64 bit capable architecture])
	AC_DEFINE([IA64LIN_SOURCE], 1, [Set to 1 when compiling on IA-64])
	AC_DEFINE([__linux], 1, [Set to 1 when compiling on x86 Linux])
	AC_DEFINE([LP64_ARCHITECTURE], 1, 
                  [Set to 1 when compiling on a platform with 64 bit longs and
                   pointers.])
	AM_CONDITIONAL([LP64_ARCHITECTURE], [true])
	AC_SUBST([HOST_PLATFORM], [ia64lin])
	AC_SUBST([HOST_COMPILER], [gcc])
	AM_CONDITIONAL([HOST_PLATFORM_IA64LIN_GCC], [true])
	AM_CONDITIONAL([HOST_PLATFORM_X86LIN_GCC], [false])
	AM_CONDITIONAL([HOST_PLATFORM_HP_CC], [false])
	AM_CONDITIONAL([HOST_PLATFORM_HP_GCC], [false])
	;;
hp*)
	AC_DEFINE([IT64BIT], 1, 
                  [Set to 1 when compiling on a 64 bit capable architecture])
	AC_DEFINE([_HPUX_SOURCE], 1, [Set to 1 when compiling on HPUX])
	AC_SUBST([HOST_PLATFORM], [hp])
	case $GCC in
	yes)
		AC_SUBST([HOST_COMPILER], [gcc])
		AM_CONDITIONAL([HOST_PLATFORM_IA64LIN_GCC], [false])
		AM_CONDITIONAL([HOST_PLATFORM_X86LIN_GCC], [false])
		AM_CONDITIONAL([HOST_PLATFORM_HP_CC], [false])
		AM_CONDITIONAL([HOST_PLATFORM_HP_GCC], [true])
		;;
	*)
		AC_SUBST([HOST_COMPILER], [cc])
		AM_CONDITIONAL([HOST_PLATFORM_IA64LIN_GCC], [false])
		AM_CONDITIONAL([HOST_PLATFORM_X86LIN_GCC], [false])
		AM_CONDITIONAL([HOST_PLATFORM_HP_CC], [true])
		AM_CONDITIONAL([HOST_PLATFORM_HP_GCC], [false])
		;;
	esac
	;;
i*86-*-linux*|*86_64-*-linux*)
	AC_DEFINE([IT64BIT], 1, 
                  [Set to 1 when compiling on a 64 bit capable architecture])
	AC_DEFINE([X86LIN_SOURCE], 1, [Set to 1 when compiling on x86])
	AC_DEFINE([__linux], 1, [Set to 1 when compiling on x86 Linux])
	AC_SUBST([HOST_PLATFORM], [x86lin])
	AC_SUBST([HOST_COMPILER], [gcc])
	AM_CONDITIONAL([HOST_PLATFORM_IA64LIN_GCC], [false])
	AM_CONDITIONAL([HOST_PLATFORM_X86LIN_GCC], [true])
	AM_CONDITIONAL([HOST_PLATFORM_HP_CC], [false])
	AM_CONDITIONAL([HOST_PLATFORM_HP_GCC], [false])
	;;
esac	

# Set the appropriate preprocessor symbols for our target architecture
case $target in
ia64-*-linux*)
	AC_SUBST([TARGET_PLATFORM], [ia64lin])
	AC_SUBST([TARGET_COMPILER], [gcc])
	AM_CONDITIONAL([TARGET_PLATFORM_IA64LIN_GCC], [true])
	AM_CONDITIONAL([TARGET_PLATFORM_X86LIN_GCC], [false])
	AM_CONDITIONAL([TARGET_PLATFORM_HP_CC], [false])
	AM_CONDITIONAL([TARGET_PLATFORM_HP_GCC], [false])
	;;
hp*)
	AC_SUBST([TARGET_PLATFORM], [hp])
	case $GCC in
	yes)
		AC_SUBST([TARGET_COMPILER], [gcc])
		AM_CONDITIONAL([TARGET_PLATFORM_IA64LIN_GCC], [false])
		AM_CONDITIONAL([TARGET_PLATFORM_X86LIN_GCC], [false])
		AM_CONDITIONAL([TARGET_PLATFORM_HP_CC], [false])
		AM_CONDITIONAL([TARGET_PLATFORM_HP_GCC], [true])
		;;
	*)
		AC_SUBST([TARGET_COMPILER], [cc])
		AM_CONDITIONAL([TARGET_PLATFORM_IA64LIN_GCC], [false])
		AM_CONDITIONAL([TARGET_PLATFORM_X86LIN_GCC], [false])
		AM_CONDITIONAL([TARGET_PLATFORM_HP_CC], [true])
		AM_CONDITIONAL([TARGET_PLATFORM_HP_GCC], [false])
		;;
	esac
	;;
i*86-*-linux*|*86_64-*-linux*)
	AC_SUBST([TARGET_PLATFORM], [x86lin])
	AC_SUBST([TARGET_COMPILER], [gcc])
	AM_CONDITIONAL([TARGET_PLATFORM_IA64LIN_GCC], [false])
	AM_CONDITIONAL([TARGET_PLATFORM_X86LIN_GCC], [true])
	AM_CONDITIONAL([TARGET_PLATFORM_HP_CC], [false])
	AM_CONDITIONAL([TARGET_PLATFORM_HP_GCC], [false])
	;;
esac	

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
AC_MSG_NOTICE([writing oicc version info])
echo "package_version = \"${PACKAGE_VERSION}\"" > \
     ${srcdir}/driver/openimpact.py
