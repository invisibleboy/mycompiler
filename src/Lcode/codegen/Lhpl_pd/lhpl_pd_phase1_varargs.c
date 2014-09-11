/*****************************************************************************\
 *
 *  File:  lhpl_pd_phase1_varargs.c
 *
 *  Description:
 *    Handle varargs in accordance with impact software conventions,
 *    Most of the code is just stolen directly from limpact_phase1_varargs.c
 *
 *  Creation Date :  July 2003
 *
 *  Author:  Scott Mahlke
 *
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "lhpl_pd_main.h"
#include "Lcode/l_build_prototype_info.h"

#undef DEBUG_SPILL

/***********************************************************************/
/*                     Varargs Handling Code                           */
/***********************************************************************/

int
Lhpl_pd_is_vararg_func (L_Func * fn)
{
  if (L_find_attr (fn->attr, "VARARG"))
    return 1;
  else
    return 0;
}

static int
Lhpl_pd_mark_as_mem_barrier(L_Oper * op)
{
  /* Since this op is inserted after memory dependence profiling,  *
   * it has no edges. For now, we treat this conservatively as     *
   * a barrier (no memory ops can be moved above or below this     *
   * in Elcor even when Elcor's memvr_profiled=yes).               *
   *                                <lieberm 11/2006>              */

  op->attr = L_concat_attr (op->attr, L_new_attr("dep_mem_barrier", 1));
}

/***********************************************************************/
/*             Floint Point Parameter Passing Code                     */
/***********************************************************************/

/* When calling a varargs function, floating point params must be passed 
   both in floating point registers and on the int parameter regs.  The 
   params must be on the int parameter regs because a vararg function 
   first moves int input registers to memory and then thereafter accesses 
   params from memory.  

   Since it is hard or sometimes impossible to tell if a function is a
   vararg function, the IMPACT convention is to always pass float
   params in both places.  PtoL issues a move to floating point macro
   instruction; this function finds these fp parameter instructions
   and creates a duplicate move to the proper register file location.

   The FP arguments are also passed on the stack.  As a corner case
   arises when the args to a function are (INT, DBL, DBL).  The 4 INT
   parameter passing registers will hold the first INT argument,
   the 2nd argument and the first half of the last argument.  So, in
   addition to passing things in the INT parameter registers, we also
   store the FP arguments to the outgoing parameter space.  In essence,
   we have 3 levels of redundancy.  
 */

static void
Lhpl_pd_insert_fp_param_moves_at_jsr (L_Cb * cb, L_Oper * jsr_oper)
{
  L_Oper *new_oper;
  L_Attr *tr_attr, *tr_aux_attr, *op_attr = NULL, *desc_attr = NULL;
  int num_reg_param;
  int fp_mac, int_mac, i;
  int param_num;
  char return_type_buf[TYPE_BUF_SIZE];
  char all_parm_type_buf[TYPE_BUF_SIZE];
  char parm_type_buf[TYPE_BUF_SIZE];
  char *parse_ptr = all_parm_type_buf;
  int parm_type;
  int found;
  
  if (!(tr_attr = L_find_attr (jsr_oper->attr, "tr")))
    return;

  num_reg_param = tr_attr->max_field;

  // First check if we have any FP args through registers.  If not
  // there is nothing to do.

  found = 0;
  for (i=0; i<num_reg_param; i++) {
    int ctype = tr_attr->field[i]->ctype;
    if (L_is_ctype_double_direct(ctype) || L_is_ctype_float_direct(ctype)) {
      found = 1;
      break;
    }
  }
  if (! found)
    return;

  // Create auxiliary tr (through-register) attribute.   This will
  // hold the extra through register parameters that will be added by this
  // routine.  Do not want to corrupt the tr attribute as trimaran simu
  // needs it to match exactly with the function prototype.
  tr_aux_attr = L_find_attr(jsr_oper->attr, "tr_aux");
  if (tr_aux_attr != NULL)
    L_punt("Lhpl_pd_insert_fp_param_moves_at_jsr: tr_aux already on jsr %d",
		jsr_oper->id);
  tr_aux_attr = L_new_attr("tr_aux", 0);
  
#ifdef DEBUG_SPILL
  printf("Lhpl_pd_insert_fp_param_moves_at_jsr: "
	 "fn %s, cb %d, op %d with %d stack params\n",
	 L_fn->name, cb->id, jsr_oper->id, num_reg_param);
#endif

  L_get_call_info (NULL, jsr_oper, jsr_oper->attr, 
		   return_type_buf, all_parm_type_buf,
		   sizeof (return_type_buf));

  for (i = 0; i < num_reg_param; i++)
    {
      /* This routine will add int parms to the tr list, 
	 but prior to this point, the tr list should match
	 the call_info exactly, with the exception that
	 there could be a P15 return struct by value 
	 element in the tr that should be ignored. 
         If the call_info runs out, there should be no
	 more parameters. MCM 2/7/02 */

      if (tr_attr->field[i]->value.mac == 15)
	continue;
      
      if (*parse_ptr == 0)
	break;

      /* Get the next parameter type */
      L_get_next_param_type (parm_type_buf, &parse_ptr);
      parm_type = L_convert_type_to_ctype (parm_type_buf);

      if (L_is_ctype_double_direct(parm_type))
	{
	  fp_mac = tr_attr->field[i]->value.mac;

	  if (!op_attr)
	    if (!(op_attr = L_find_attr (jsr_oper->attr, "op_info")))
	      L_punt("Lhpl_pd_insert_fp_param_move_at_jsr: "
		     "op_info is required");

	  param_num = op_attr->field[(2 * i)+1]->value.i;
	  // Seems like this should use Mspec rather than being hard coded!
	  int_mac = L_MAC_P0 + param_num;
  
	  /* Use a store and a load or two to move the double registers 
	     into integer parameter registers */
	  
	  /* Create store */
	  new_oper = L_create_new_op(Lop_ST_F2);
	  new_oper->src[0] = L_new_macro_operand (L_MAC_OP, L_CTYPE_INT,
						  L_PTYPE_NULL);
	  new_oper->src[1] = L_new_gen_int_operand (param_num * 4);
	  new_oper->src[2] = L_new_macro_operand (fp_mac,
						  L_CTYPE_DOUBLE, 
						  L_PTYPE_NULL);
	  new_oper->pred[0] = L_copy_operand (jsr_oper->pred[0]);

	  desc_attr = L_new_attr("param_spill",1);
	  L_set_macro_attr_field (desc_attr, 0,
				  fp_mac, L_CTYPE_DOUBLE, L_PTYPE_NULL);
	  new_oper->attr = L_concat_attr (new_oper->attr, desc_attr);
	  Lhpl_pd_mark_as_mem_barrier(new_oper);
	  
	  L_insert_oper_before (cb, jsr_oper, new_oper);
	  
#ifdef DEBUG_SPILL
	  printf("double param st: mac P%d at offset %d\n",
		 fp_mac, (param_num*4));
#endif

	  /* Create load */
	  new_oper = L_create_new_op (Lop_LD_I);

	  new_oper->src[0] = L_new_macro_operand (L_MAC_OP, L_CTYPE_INT,
						  L_PTYPE_NULL);
	  new_oper->src[1] = L_new_gen_int_operand (param_num * 4);
	  new_oper->dest[0] = L_new_macro_operand (int_mac, L_CTYPE_INT, 
						   L_PTYPE_NULL);
	  new_oper->pred[0] = L_copy_operand (jsr_oper->pred[0]);
	  
	  desc_attr = L_new_attr("param_spill",1);
	  L_set_macro_attr_field (desc_attr, 0,
				  fp_mac, L_CTYPE_DOUBLE, L_PTYPE_NULL);
	  new_oper->attr = L_concat_attr (new_oper->attr, desc_attr);
	  Lhpl_pd_mark_as_mem_barrier(new_oper);

	  L_insert_oper_before (cb, jsr_oper, new_oper);
	  
#ifdef DEBUG_SPILL
	  printf("double param ld: mac P%d at offset %d\n",
		 int_mac, (param_num*4));
#endif
	  /*
	   * Add the int mac to the tr to prevent deadcode from 
	   * eating the move
	   */

	  L_set_macro_attr_field (tr_aux_attr, tr_aux_attr->max_field,
				  int_mac, L_CTYPE_INT, L_PTYPE_NULL);
	  
	  /* Create second load (if needed) */
	  if (param_num < 3) /* If we are not on the last
				parameter passed by register */
	    {
	      new_oper = L_create_new_op (Lop_LD_I);
	      
	      new_oper->src[0] = L_new_macro_operand (L_MAC_OP, L_CTYPE_INT,
						      L_PTYPE_NULL);
	      new_oper->src[1] = L_new_gen_int_operand ((param_num+1) * 4);
	      new_oper->dest[0] = L_new_macro_operand (int_mac+1, L_CTYPE_INT, 
						       L_PTYPE_NULL);
	      new_oper->pred[0] = L_copy_operand (jsr_oper->pred[0]);
	      
	      desc_attr = L_new_attr("param_spill",1);
	      L_set_macro_attr_field (desc_attr, 0,
				      fp_mac, L_CTYPE_DOUBLE, L_PTYPE_NULL);
	      new_oper->attr = L_concat_attr (new_oper->attr, desc_attr);
	      Lhpl_pd_mark_as_mem_barrier(new_oper);
	      
	      L_insert_oper_before (cb, jsr_oper, new_oper);
	      
#ifdef DEBUG_SPILL
	      printf("double param ld: mac P%d at offset %d\n",
		     (int_mac+1), ((param_num+1)*4));
#endif
	      /*
	       * Add the int mac to the tr to prevent deadcode from 
	       * eating the move
	       */

	      L_set_macro_attr_field (tr_aux_attr, tr_aux_attr->max_field,
				      int_mac+1, L_CTYPE_INT, L_PTYPE_NULL);
	    }
	}
      else if (L_is_ctype_float_direct(parm_type))
	{
	  fp_mac = tr_attr->field[i]->value.mac;

	  if (!op_attr)
	    if (!(op_attr = L_find_attr (jsr_oper->attr, "op_info")))
	      L_punt("Lhpl_pd_insert_fp_param_move_at_jsr: "
		     "op_info is required");

	  param_num = op_attr->field[(2 * i)+1]->value.i;
	  int_mac = L_MAC_P0 + param_num;
	  
	  /* Use a store and a load or two to move the double registers 
	     into integer parameter registers */
	  
	  /* Create store */
	  new_oper = L_create_new_op(Lop_ST_F);
	  new_oper->src[0] = L_new_macro_operand (L_MAC_OP, L_CTYPE_INT,
						  L_PTYPE_NULL);
	  new_oper->src[1] = L_new_gen_int_operand (param_num * 4);
	  new_oper->src[2] = L_new_macro_operand (fp_mac,
						  L_CTYPE_DOUBLE, 
						  L_PTYPE_NULL);
	  new_oper->pred[0] = L_copy_operand (jsr_oper->pred[0]);
	  
	  desc_attr = L_new_attr("param_spill",1);
	  L_set_macro_attr_field (desc_attr, 0,
				  fp_mac, L_CTYPE_DOUBLE, L_PTYPE_NULL);
	  new_oper->attr = L_concat_attr (new_oper->attr, desc_attr);
	  Lhpl_pd_mark_as_mem_barrier(new_oper);

	  L_insert_oper_before (cb, jsr_oper, new_oper);
	  
#ifdef DEBUG_SPILL
	  printf("float param st: mac P%d at offset %d\n",
		 fp_mac, (param_num*4));
#endif

	  /* Create load */
	  new_oper = L_create_new_op (Lop_LD_I);

	  new_oper->src[0] = L_new_macro_operand (L_MAC_OP, L_CTYPE_INT,
						  L_PTYPE_NULL);
	  new_oper->src[1] = L_new_gen_int_operand (param_num * 4);
	  new_oper->dest[0] = L_new_macro_operand (int_mac, L_CTYPE_INT, 
						   L_PTYPE_NULL);
	  new_oper->pred[0] = L_copy_operand (jsr_oper->pred[0]);
	  
	  desc_attr = L_new_attr("param_spill",1);
	  L_set_macro_attr_field (desc_attr, 0,
				  fp_mac, L_CTYPE_DOUBLE, L_PTYPE_NULL);
	  new_oper->attr = L_concat_attr (new_oper->attr, desc_attr);
	  Lhpl_pd_mark_as_mem_barrier(new_oper);

#ifdef DEBUG_SPILL
	  printf("float param ld: mac P%d at offset %d\n",
		 int_mac, (param_num*4));
#endif
	  L_insert_oper_before (cb, jsr_oper, new_oper);
	  
	  /*
	   * Add the int mac to the tr to prevent deadcode from 
	   * eating the move
	   */

	  L_set_macro_attr_field (tr_aux_attr, tr_aux_attr->max_field,
				  int_mac, L_CTYPE_INT, L_PTYPE_NULL);
	}
    }

    if (tr_aux_attr->max_field > 0)
      jsr_oper->attr = L_concat_attr(jsr_oper->attr, tr_aux_attr);
    else  // Free it just in the rare case it was unnecessary
      L_free_attr(tr_aux_attr);

  return;
}

void
Lhpl_pd_adjust_fp_parameter_passing (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	{
	  if (L_subroutine_call_opcode (oper))
	    Lhpl_pd_insert_fp_param_moves_at_jsr (cb, oper);
	}
    }
  return;
}
