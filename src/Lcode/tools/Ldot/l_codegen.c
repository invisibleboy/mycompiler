/*===========================================================================
 *      File :          l_codegen.c
 *      Description :   Driver for dot graph display tool
 *      Creation Date : January 2007
 *      Author :        Steve Lieberman
 *==========================================================================*/
#include <config.h>
#include <Lcode/l_main.h>
#include "l_dot.h"

static void
process_input (void)
{
  switch (L_token_type)
    {
    case L_INPUT_EOF:
    case L_INPUT_MS:
    case L_INPUT_VOID:
    case L_INPUT_BYTE:
    case L_INPUT_WORD:
    case L_INPUT_LONG:
    case L_INPUT_LONGLONG:
    case L_INPUT_FLOAT:
    case L_INPUT_DOUBLE:
    case L_INPUT_ALIGN:
    case L_INPUT_ASCII:
    case L_INPUT_ASCIZ:
    case L_INPUT_ELEMENT_SIZE:
    case L_INPUT_RESERVE:
    case L_INPUT_GLOBAL:
    case L_INPUT_WB:
    case L_INPUT_WW:
    case L_INPUT_WI:
    case L_INPUT_WQ:
    case L_INPUT_WF:
    case L_INPUT_WF2:
    case L_INPUT_WS:
      /* LCW - new tokens for preserving debugging info - 8/19/97 */
    case L_INPUT_DEF_STRUCT:
    case L_INPUT_DEF_UNION:
    case L_INPUT_DEF_ENUM:
    case L_INPUT_FIELD:
    case L_INPUT_ENUMERATOR:
      L_repair_hashtbl (L_data);
      L_delete_data (L_data);
      break;
    case L_INPUT_FUNCTION:
      L_define_fn_name (L_fn->name);
      Ldot_process (L_fn);
      L_delete_func (L_fn);
      break;
    default:
      L_punt ("process_input: illegal token", L_ERR_INTERNAL);
    }
}


void
L_gen_code (Parm_Macro_List * command_line_macro_list)
{
  L_open_input_file (L_input_file);
  while (L_get_input () != L_INPUT_EOF)
    {
      process_input();
    }

  L_close_input_file (L_input_file);
}
