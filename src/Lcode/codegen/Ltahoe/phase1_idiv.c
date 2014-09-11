/**
***  Copyright (C) 1996 Intel Corporation.
***
*** The information and source code contained herein is the exclusive
*** property of Intel Corporation and may not be disclosed, examined
*** or reproduced in whole or in part without explicit written authorization
*** from the company.
**/

/**
*** Author:  Steve Skedzielewski
**/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <library/i_types.h>

/* Perform 64x32 unsigned divide on a numerator that has bit 63 set
 */

ITuint64
div64_32 (ITuint64 numerator, ITuint32 denominator, int bits_numerator,
	  int bits_denominator)
{

  int comparisons;
  ITuint64 result = 0;
  ITuint64 long_denominator = denominator;

  /* Check for the easy case (which will break the later code) */

  if (denominator > numerator)
    {
      return result;
    };

  /* shift the denominator left for the first subtraction;
   * That distance bounds the number of subtractions we'll do
   */
  comparisons = bits_numerator - bits_denominator + 1;
  long_denominator <<= comparisons - 1;
  while (comparisons)
    {
      result <<= 1;
      if (numerator >= long_denominator)
	{
	  numerator -= long_denominator;
	  result++;
	}
      long_denominator >>= 1;
      comparisons--;
    }
  return result;
}


/* Calculate the reciprocal of an integer to L (log2) digits of precision
 *
 * Ref: Division by Invariant Integers using Multiplication
 *   SIGPLAN PLDI 94, pp61-72 (Choose multiplier is Figure 6.2 on page 67)
 *
 * inputs:
 *      d - divisor (should be odd for best results - preshift to make it odd)
 *   prec - precision (<=32)
 * outputs:
 *  Choose_multiplier - multiplier
 *                 sh - shift
 *                  l - ceiling(log2(d))
 */

static ITuint64
Choose_multiplier (ITuint32 d, ITuint32 prec, int *sh, int *l)
{

  ITuint32 log = 0;
  int shift_post;
  ITuint64 temp;
  ITuint64 m_high, m_low;
  ITuint64 two_N_plus_log;
  ITuint64 two_N_log_prec;
  ITuint64 long_long_d = d;
  const int N = 32;

  /* better way to compute ceiling log? */

  temp = d;
  while (temp)
    {
      log++;
      temp >>= 1;
    }

  /* approximate the multiplier with m_low and m_high */

  two_N_plus_log = ((ITuint64) 1) << (N + log);
  two_N_log_prec = ((ITuint64) 1) << (N + log - prec);

  m_low = div64_32 (two_N_plus_log, long_long_d, N + log, log);
  m_high = div64_32 (two_N_plus_log + two_N_log_prec,
		     long_long_d, N + log, log);

  /* reduce the multiplier when possible */

  shift_post = log;
  while (shift_post && (m_low >> 1 < m_high >> 1))
    {
      shift_post--;
      m_low >>= 1;
      m_high >>= 1;
    }

  *sh = shift_post;
  *l = log;
  return m_high;
}

/* return codes */
#define OUT_OF_RANGE 1
#define STRING_TOO_SHORT 2

#define emit( c ) if(cursor<result_len) result[cursor++]=c ; else  return STRING_TOO_SHORT;
#define binop(op,op1,op2) emit(op);emit(' ');emit(op1);emit(' ');emit(op2);emit(' ')
#define rshiftop(n,op1) emit('>');emit('0'+n);emit(' ');emit(op1);emit(' ')
#define rashiftop(n,op1) emit('A');emit('0'+n);emit(' ');emit(op1);emit(' ')

/* recip_unsigned_idiv
 *   Generate a sequence of operations that divide an integer by a constant
 * Output values:
 *   recip_idiv: 0 if successful, 1 if N==0, 2 if result is too short a string
 *   result: a string that gives the sequence
 *   multiplier: a pointer to an unsigned int
 *        (will be referenced by the sequence)
 * Input values:
 *   N : the constant divisor
 *   result_length: length of the result string
 *   IA32: Take care to generate no additions that will overflow 32 bits
 *
 * Ref: Figure 4.2: Optimized code generation of unsigned q=floor(n/d)...
 *      page 64
 */
int
Recip_unsigned_idiv (char *result, ITuint32 * multiplier,
		     ITuint64 N, int result_len, int IA32)
{

  ITuint64 two_32 = ((ITuint64) 1) << 32;
  ITuint64 ull_mult, abs_n;
  int shift, log2;
  int cursor = 0;
  int preshift;
  int log;
  int orig, high;

  abs_n = N;			/* unsigned world is different! */
  if (N == 0)
    {
      return OUT_OF_RANGE;
    }

  /* look for powers of 2, they are easy in the unsigned world */
  log = 0;
  while (!(abs_n & 1))
    {
      log++;
      abs_n >>= 1;
    }
  if (abs_n == 1)
    {				/* power of 2 */
      if (log)
	{
	  rshiftop (log, '1');
	}
      emit ((char) 0);
      *multiplier = 1;
      return 0;
    }

  /* Use subtraction when bit 31 of N is on
   *    sub t = input, N
   *    shr t = t,32 (-1 if input<N, 0 otherwise)
   *    add t = t,1  ( 0 if input<N, 1 otherwise)
   * A better sequence is only 2 cycles:
   *            cmpge p1,!p2 = input, N
   *       (p1) mov t,1
   *       (p2) mov t,0
   *   but I don't know how to represent predicated operations in il2!)
   */

  if (N & 0x80000000)
    {
      emit ('D');
      emit (' ');
      emit ('1');
      emit (' ');		/* subtract divisor */
      rashiftop (32, '2');
      emit ('I');
      emit (' ');
      emit ('3');
      emit (' ');		/* increment */
      emit ((char) 0);

    }

  ull_mult = Choose_multiplier (N, 32, &shift, &log2);

  /* reduce even multipliers when they are greater than 32 bits */

  preshift = 0;
  if ((ull_mult & two_32) && !(N & 1))
    {
      while (!(N & 1))
	{
	  preshift++;
	  N >>= 1;
	}
      if (N == 1)
	{			/* power of 2 */
	  rshiftop (preshift, '1');
	  emit ((char) 0);
	  *multiplier = 1;
	  return 0;
	}
      if (preshift)
	{
	  ull_mult = Choose_multiplier (N, 32 - preshift, &shift, &log2);
	}
    }

  if (ull_mult & two_32)
    {
      *multiplier = ull_mult - two_32;
      emit ('%');
      emit (' ');
      emit ('1');
      emit (' ');		/* t2 */
      if (IA32)
	{
	  binop ('-', '1', '2');	/* t3 */
	  rshiftop (1, '3');	/* t4 */
	  binop ('+', '2', '4');	/* t5 */
	  rshiftop (shift - 1, '5');
	}
      else
	{			/* IA64 */
	  binop ('+', '1', '2');
	  rshiftop (shift, '3');
	}
    }
  else
    {
      *multiplier = ull_mult;
      if (preshift)
	{
	  rshiftop (preshift, '1');
	  orig = '2';
	}
      else
	{
	  orig = '1';
	}
      if (IA32)
	{
	  emit ('%');
	  emit (' ');
	  emit (orig);
	  emit (' ');
	  high = orig + 1;
	  rshiftop (shift, high);
	}
      else
	{			/* IA64, combine the two shifts */
	  emit ('*');
	  emit (' ');
	  emit (orig);
	  emit (' ');
	  high = orig + 1;
	  rshiftop (shift + 32, high);
	}
    }
  emit ((char) 0);
  return 0;
}

/* recip_signed_idiv
 *   Generate a sequence of operations that divide an integer by a constant
 * Output values:
 *   recip_signed_idiv: 0 if successful, 1 otherwise
 *   result: a string that gives the sequence
 *   multiplier: a pointer to an unsigned int
 *        (will be referenced by the sequence)
 * Input values:
 *   N : the constant divisor
 *   result_length: length of the result string
 *   IA32: Take care to generate no additions that will overflow 32 bits
 *
 * Ref:  Figure 5.2, "Optimized code generation of signed q=TRUNC(n/d)
 *       page 65
 */
int
Recip_signed_idiv (char *result, ITuint32 * multiplier,
		   ITint64 N, int result_len, int IA32)
{

  ITuint64 two_31 = ((ITuint64) 1) << 31;
  ITuint64 two_32 = ((ITuint64) 1) << 32;
  ITuint64 ull_mult;
  int shift, log2;
  int cursor = 0;
  int done = 0;
  int log;
  int last = 0;
  int abs_n;

  if (N == 0)
    {
      return OUT_OF_RANGE;
    }

  /* generate best possible code when divisor is 1, -1, 2, -2 */
  abs_n = abs (N);
  if (abs_n == 1)
    {
      last = '1';
      done = 1;
    }
  if (abs_n == 2)
    {
      if (IA32)
	{
	  rshiftop (31, '1');
	}
      else
	{
	  rshiftop (63, '1');
	}
      binop ('+', '1', '2');
      rashiftop (1, '3');
      last = '4';
      done = 1;
    }
  if (done)
    {
      if (N < 0)
	{
	  emit ('N');
	  emit (' ');
	  emit (last);
	  emit (' ');
	}
      emit ((char) 0);
      *multiplier = 1;
      return 0;
    }

  /* very special case here; only works on IA64 */

  if (abs_n == 0x80000000)
    {
      if (IA32)
	return OUT_OF_RANGE;
      binop ('+', '1', '1');
      rshiftop (32, '2');
      emit ((char) 0);
      *multiplier = 1;
      return 0;
    }

  /* look for powers of 2 other than -2^32 */
  log = 0;
  while (!(abs_n & 1))
    {
      log++;
      abs_n >>= 1;
    }
  if (abs_n == 1)
    {				/* power of 2 */
      rashiftop (log - 1, '1');
      if (IA32)
	{
	  rshiftop (32 - log, '2');
	}
      else
	{
	  rshiftop (64 - log, '2');
	}
      binop ('+', '1', '3');
      rashiftop (log, '4');
      if (N < 0)
	{
	  emit ('N');
	  emit (' ');
	  emit ('5');
	  emit (' ');
	}
      emit ((char) 0);
      *multiplier = 1;
      return 0;
    }

  ull_mult = Choose_multiplier (abs (N), 32 - 1, &shift, &log2);
  if (ull_mult >= two_31)
    {
      *multiplier = ull_mult - two_32;
      emit ('H');
      emit (' ');
      emit ('1');
      emit (' ');		/* t2 */
      binop ('+', '1', '2');	/* t3 */
      rashiftop (shift, '3');	/* t4 */
      if (IA32)
	{
	  rashiftop (31, '1');	/* t5 */
	}
      else
	{
	  rashiftop (63, '1');	/* t5 */
	}
      binop ('-', '4', '5');
      last = 6;
    }
  else
    {
      *multiplier = ull_mult;
      if (IA32)
	{
	  emit ('H');
	  emit (' ');
	  emit ('1');
	  emit (' ');		/* t2 */
	  rashiftop (shift, '2');	/* t3 */
	}
      else
	{
	  emit ('S');
	  emit (' ');
	  emit ('1');
	  emit (' ');		/* t2 */
	  rashiftop (32 + shift, '2');	/* t3 */
	}
      rashiftop (31, '1');	/* t4 */
      binop ('-', '3', '4');
      last = 5;
    }
  if (N < 0)
    {
      emit ('N');
      emit (' ');
      emit ('0' + last);
      emit (' ');
    }
  emit ((char) 0);
  return 0;
}
