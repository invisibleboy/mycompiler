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
/*==================================================================
 *  File          : impact_lemul_lib.c
 *  Description   : Routine to send trace info from program into
 *                  into a Unix named pipe
 * 
 *  Created by Yoji Yamada, John C. Gyllenhaal, Wen-mei Hwu 7/92
 *  Revised for new prober by John C. Gyllenhaal 5/93
 *  Ported to new emulation framework by Qudus Olaniran 1/98
 *  Reorganized and portability enhanced by IMPACT Technologies, Inc (JCG) 1/99
 *  New non-trapping load support by IMPACT Technologies, Inc. (JCG) 3/99
 * 
 * The following command was used to generate impact_lemul_lib.o: 
 *   gcc -c -O -I${IMPACT_REL_PATH}/include impact_lemul_lib.c
*==================================================================*/
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <Lcode/l_trace_interface.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <setjmp.h>

#define _IA64_USE_GENERAL_SPECULATION

/* It is not safe to change these unless you change Lemulate also */
#define BUF_MARK  5000
#define BUF_SIZE 10000

/* Global error number varable used by the pipe/file functions */
extern int      errno;

/* Permissions used to trace fifo */
#define PERMS           0666

/* Global variables */
char _TR_MEM_BUF[BUF_SIZE];
char *_TR_MEM_PTR = _TR_MEM_BUF;
char *_TR_MEM_FLUSH_PTR = _TR_MEM_BUF + BUF_MARK;
int _TR_FD = -1;
long _TR_COUNT = 0;
int _TR_AT_END = 0;
int _TR_IN_PARENT = 1;
int _TR_SYSTEM_INITIALIZED = 0;

/* Allow tracing to be controlled by a few global parameters.
 * Primarily for debugging the instrumentation code.
 * Controlled by Lemulate parameters.
 */
int _TR_TRIGGER_DUMP_SIZE = BUF_MARK;
int _TR_TRANSMIT_TRACE=1;
int _TR_TRACE_FLAGS = 0;

/* Use global space to store tr_ptr, tr_mark, and the
 * previous contents of the three trace registers so that
 * call-back routines like those found when using the qsort
 * library routines in perl, do not trash library registers.
 */
int _TR_GLOBAL_SPACE[16];

/* Flags when an exception is masked */
int _TR_EXCEPTION_MASKED = 0;

/* When non-zero, mask any exceptions caused by program */
int _TR_MASK_EXCEPTION = 0;

/* Flags that the non-trapping load signal handler is installed.
 * Initially set to 1 and it will be reset 'manually' whenever the user
 * calls signal() (or a related function), so that our handler
 * gets reinstalled properly.
 */
int _EM_INSTALL_TRAP_HANDLER = 1;

/* setjmp/longjmp state for handling non-trapping loads 
 * Used by _EM_NTload_xxx and _EM_trap_signal_handler.
 */
jmp_buf _EM_pretrap_env;

/* Internal prototypes */
void _TR_DUMP_TRACE();

/* Should only be called once, due to the atexit() function call.
 * Flushes the trace and marks trace as ending cleanly.
 * All further trace info will be thrown away after this call.
 */
void _TR_TRACE_FINISHED()
{
    /* Flush any remaining trace info */
    _TR_DUMP_TRACE();

    /* Flag end of trace by moving the trace pointer to 1 byte
     * before the beginning of the buffer 
     */
    _TR_MEM_PTR = _TR_MEM_BUF - 1;
    _TR_DUMP_TRACE();
}

/* Should only be called once, when the first instrumented function
 * is entered.  Currently used to set up cleanup code (via atexit()).
 */
void _TR_INITIALIZE_TRACE_SYSTEM(int flags)
{
    /* Do nothing if have already initialized */
    if (_TR_SYSTEM_INITIALIZED)
	return;
    
    /* Set trace flags using passed flags */
    _TR_TRACE_FLAGS = flags;

    /* Cause _TR_TRACE_FINISHED() to be called when program exits normally */
    if (atexit (_TR_TRACE_FINISHED) != 0)
    {
	fprintf (stderr, "\n_TR_INITIALIZE_TRACE_SYSTEM: atexit() failed!\n");
	exit (1);
    }
    
    /* Flag the trace system initialized */
    _TR_SYSTEM_INITIALIZED = 1;
}

void _TR_DUMP_TRACE()
{
  int i;
  int size;
  int temp;
  int flag_buf[5];
  int flag_size;
  char name_buf[40];
  char link_buf[40];

  /*
   * If not in parent, throw away trace info and return.
   */
  if (!_TR_IN_PARENT)
  {
      _TR_MEM_PTR = _TR_MEM_BUF;
      return;
  }

  /*
   * If trace should have ended, warn.
   */
  if (_TR_AT_END)
  {
      size = _TR_MEM_PTR - _TR_MEM_BUF;
      fprintf (stderr,
               "_DUMP_TRACE called after end of trace (with %i bytes)\n",
               size);
      _TR_MEM_PTR = _TR_MEM_BUF;
      return;
  }

  if (_TR_TRANSMIT_TRACE)
  {
      /*
       * Detect first trace dump (if _TR_FD == -1)
       *
       * Do all trace starup code here.
       *
       * Make Unix named pipe (aka fifo), and open it to send trace to
       * simulation, etc.
       * Then L_TRACE_START_FORMAT3, _TR_TRACE_FLAGS, and 
       * L_TRACE_SAMPLE_START codes in front of
       * the first write of the trace.  This allows the simulator to
       * check byte order (if written to file and used on a differenct 
       * machine.)
       * It also allows a sampled trace to be used.
       */
      if (_TR_FD == -1)
      {
	  /* Use process group id to make trace pipe name */
	  sprintf (link_buf, "trace.pipe.%i", getpgrp());
	  sprintf (name_buf, "/tmp/%s", link_buf);
	  
	  /* Make fifo, ok if already exists */
	  if ((mknod (name_buf, S_IFIFO | PERMS, 0) < 0) && (errno != EEXIST))
          {
	     fprintf (stderr, 
		      "Cannot create trace fifo %s, unix error no %i\n",
		       name_buf, errno);
	      exit (1);
          }
	  
	  /* Create link to the name buf in current directory, ok if
	   * already exists -JCG 10/18/96
	   */
	  if ((symlink (name_buf, link_buf) == -1) && (errno != EEXIST))
          {
	      fprintf (stderr, "Unable to link '%s' to '%s'\n", link_buf,
		       name_buf);
	      exit (1);
	      
          }
	  
	  /* Open it for writing */
	  if ((_TR_FD = open (name_buf, 1, PERMS)) < 0)
          {
	      fprintf (stderr, "Unable to open trace fifo %s for writing\n",
		       name_buf);
	      exit (1);
          }
	  
	  /* Remove named pipe and link (neither are necessary at this point)
	   * Ok if already removed by Lsim/Lprofile.
	   */
	  unlink (name_buf);
	  unlink (link_buf);
	  
	  /* Insert L_TRACE_START_FORMAT3, _TR_TRACE_FLAGS, and 
	   * L_TRACE_SAMPLE_START at beginning of trace -ITI (JCG) 1/99
	   */
	  flag_buf[0] = L_TRACE_START_FORMAT3;
	  flag_buf[1] = _TR_TRACE_FLAGS;
	  flag_buf[2] = L_TRACE_SAMPLE_START;
	  flag_size = 3 * sizeof(int);
	  if (write (_TR_FD, flag_buf, flag_size ) != flag_size)
          {
	      fprintf (stderr,
		       "Error writing start trace flags to trace fifo\n");
	      exit (1);
          }
      }
  }


  size = _TR_MEM_PTR - _TR_MEM_BUF;

  /*
   * Detect end of trace  (size == -1)
   *
   * To get size == -1, on exit, probed programs calls _TRACE_DUMP,
   * decrements TR_PTR by one, then call _TRACE_DUMP again.
   */
  if (size == -1)
  {
      _TR_AT_END = 1;

      if (_TR_TRANSMIT_TRACE)
      {
	  flag_buf[0] = L_TRACE_SAMPLE_END;
	  flag_buf[1] = L_TRACE_END;
	  flag_size = 2 * sizeof(int);
	  if (write (_TR_FD, flag_buf, flag_size) != flag_size)
	  {
	      fprintf (stderr, 
		       "Error writing end trace flags to trace fifo\n");
	      exit (1);
	  }
	  close (_TR_FD);
      }

      _TR_FD = -1;
      _TR_MEM_PTR = _TR_MEM_BUF;
      return;
  }

  /* Make sure trace buffer did not overflow */
  if (size > BUF_SIZE)
  {
      fprintf (stderr, "*** Error, trace buffer size exceeded, max %i (%i)!\n",
               BUF_SIZE, size);
      fprintf (stderr, "*** Overflow occured after %i bytes written\n",
               _TR_COUNT);
      fprintf (stderr,
               "*** Writing out last part of trace to facilitate debugging\n");

      /* Write out last part of trace to help debug */
      if (_TR_TRANSMIT_TRACE)
      {
	  /* Write trace to trace fifo */
	  if (write (_TR_FD, &_TR_MEM_BUF[0], size) != size)
	  {
	      fprintf (stderr, 
		       "*** Error writing trace to trace fifo (size %i)\n",
		       size);
	      exit (1);
	  }
      }

      exit (1);
  }

  /* Keep trace count for debugging purposes */
  _TR_COUNT += size;

  if (_TR_TRANSMIT_TRACE)
  {
      /* Write trace to trace fifo */
      if (write (_TR_FD, &_TR_MEM_BUF[0], size) != size)
      {
	  fprintf (stderr, "*** Error writing trace to trace fifo (size %i)\n",
		   size);
	  exit (1);
      }
  }

  _TR_MEM_PTR = _TR_MEM_BUF;
  return;
}


/* 
 * Trap signal handler to support _EM_NTload_xxx() functions.
 * After checking that trap should be ignored and reinstalling itself,
 * it does a longjmp() to _EM_pretrap_env (which should be set
 * by _EM_NTload_xxx() functions). -ITI/JCG 3/99.
 */
static void _EM_trap_signal_handler (int sig, int code, struct sigcontext *scp)
{
    /* Detect exceptions in code other than masked instructions */
    if (_TR_MASK_EXCEPTION == 0)
    {
        if (sig == SIGBUS)
        {
  	   fprintf (stderr, 
                    "\n_EM_trap_signal_handler: Unexpected SIGBUS caught"
		    "(not at a non-trapping load)!\n");
        }
        else if (sig == SIGSEGV)
        {
  	   fprintf (stderr, 
                    "\n_EM_trap_signal_handler: Unexpected SIGSEGV caught"
		    "(not at a non-trapping load)!\n");
        }
        else if (sig == SIGILL)
        {
  	   fprintf (stderr, 
                    "\n_EM_trap_signal_handler: Unexpected SIGILL caught"
		    "(not at a non-trapping load)!\n");
        }
	fprintf (stderr, "Aborting program with coredump (for debugging).\n");
        abort();
    }

    /* Reinstall trap signal handler (uninstalls itself when signal called) */
    if (signal (sig, (void *)_EM_trap_signal_handler) == SIG_ERR)
    {
	fprintf (stderr, "\n_EM_trap_signal_handler: Unable to reinstall "
		 "signal handler!\n");
	abort();
    }

    /* Jump back to EM_NTload_xxx() function, signally that load caused trap*/
    longjmp (_EM_pretrap_env, 1);
}

/* 
 * Initialize trap signal handler to support _EM_NTload_xxx() functions.
 * Installs _EM_trap_signal_handler for all the signals that non-trapping
 * load can cause.
 *
 * Note: This function may be called several times during program 
 * execution, primarily after the user-program calls signal. -ITI/JCG 3/99
 */
static void _EM_install_trap_signal_handlers ()
{
    /* Install trap signal handler for each signal a non-trapping load c
     * an cause.
     */
    if (signal (SIGSEGV, (void *)_EM_trap_signal_handler) == SIG_ERR)
    {
	fprintf (stderr, 
		 "\n_EM_install_trap_signal_handlers: Unable to install\n"
		 "signal handler for SIGSEGV!\n");
	abort();
    }

    if (signal (SIGBUS, (void *)_EM_trap_signal_handler) == SIG_ERR)
    {
	fprintf (stderr, 
		 "\n_EM_install_trap_signal_handlers: Unable to install\n"
		 "signal handler for SIGBUS!\n");
	abort();
    }

  if (signal (SIGILL, (void *)_EM_trap_signal_handler) == SIG_ERR)
    {
	fprintf (stderr, 
		 "\n_EM_install_trap_signal_handlers: Unable to install\n"
		 "signal handler for SIGSEGV!\n");
	abort();
    }

    /* Flag that the trap handler has been installed (doesn't need
     * to be installed again).
     */
    _EM_INSTALL_TRAP_HANDLER = 0;
}


/* Emulate a non-trapping char load in a (hopefully) portable fashion */
char _EM_NTload_char (char *ptr)
{
    char ret_value;
    int mask_exception = 0;

    /* To save exception overhead, handle common case of NULL pointer,
     * offsets off of NULL.
     */
    if (((unsigned long)ptr) < ((unsigned long)4096))
    {
	/* If got here, an exception was masked.  Flag as such */
	_TR_EXCEPTION_MASKED = 1;

	/* Return 0 for value */
	return (0);
    }

#ifdef _IA64_USE_GENERAL_SPECULATION

    __asm__ __volatile__("ld1.s %0 = [%2];; sxt1 %0 = %0;;"
                         /* check for NAT */
                         "tnat.nz p6, p0 = %0;;"
			 /* if NAT, mov 0 into ret_value */
			 "(p6) addl %0 = 0, r0;"
			 /* and 1 into mask_exception */
			 "(p6) addl %1 = 1, r0;"
			 : "=r"(ret_value) , "=r"(mask_exception)
			 :"r"(ptr));

    _TR_EXCEPTION_MASKED = mask_exception;

#else

    /* Install the trap signal handler, if necessary */
    if (_EM_INSTALL_TRAP_HANDLER)
      _EM_install_trap_signal_handlers();

    /* Flag that about to execute non-trapping load */
    _TR_MASK_EXCEPTION = 1;

    /* Save pre-trap context.  If load causes trap, 
     * setjmp will return 1 after catching signal.
     */
    if (setjmp (_EM_pretrap_env) == 0)
    {
	/* Perform the load */
	ret_value = *ptr;

	/* If got here, no exceptions were masked.  Flag as such */
	_TR_EXCEPTION_MASKED = 0;
    }
    else
    {
	/* Set ret_value to 0 to emulate zero-filled page mapping support 
	 * for non-trapping loads.
	 */
	ret_value = 0;

	/* If got here, an exception was masked.  Flag as such */
	_TR_EXCEPTION_MASKED = 1;
    }

    /* Flag that handler should punt if it catchs trap in user code */
    _TR_MASK_EXCEPTION = 0;

#endif

    /* Return the value returned by the non-trapping load */
    return (ret_value);

}

/* Emulate a non-trapping unsigned char load in a (hopefully) portable 
 * fashion 
 */
unsigned char _EM_NTload_uchar (unsigned char *ptr)
{
    unsigned char ret_value;
    int mask_exception = 0;
    
    /* To save exception overhead, handle common case of NULL pointer,
     * offsets off of NULL.
     */
    if (((unsigned long)ptr) < ((unsigned long)4096))
    {
	/* If got here, an exception was masked.  Flag as such */
	_TR_EXCEPTION_MASKED = 1;

	/* Return 0 for value */
	return (0);
    }
	
#ifdef _IA64_USE_GENERAL_SPECULATION

    __asm__ __volatile__("ld1.s %0 = [%2];;"
                         /* check for NAT */
                         "tnat.nz p6, p0 = %0;;"
                         /* if NAT, mov 0 into ret_value */
                         "(p6) addl %0 = 0, r0;"
                         /* and 1 into mask_exception */
                         "(p6) addl %1 = 1, r0;"
                         : "=r"(ret_value) , "=r"(mask_exception)
                         :"r"(ptr));

    _TR_EXCEPTION_MASKED = mask_exception;

#else

    /* Install the trap signal handler, if necessary */
    if (_EM_INSTALL_TRAP_HANDLER)
      _EM_install_trap_signal_handlers();

    /* Flag that about to execute non-trapping load */
    _TR_MASK_EXCEPTION = 1;

    /* Save pre-trap context.  If load causes trap, 
     * setjmp will return 1 after catching signal.
     */
    if (setjmp (_EM_pretrap_env) == 0)
    {
	/* Perform the load */
	ret_value = *ptr;

	/* If got here, no exceptions were masked.  Flag as such */
	_TR_EXCEPTION_MASKED = 0;
    }
    else
    {
	/* Set ret_value to 0 to emulate zero-filled page mapping support 
	 * for non-trapping loads.
	 */
	ret_value = 0;

	/* If got here, an exception was masked.  Flag as such */
	_TR_EXCEPTION_MASKED = 1;
    }

    /* Flag that handler should punt if it catchs trap in user code */
    _TR_MASK_EXCEPTION = 0;

#endif

  /* Return the value returned by the non-trapping load */
  return (ret_value);
}

/* Emulate a non-trapping short load in a (hopefully) portable fashion */
short _EM_NTload_short (short *ptr)
{
    short ret_value;
    int mask_exception = 0;
    
    /* To save exception overhead, handle common case of NULL pointer,
     * offsets off of NULL, and unaligned accesses.
     */
    if ((((unsigned long)ptr) < ((unsigned long)4096)) ||
	((((unsigned long)ptr) & 1) != 0))
    {
	/* If got here, an exception was masked.  Flag as such */
	_TR_EXCEPTION_MASKED = 1;

	/* Return 0 for value */
	return (0);
    }

#ifdef _IA64_USE_GENERAL_SPECULATION

    __asm__ __volatile__("ld2.s %0 = [%2];;sxt2 %0 = %0;;"
                         /* check for NAT */
                         "tnat.nz p6, p0 = %0;;" 
                         /* if NAT, mov 0 into ret_value */
                         "(p6) addl %0 = 0, r0;"
                         /* and 1 into mask_exception */
                         "(p6) addl %1 = 1, r0;"
                         : "=r"(ret_value) , "=r"(mask_exception)
                         :"r"(ptr));

    _TR_EXCEPTION_MASKED = mask_exception;

#else

    /* Install the trap signal handler, if necessary */
    if (_EM_INSTALL_TRAP_HANDLER)
      _EM_install_trap_signal_handlers();

    /* Flag that about to execute non-trapping load */
    _TR_MASK_EXCEPTION = 1;

    /* Save pre-trap context.  If load causes trap, 
     * setjmp will return 1 after catching signal.
     */
    if (setjmp (_EM_pretrap_env) == 0)
    {
	/* Perform the load */
	ret_value = *ptr;

	/* If got here, no exceptions were masked.  Flag as such */
	_TR_EXCEPTION_MASKED = 0;
    }
    else
    {
	/* Set ret_value to 0 to emulate zero-filled page mapping support 
	 * for non-trapping loads.
	 */
	ret_value = 0;

	/* If got here, an exception was masked.  Flag as such */
	_TR_EXCEPTION_MASKED = 1;
    }

    /* Flag that handler should punt if it catchs trap in user code */
    _TR_MASK_EXCEPTION = 0;

#endif

    /* Return the value returned by the non-trapping load */
    return (ret_value);
}

/* Emulate a non-trapping unsigned short load in a (hopefully) portable 
 * fashion 
 */
unsigned short _EM_NTload_ushort (unsigned short *ptr)
{
    unsigned short ret_value;
    int mask_exception = 0;

    /* To save exception overhead, handle common case of NULL pointer,
     * offsets off of NULL, and unaligned accesses.
     */
    if ((((unsigned long)ptr) < ((unsigned long)4096)) ||
	((((unsigned long)ptr) & 1) != 0))
    {
	/* If got here, an exception was masked.  Flag as such */
	_TR_EXCEPTION_MASKED = 1;

	/* Return 0 for value */
	return (0);
    }

#ifdef _IA64_USE_GENERAL_SPECULATION

    __asm__ __volatile__("ld2.s %0 = [%2];;"
                         /* check for NAT */
                         "tnat.nz p6, p0 = %0;;" 
                         /* if NAT, mov 0 into ret_value */
                         "(p6) addl %0 = 0, r0;"
                         /* and 1 into mask_exception */
                         "(p6) addl %1 = 1, r0;"
                         : "=r"(ret_value) , "=r"(mask_exception)
                         :"r"(ptr));

    _TR_EXCEPTION_MASKED = mask_exception;

#else

    /* Install the trap signal handler, if necessary */
    if (_EM_INSTALL_TRAP_HANDLER)
      _EM_install_trap_signal_handlers();

    /* Flag that about to execute non-trapping load */
    _TR_MASK_EXCEPTION = 1;

    /* Save pre-trap context.  If load causes trap, 
     * setjmp will return 1 after catching signal.
     */
    if (setjmp (_EM_pretrap_env) == 0)
    {
	/* Perform the load */
	ret_value = *ptr;

	/* If got here, no exceptions were masked.  Flag as such */
	_TR_EXCEPTION_MASKED = 0;
    }
    else
    {
	/* Set ret_value to 0 to emulate zero-filled page mapping support 
	 * for non-trapping loads.
	 */
	ret_value = 0;

	/* If got here, an exception was masked.  Flag as such */
	_TR_EXCEPTION_MASKED = 1;
    }

    /* Flag that handler should punt if it catchs trap in user code */
    _TR_MASK_EXCEPTION = 0;

#endif

    /* Return the value returned by the non-trapping load */
    return (ret_value);
}

/* Emulate a non-trapping int load in a (hopefully) portable fashion */
unsigned int _EM_NTload_uint (unsigned int *ptr)
{
    unsigned int ret_value;
    int mask_exception = 0;
    
    /* To save exception overhead, handle common case of NULL pointer,
     * offsets off of NULL, and unaligned accesses.
     */
    if ((((unsigned long)ptr) < ((unsigned long)4096)) ||
	((((unsigned long)ptr) & 3) != 0))
    {
	/* If got here, an exception was masked.  Flag as such */
	_TR_EXCEPTION_MASKED = 1;

	/* Return 0 for value */
	return (0);
    }

#ifdef _IA64_USE_GENERAL_SPECULATION

    __asm__ __volatile__("ld4.s %0= [%2];;"
                         /* check for NAT */
                         "tnat.nz p6, p0 = %0;;"
                         /* if NAT, mov 0 into ret_value */
                         "(p6) addl %0 = 0, r0;"
                         /* and 1 into mask_exception */
                         "(p6) addl %1 = 1, r0;"
                         : "=r"(ret_value) , "=r"(mask_exception)
                         :"r"(ptr));

    _TR_EXCEPTION_MASKED = mask_exception;

#else

    /* Install the trap signal handler, if necessary */
    if (_EM_INSTALL_TRAP_HANDLER)
      _EM_install_trap_signal_handlers();

    /* Flag that about to execute non-trapping load */
    _TR_MASK_EXCEPTION = 1;

    /* Save pre-trap context.  If load causes trap, 
     * setjmp will return 1 after catching signal.
     */
    if (setjmp (_EM_pretrap_env) == 0)
    {
	/* Perform the load */
	ret_value = *ptr;

	/* If got here, no exceptions were masked.  Flag as such */
	_TR_EXCEPTION_MASKED = 0;
    }
    else
    {
	/* Set ret_value to 0 to emulate zero-filled page mapping support 
	 * for non-trapping loads.
	 */
	ret_value = 0;

	/* If got here, an exception was masked.  Flag as such */
	_TR_EXCEPTION_MASKED = 1;
    }

    /* Flag that handler should punt if it catchs trap in user code */
    _TR_MASK_EXCEPTION = 0;

#endif

    /* Return the value returned by the non-trapping load */
    return (ret_value);
}

/* Emulate a non-trapping int load in a (hopefully) portable fashion */
int _EM_NTload_int (int *ptr)
{
    int ret_value;
    int mask_exception = 0;

    /* To save exception overhead, handle common case of NULL pointer,
     * offsets off of NULL, and unaligned accesses.
     */
    if ((((unsigned long)ptr) < ((unsigned long)4096)) ||
	((((unsigned long)ptr) & 3) != 0))
    {
	/* If got here, an exception was masked.  Flag as such */
	_TR_EXCEPTION_MASKED = 1;

	/* Return 0 for value */
	return (0);
    }

#ifdef _IA64_USE_GENERAL_SPECULATION

    __asm__ __volatile__("ld4.s %0 = [%2];;sxt4 %0 = %0;;"
                         /* check for NAT */
                         "tnat.nz p6, p0 = %0;;"
                         /* if NAT, mov 0 into ret_value */
                         "(p6) addl %0 = 0, r0;"
                         /* and 1 into mask_exception */
                         "(p6) addl %1 = 1, r0;"  
                         : "=r"(ret_value) , "=r"(mask_exception)
                         :"r"(ptr));

    _TR_EXCEPTION_MASKED = mask_exception;

#else

    /* Install the trap signal handler, if necessary */
    if (_EM_INSTALL_TRAP_HANDLER)
      _EM_install_trap_signal_handlers();

    /* Flag that about to execute non-trapping load */
    _TR_MASK_EXCEPTION = 1;

    /* Save pre-trap context.  If load causes trap, 
     * setjmp will return 1 after catching signal.
     */
    if (setjmp (_EM_pretrap_env) == 0)
    {
	/* Perform the load */
	ret_value = *ptr;

	/* If got here, no exceptions were masked.  Flag as such */
	_TR_EXCEPTION_MASKED = 0;
    }
    else
    {
	/* Set ret_value to 0 to emulate zero-filled page mapping support 
	 * for non-trapping loads.
	 */
	ret_value = 0;

	/* If got here, an exception was masked.  Flag as such */
	_TR_EXCEPTION_MASKED = 1;
    }

    /* Flag that handler should punt if it catchs trap in user code */
    _TR_MASK_EXCEPTION = 0;

#endif

    /* Return the value returned by the non-trapping load */
    return (ret_value);
}

/* Emulate a non-trapping long load in a (hopefully) portable fashion */
long _EM_NTload_long (long *ptr)
{
    long ret_value;
    int mask_exception = 0;

    /* To save exception overhead, handle common case of NULL pointer,
     * offsets off of NULL, and unaligned accesses.
     */
    if ((((unsigned long)ptr) < ((unsigned long)4096)) ||
	((((unsigned long)ptr) & 7) != 0))
    {
	/* If got here, an exception was masked.  Flag as such */
	_TR_EXCEPTION_MASKED = 1;

	/* Return 0 for value */
	return (0);
    }

#ifdef _IA64_USE_GENERAL_SPECULATION

    __asm__ __volatile__("ld8.s %0 = [%2];;"
                         /* check for NAT */
                         "tnat.nz p6, p0 = %0;;"
                         /* if NAT, mov 0 into ret_value */
                         "(p6) addl %0 = 0, r0;"
                         /* and 1 into mask_exception */
                         "(p6) addl %1 = 1, r0;"
                         : "=r"(ret_value) , "=r"(mask_exception)
                         :"r"(ptr));

    _TR_EXCEPTION_MASKED = mask_exception;

#else

    /* Install the trap signal handler, if necessary */
    if (_EM_INSTALL_TRAP_HANDLER)
      _EM_install_trap_signal_handlers();

    /* Flag that about to execute non-trapping load */
    _TR_MASK_EXCEPTION = 1;

    /* Save pre-trap context.  If load causes trap, 
     * setjmp will return 1 after catching signal.
     */
    if (setjmp (_EM_pretrap_env) == 0)
    {
	/* Perform the load */
	ret_value = *ptr;

	/* If got here, no exceptions were masked.  Flag as such */
	_TR_EXCEPTION_MASKED = 0;
    }
    else
    {
	/* Set ret_value to 0 to emulate zero-filled page mapping support 
	 * for non-trapping loads.
	 */
	ret_value = 0;

	/* If got here, an exception was masked.  Flag as such */
	_TR_EXCEPTION_MASKED = 1;
    }

    /* Flag that handler should punt if it catchs trap in user code */
    _TR_MASK_EXCEPTION = 0;

#endif

    /* Return the value returned by the non-trapping load */
    return (ret_value);
}

/* Emulate a non-trapping long load in a (hopefully) portable fashion */
long long _EM_NTload_longlong (long long *ptr)
{
    long long ret_value;
    int mask_exception = 0;

    /* To save exception overhead, handle common case of NULL pointer,
     * offsets off of NULL, and unaligned accesses.
     */
    if ((((unsigned long)ptr) < ((unsigned long)4096)) ||
	((((unsigned long)ptr) & 7) != 0))
    {
	/* If got here, an exception was masked.  Flag as such */
	_TR_EXCEPTION_MASKED = 1;

	/* Return 0 for value */
	return (0);
    }

#ifdef _IA64_USE_GENERAL_SPECULATION

    __asm__ __volatile__("ld8.s %0 = [%2];;"
                         /* check for NAT */
                         "tnat.nz p6, p0 = %0;;"
                         /* if NAT, mov 0 into ret_value */
                         "(p6) addl %0 = 0, r0;"
                         /* and 1 into mask_exception */
                         "(p6) addl %1 = 1, r0;"
                         : "=r"(ret_value) , "=r"(mask_exception)
                         :"r"(ptr));

    _TR_EXCEPTION_MASKED = mask_exception;

#else

    /* Install the trap signal handler, if necessary */
    if (_EM_INSTALL_TRAP_HANDLER)
      _EM_install_trap_signal_handlers();

    /* Flag that about to execute non-trapping load */
    _TR_MASK_EXCEPTION = 1;

    /* Save pre-trap context.  If load causes trap, 
     * setjmp will return 1 after catching signal.
     */
    if (setjmp (_EM_pretrap_env) == 0)
    {
	/* Perform the load */
	ret_value = *ptr;

	/* If got here, no exceptions were masked.  Flag as such */
	_TR_EXCEPTION_MASKED = 0;
    }
    else
    {
	/* Set ret_value to 0 to emulate zero-filled page mapping support 
	 * for non-trapping loads.
	 */
	ret_value = 0;

	/* If got here, an exception was masked.  Flag as such */
	_TR_EXCEPTION_MASKED = 1;
    }

    /* Flag that handler should punt if it catchs trap in user code */
    _TR_MASK_EXCEPTION = 0;

#endif

    /* Return the value returned by the non-trapping load */
    return (ret_value);
}

/* Emulate a non-trapping float load in a (hopefully) portable fashion */
float _EM_NTload_float (float *ptr)
{
    float ret_value;
    int mask_exception = 0;

    /* To save exception overhead, handle common case of NULL pointer,
     * offsets off of NULL, and unaligned accesses.
     */
    if ((((unsigned long)ptr) < ((unsigned long)4096)) ||
	((((unsigned long)ptr) & 3) != 0))
    {
	/* If got here, an exception was masked.  Flag as such */
	_TR_EXCEPTION_MASKED = 1;

	/* Return 0 for value */
	return (0);
    }

#ifdef _IA64_USE_GENERAL_SPECULATION

    __asm__ __volatile__("ldfs.s %0 = [%2];;"
                         /* check for NaTVal */
			 "fclass.m.unc p6, p0 = %0, @nat;;"
                         /* if NAT, mov 0 into ret_value */
                         "(p6) mov %0 = f0;"
                         /* and 1 into mask_exception */
                         "(p6) addl %1 = 1, r0;"
                         : "=f"(ret_value) , "=r"(mask_exception)
                         :"r"(ptr));

    _TR_EXCEPTION_MASKED = mask_exception;

#else

    /* Install the trap signal handler, if necessary */
    if (_EM_INSTALL_TRAP_HANDLER)
      _EM_install_trap_signal_handlers();

    /* Flag that about to execute non-trapping load */
    _TR_MASK_EXCEPTION = 1;

    /* Save pre-trap context.  If load causes trap, 
     * setjmp will return 1 after catching signal.
     */
    if (setjmp (_EM_pretrap_env) == 0)
    {
	/* Perform the load */
	ret_value = *ptr;

	/* If got here, no exceptions were masked.  Flag as such */
	_TR_EXCEPTION_MASKED = 0;
    }
    else
    {
	/* Set ret_value to 0 to emulate zero-filled page mapping support 
	 * for non-trapping loads.
	 */
	ret_value = 0.0;

	/* If got here, an exception was masked.  Flag as such */
	_TR_EXCEPTION_MASKED = 1;
    }

    /* Flag that handler should punt if it catchs trap in user code */
    _TR_MASK_EXCEPTION = 0;

#endif

    /* Return the value returned by the non-trapping load */
    return (ret_value);
}

/* Emulate a non-trapping double load in a (hopefully) portable fashion */
double _EM_NTload_double (double *ptr)
{
    double ret_value;
    int mask_exception = 0;

    /* To save exception overhead, handle common case of NULL pointer,
     * offsets off of NULL, and unaligned accesses.
     */
    if ((((unsigned long)ptr) < ((unsigned long)4096)) ||
	((((unsigned long)ptr) & 7) != 0))
    {
	/* If got here, an exception was masked.  Flag as such */
	_TR_EXCEPTION_MASKED = 1;

	/* Return 0 for value */
	return (0);
    }

#ifdef _IA64_USE_GENERAL_SPECULATION

    __asm__ __volatile__("ldfd.s %0 = [%2];;"
                         /* check for NaTVal */
			 "fclass.m.unc p6, p0 = %0, @nat;;"
                         /* if NAT, mov 0 into ret_value */
                         "(p6) mov %0 = f0;"
                         /* and 1 into mask_exception */
                         "(p6) addl %1 = 1, r0;"
                         : "=f"(ret_value) , "=r"(mask_exception)
                         :"r"(ptr));

    _TR_EXCEPTION_MASKED = mask_exception;

#else

    /* Install the trap signal handler, if necessary */
    if (_EM_INSTALL_TRAP_HANDLER)
      _EM_install_trap_signal_handlers();

    /* Flag that about to execute non-trapping load */
    _TR_MASK_EXCEPTION = 1;

    /* Save pre-trap context.  If load causes trap, 
     * setjmp will return 1 after catching signal.
     */
    if (setjmp (_EM_pretrap_env) == 0)
    {
	/* Perform the load */
	ret_value = *ptr;

	/* If got here, no exceptions were masked.  Flag as such */
	_TR_EXCEPTION_MASKED = 0;
    }
    else
    {
	/* Set ret_value to 0 to emulate zero-filled page mapping support 
	 * for non-trapping loads.
	 */
	ret_value = 0.0;

	/* If got here, an exception was masked.  Flag as such */
	_TR_EXCEPTION_MASKED = 1;
    }

    /* Flag that handler should punt if it catchs trap in user code */
    _TR_MASK_EXCEPTION = 0;

#endif

    /* Return the value returned by the non-trapping load */
    return (ret_value);
}
