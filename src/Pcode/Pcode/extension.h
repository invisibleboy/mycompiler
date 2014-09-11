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
/*! \file
 * \brief Functions to manage the Extension field.
 *
 * \author Robert Kidd and Wen-mei Hwu
 */

#ifndef _PCODE_EXTENSION_H_
#define _PCODE_EXTENSION_H_

#include <config.h>
#include "pcode.h"

#define EXT_DATA_TAG "ExtData"

extern int P_ExtSetupM (_ExtStruct struct_type, AllocHandler alloc,
			FreeHandler free);

/*! \brief Registers a copy handler for a module.
 *
 * \param s
 *  the type of struct handled by the copy handler.
 * \param c
 *  a function to perform the copy.
 *
 * \sa P_ExtSetupM(), #P_ExtRegisterReadM(), #P_ExtRegisterWriteM()
 */
#define P_ExtRegisterCopyM(s, c) P_ExtRegisterCopyL ((s), 0, (c))

/*! \brief Registers a read handler for a module.
 *
 * \param s
 *  the type of struct handled by the read handler.
 * \param r
 *  a function to read the extension field from a character string.
 * \param t
 *  the field's signature.
 *
 * \sa P_ExtSetupM(), #P_ExtRegisterCopyM(), #P_ExtRegisterWriteM()
 */
#define P_ExtRegisterReadM(s, r, t) P_ExtRegisterReadL ((s), 0, (r), (t))

/*! \brief Registers a write handler for a module.
 *
 * \param s
 *  the type of struct handled by the write handler.
 * \param w
 *  a function to write a character string from the extension field.
 * \param t
 *  the field's signature.
 *
 * \sa P_ExtSetupM(), #P_ExtRegisterCopyM(), #P_ExtRegisterReadM()
 */
#define P_ExtRegisterWriteM(s, w, t) P_ExtRegisterWriteL ((s), 0, (w), (t))

extern int P_ExtSetupL (_ExtStruct struct_type,	AllocHandler alloc,
			FreeHandler free);
extern void P_ExtRegisterCopyL (_ExtStruct struct_type, int index,
				CopyHandler copy);
extern void P_ExtRegisterReadL (_ExtStruct struct_type, int index,
				ReadHandler read, char *sig);
extern void P_ExtRegisterWriteL (_ExtStruct struct_type, int index,
				 WriteHandler write, char *sig);

/*! \brief Sets a handler option for a library.
 *
 * \param s
 *  the type of struct the handler handles (see ::_ExtStruct).
 * \param i
 *  the library's index in the struct's extension field.
 * \param o
 *  the option to set (see ::_HandlerOptions).
 *
 * \return
 *  The handler's new options.
 *
 * Sets option \a o for the handler of struct \a s.
 *
 * \sa #P_ExtGetOptionL(), #P_ExtClrOptionL(), #P_ExtSetOptionM(),
 * #P_ExtGetOptionM(), #P_ExtClrOptionM() */
#define P_ExtSetOptionL(s, i, o) (Handlers[(s)][(i)].options |= (o))

/*! \brief Gets the handler options for a library.
 *
 * \param s
 *  the type of struct the handler handles (see ::_ExtStruct).
 * \param i
 *  the library's index in the struct's extension field.
 *
 * \return
 *  The handler's options.
 *
 * Returns the handler options for a module.
 *
 * \sa #P_ExtSetOptionL(), #P_ExtClrOptionL(), #P_ExtSetOptionM(),
 * #P_ExtGetOptionM(), #P_ExtClrOptionM() */
#define P_ExtGetOptionL(s, i) (Handlers[(s)][(i)].options)

/*! \brief Clears a handler option for a library.
 *
 * \param s
 *  the type of struct the handler handles (see ::_ExtStruct).
 * \param i
 *  the library's index in the struct's extension field.
 * \param o
 *  the option to clear (see ::_HandlerOptions).
 *
 * \return
 *  The handler's new options.
 *
 * Clears option \a o for the handler of struct \a s.
 *
 * \sa #P_ExtSetOptionL(), #P_ExtGetOptionL(), #P_ExtSetOptionM(),
 * #P_ExtGetOptionM(), #P_ExtClrOptionM() */
#define P_ExtClrOptionL(s, i, o) (Handlers[(s)][(i)].options &= ~(o))

/*! \brief Sets a handler option for a module.
 *
 * \param s
 *  the type of struct the handler handles (see ::_ExtStruct).
 * \param o
 *  the option to set (see ::_HandlerOptions).
 *
 * \return
 *  The handler's new options.
 *
 * Sets option \a o for the handler of struct \a s.
 *
 * \sa #P_ExtSetOptionL(), #P_ExtGetOptionL(), #P_ExtClrOptionL(),
 * #P_ExtGetOptionM(), #P_ExtClrOptionM() */
#define P_ExtSetOptionM(s, o) (P_ExtSetOptionL ((s), 0, (o)))

/*! \brief Gets the handler options for a module.
 *
 * \param s
 *  the type of struct the handler handles (see ::_ExtStruct).
 *
 * \return
 *  The handler's options.
 *
 * Returns the handler options for a module.
 *
 * \sa #P_ExtSetOptionL(), #P_ExtGetOptionL(), #P_ExtClrOptionL(),
 * #P_ExtSetOptionM(), #P_ExtClrOptionM() */
#define P_ExtGetOptionM(s) (P_ExtGetOptionL ((s), 0))

/*! \brief Clears a handler option for a module.
 *
 * \param s
 *  the type of struct the handler handles (see ::_ExtStruct).
 * \param o
 *  the option to clear (see ::_HandlerOptions).
 *
 * \return
 *  The handler's new options.
 *
 * Clears option \a o for the handler of struct \a s.
 *
 * \sa #P_ExtSetOptionL(), #P_ExtGetOptionL(), #P_ExtClrOptionL(),
 * #P_ExtSetOptionM(), #P_ExtGetOptionM() */
#define P_ExtClrOptionM(s, o) (P_ExtClrOptionL ((s), 0, (o)))

extern void P_ExtRead (_ExtStruct struct_type, void *data);
extern void P_ExtWrite (_ExtStruct struct_type, void *data);

extern void P_ExtCleanup ();

#endif

			       
