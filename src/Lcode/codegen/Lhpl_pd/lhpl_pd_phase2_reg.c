/*===========================================================================
 *	File :		lhpl_pd_phase2_reg.c
 *	Description :	Register allocation.
 *	Author : 	Scott A. Mahlke
 *	Date :		August 1993
 *
 *==========================================================================*/
#include "lhpl_pd_main.h"

static int *caller_prd_reg_map;
static int *callee_prd_reg_map;
static int *caller_int_reg_map;
static int *callee_int_reg_map;
static int *caller_flt_reg_map;
static int *callee_flt_reg_map;
static int *caller_dbl_reg_map;
static int *callee_dbl_reg_map; 
static int *caller_btr_reg_map;
static int *callee_btr_reg_map;

int num_flt_callee_reg;

static Set caller_int_set = NULL;
static Set callee_int_set = NULL;
static Set caller_float_set = NULL;
static Set callee_float_set = NULL;
static Set caller_double_set = NULL;
static Set callee_double_set = NULL;
static Set caller_predicate_set = NULL;
static Set callee_predicate_set = NULL;
static Set caller_btr_set = NULL;
static Set callee_btr_set = NULL;

/*
 *      Just check if function hyperblock flag is set correctly
 *      so we can handle old files, can get rid of this eventually.
 */
static void L_check_func_hyperblock_flag(L_Func *fn)
{
    int func_flag, cb_flag;
    L_Cb *cb;

    cb_flag = 0;
    for (cb=fn->first_cb; cb!=NULL; cb=cb->next_cb) {
        if (L_EXTRACT_BIT_VAL(cb->flags, L_CB_HYPERBLOCK)) {
            cb_flag = 1;
            break;
        }
    }

    func_flag = L_EXTRACT_BIT_VAL(fn->flags, L_FUNC_HYPERBLOCK);
    if (cb_flag!=func_flag) {
        fprintf(stderr, "Limpact: WARNING - hyperblock func flag not correct!\n");
        if (cb_flag) {
            fn->flags = L_SET_BIT_FLAG(fn->flags, L_FUNC_HYPERBLOCK);
        }
        else {
            fn->flags = L_CLR_BIT_FLAG(fn->flags, L_FUNC_HYPERBLOCK);
        }
    }
}

/* Hack to fixup predicate attributes for spill code inserted SAM 10-94 */
static void L_fix_vpred_attrs(L_Func *fn)
{
    L_Cb *cb;
    L_Oper *oper, *ptr;
    L_Attr *attr, *new_attr;

    for (cb=fn->first_cb; cb!=NULL; cb=cb->next_cb) {
	if (! L_EXTRACT_BIT_VAL(cb->flags, L_CB_HYPERBLOCK))
	    continue;
	for (oper=cb->first_op; oper!=NULL; oper=oper->next_op) {
	    if (! L_EXTRACT_BIT_VAL(oper->flags, L_OPER_SPILL_CODE))
		continue;
	    if (oper->pred[0]==NULL)
		continue;

	    if (L_store_opcode(oper)) {
		/* search up first */
		for (ptr=oper->prev_op; ptr!=NULL; ptr=ptr->prev_op) {
		    if (L_EXTRACT_BIT_VAL(ptr->flags, L_OPER_SPILL_CODE))
			continue;
		    if (L_same_operand(oper->pred[0], ptr->pred[0]))
			break;
		}
		if (ptr==NULL) {	/* try down */
		    for (ptr=oper->next_op; ptr!=NULL; ptr=ptr->next_op) {
		        if (L_EXTRACT_BIT_VAL(ptr->flags, L_OPER_SPILL_CODE))
			    continue;
		        if (L_same_operand(oper->pred[0], ptr->pred[0]))
			    break;
		    }
		}
		if (ptr==NULL)
		    L_punt("L_fix_vpred_attrs: no match found for op %d", oper->id);

		attr = L_find_attr(ptr->attr, L_VPRED_PRD_ATTR_NAME);
		if (attr==NULL)
		    L_punt("L_fix_vpred_attrs: no vpred attr found");
		new_attr = L_copy_attr_element(attr);
		oper->attr = L_concat_attr(oper->attr, new_attr);
	    }

	    else if (L_load_opcode(oper) || L_int_add_opcode(oper)) {
		/* look down first */
		for (ptr=oper->next_op; ptr!=NULL; ptr=ptr->next_op) {
		    if (L_EXTRACT_BIT_VAL(ptr->flags, L_OPER_SPILL_CODE))
			continue;
		    if (L_same_operand(oper->pred[0], ptr->pred[0]))
			break;
		}
		if (ptr==NULL) {		/* try up */
		    for (ptr=oper->prev_op; ptr!=NULL; ptr=ptr->prev_op) {
		        if (L_EXTRACT_BIT_VAL(ptr->flags, L_OPER_SPILL_CODE))
			    continue;
		        if (L_same_operand(oper->pred[0], ptr->pred[0]))
			    break;
		    }
		}
		if (ptr==NULL)
		    L_punt("L_fix_vpred_attrs: no match found for op %d, oper->id");

		attr = L_find_attr(ptr->attr, L_VPRED_PRD_ATTR_NAME);
		if (attr==NULL)
		    L_punt("L_fix_vpred_attrs: no vpred attr found");
		new_attr = L_copy_attr_element(attr);
		oper->attr = L_concat_attr(oper->attr, new_attr);
	    }

	    else {
		L_punt("L_fix_vpred_attrs: illegal spill oper");
	    }
	}
    }
}

/******************************************************************************\
 *
 * Functions which provide penalties for use of callee and caller save 
 * registers.
 *
\******************************************************************************/

#define LOAD_COST       1.0
#define STORE_COST      1.0

double R_callee_cost(int ctype, int leaf, int callee_allocated)
{
    double      cost;
    int         loads_per_cycle, stores_per_cycle;

    /* currently assumes uniform model */
    loads_per_cycle = Lsched_loads_per_cycle();
    stores_per_cycle = Lsched_stores_per_cycle();

    switch (ctype)
    {
        case L_CTYPE_INT:
        case L_CTYPE_FLOAT:
        case L_CTYPE_DOUBLE:
            cost = LOAD_COST/loads_per_cycle + STORE_COST/stores_per_cycle;
            break;
        case L_CTYPE_PREDICATE:
            cost = 0;
            break;

        default:
            L_punt ("R_callee_cost: invalid ctype of %d", ctype);
    }

    return cost;
}

double R_caller_cost(int ctype, int leaf)
{
    double      cost;
    int         loads_per_cycle, stores_per_cycle;

    if ( leaf ) return 0;

    /* currently assumes uniform model */
    loads_per_cycle = Lsched_loads_per_cycle();
    stores_per_cycle = Lsched_stores_per_cycle();

    switch (ctype)
    {
        case L_CTYPE_INT:
        case L_CTYPE_FLOAT:
        case L_CTYPE_DOUBLE:
            cost = LOAD_COST/loads_per_cycle + STORE_COST/stores_per_cycle;
            break;
        case L_CTYPE_PREDICATE:
            cost = 0;
            break;

        default:
            L_punt ("R_caller_cost: invalid ctype of %d", ctype);
    }

    return cost;
}

double R_spill_load_cost(int ctype)
{
    double      cost;
    int         loads_per_cycle;

    /* currently assumes uniform model */
    loads_per_cycle = Lsched_loads_per_cycle();

    switch (ctype)
    {
        case L_CTYPE_INT:
        case L_CTYPE_FLOAT:
        case L_CTYPE_DOUBLE:
            cost = LOAD_COST;
            break;
        case L_CTYPE_PREDICATE:
            cost = LOAD_COST;
            break;

        default:
            L_punt ("R_spill_load_cost: invalid ctype of %d", ctype);
    }

    return cost;
}

double R_spill_store_cost(int ctype)
{
    double      cost;
    int         stores_per_cycle;

    /* currently assumes uniform model */
    stores_per_cycle = Lsched_stores_per_cycle();

    switch (ctype)
    {
        case L_CTYPE_INT:
        case L_CTYPE_FLOAT:
        case L_CTYPE_DOUBLE:
            cost = STORE_COST;
            break;
        case L_CTYPE_PREDICATE:
            cost = STORE_COST;
            break;

        default:
            L_punt ("R_spill_store_cost: invalid ctype of %d", ctype);
    }

    return cost;
}

struct R_Physical_Bank *O_locate_rot_reg_bank(L_Func *fn, struct R_Reg *vreg)
{
  L_punt("O_locate_rot_reg_bank: Unsupported.");
  return NULL;
}


/****************************************************************\
 *
 * Register allocator callbacks for insertion of spill code
 *
\****************************************************************/

int O_require_callee_save_code(int ctype)
{
    /* always insert callee save code for HP playdoh */
    if (M_playdoh_model==M_PLAYDOH_V1) {
	return (1);
    }
    else {
	L_punt("O_require_callee_save_code: illegal value of M_playdoh_model %d",
		M_playdoh_model);
    }
    return (0);
}

L_Oper* O_address_add(int offset, L_Operand **pred, int type_flag, int operand_ptype)
{
    int i;
    L_Oper *new_oper;
    L_Attr *attr;

    new_oper = L_create_new_op(Lop_ADD);
    new_oper->flags = L_SET_BIT_FLAG(new_oper->flags, L_OPER_SPILL_CODE);
    new_oper->dest[0] = L_new_macro_operand(PLAYDOH_MAC_TEMPREG, L_CTYPE_INT,
						L_PTYPE_NULL);
    new_oper->src[0] = L_new_macro_operand(L_MAC_SP, L_CTYPE_INT, L_PTYPE_NULL);
    new_oper->src[1] = L_new_gen_int_operand(offset);

    /* flag the add as being inserted by the register allocator, so the spill
        offset can be later updated */
    attr = L_new_attr("regalloc1",1);
    L_set_int_attr_field(attr, 0, type_flag);
    new_oper->attr = L_concat_attr(new_oper->attr, attr);

    /* Set the predicates of oper1 and oper2 */
    if (pred!=NULL) {
        for (i=0; i<L_max_pred_operand; i++) {
            new_oper->pred[i] = L_copy_operand(pred[i]);
        }
        if ((operand_ptype==L_PTYPE_UNCOND_T) ||
		(operand_ptype==L_PTYPE_UNCOND_F)) {
            L_delete_operand(new_oper->pred[0]);
            new_oper->pred[0] = NULL;
        }
    }

    return (new_oper);
}

L_Oper* O_fill_reg(int reg, int type, L_Operand *operand, int fill_offset,
			L_Operand **pred, int type_flag)
{
    int op, i, pop;
    L_Oper *new_oper1, *new_oper2;
    L_Attr *attr;
    
    switch (L_operand_case_ctype(operand)) {
        case L_CTYPE_INT:
	case L_CTYPE_BTR:
	    op = Lop_LD_I;
	    pop = PLAYDOHop_L_W_C1_C1;
    	    break;
        case L_CTYPE_FLOAT:
	    op = Lop_LD_F;
	    pop = PLAYDOHop_FL_S_C1_C1;
	    break;
        case L_CTYPE_DOUBLE:
            op = Lop_LD_F2;
	    pop = PLAYDOHop_FL_D_C1_C1;
	    break;
	case L_CTYPE_PREDICATE:
#if 0
	    op = Lop_PRED_LD;
	    break;
#endif
        default :
    	    L_punt("O_fill_reg: unsupported register type %d", L_return_old_ctype(operand));
    }
   
    /* Generate spill address */
    new_oper1 = O_address_add(fill_offset, pred, type_flag, operand->ptype);

    /* Generate the load op */
    new_oper2 = L_create_new_op(op);
    new_oper2->flags = L_SET_BIT_FLAG(new_oper2->flags,
					L_OPER_SPILL_CODE|L_OPER_SAFE_PEI);
    new_oper2->proc_opc = pop;
    new_oper2->src[0] = L_copy_operand(new_oper1->dest[0]);
    new_oper2->src[1] = L_new_gen_int_operand(0);

    if (L_is_reg_direct(type))
        new_oper2->dest[0] = L_new_register_operand(reg,L_return_old_ctype(operand),L_PTYPE_NULL);
    else
        new_oper2->dest[0] = L_new_macro_operand(reg,L_return_old_ctype(operand),L_PTYPE_NULL);
    
    /* Add the spill offset attribute to assist memory disambiguation */
    attr = L_new_attr("offset", 1);
    L_set_int_attr_field(attr, 0, fill_offset);
    new_oper2->attr = L_concat_attr(new_oper2->attr,attr);

    /* Set the predicates of oper1 and oper2 */
    if (pred!=NULL) {
        for (i=0; i<L_max_pred_operand; i++) {
            new_oper2->pred[i] = L_copy_operand(pred[i]);
        }
        if ((operand->ptype==L_PTYPE_UNCOND_T) ||
		(operand->ptype==L_PTYPE_UNCOND_F)) {
            L_delete_operand(new_oper2->pred[0]);
            new_oper2->pred[0] = NULL;
        }
    }

    new_oper1->next_op = new_oper2;
    
    return(new_oper1);
}


L_Oper* O_spill_reg(int reg, int type, L_Operand *operand, int spill_offset,
			L_Operand **pred, int type_flag)
{
    int op, i, pop;
    L_Oper *new_oper1, *new_oper2;
    L_Attr *attr;
    
    switch (L_operand_case_ctype(operand)) {
        case L_CTYPE_INT:
	case L_CTYPE_BTR:
	    op = Lop_ST_I;
	    pop = PLAYDOHop_S_W_C1;
    	    break;
        case L_CTYPE_FLOAT :
	    op = Lop_ST_F;
	    pop = PLAYDOHop_FS_S_C1;
	    break;
        case L_CTYPE_DOUBLE :
            op = Lop_ST_F2;
	    pop = PLAYDOHop_FS_D_C1;
	    break;
	case L_CTYPE_PREDICATE:
#if 0
	    op = Lop_PRED_ST;
	    break;
#endif
        default :
    	    L_punt("O_spill_reg: unsupported register type", L_return_old_ctype(operand));
    }

    /* Generate spill address */
    new_oper1 = O_address_add(spill_offset, pred, type_flag, operand->ptype);

    /* Generate the store op */
    new_oper2 = L_create_new_op(op);
    new_oper2->flags = L_SET_BIT_FLAG(new_oper2->flags,
					L_OPER_SPILL_CODE|L_OPER_SAFE_PEI);
    new_oper2->proc_opc = pop;
    new_oper2->src[0] = L_copy_operand(new_oper1->dest[0]);
    new_oper2->src[1] = L_new_gen_int_operand(0);
    if (L_is_reg_direct(type))
	new_oper2->src[2] = L_new_register_operand(reg,L_return_old_ctype(operand),L_PTYPE_NULL);
    else 
        new_oper2->src[2] = L_new_macro_operand(reg,L_return_old_ctype(operand),0);
   
    /* Add the spill offset attribute to assist memory disambiguation */
    attr = L_new_attr("offset", 1);
    L_set_int_attr_field(attr, 0, spill_offset);
    new_oper2->attr = L_concat_attr(new_oper2->attr,attr);

    if ( pred != NULL ) {
        for (i=0; i<L_max_pred_operand; i++) {
            new_oper2->pred[i] = L_copy_operand(pred[i]);
        }
        if ((operand->ptype==L_PTYPE_UNCOND_T) ||
		(operand->ptype==L_PTYPE_UNCOND_F)) {
            L_delete_operand(new_oper2->pred[0]);
            new_oper2->pred[0] = NULL;
        }
    } 
    
    new_oper1->next_op = new_oper2;
	
    return(new_oper1);
}


L_Oper *O_jump_oper(int opc, L_Cb *dest_cb)
{

    L_Oper *new_oper = L_create_new_op(opc);
    new_oper->src[0] = L_new_cb_operand(dest_cb); 

    return(new_oper);
}

static L_Oper *new_load_operation(int index, int type, int offset, int type_flag)
{
    L_Oper *oper;
    L_Attr *attr;

    switch (type) {
    case L_CTYPE_BTR:
	oper = L_create_new_op(Lop_LD_I);
	oper->proc_opc = PLAYDOHop_L_W_C1_C1;
	oper->dest[0] = L_new_register_operand(index, L_CTYPE_BTR, L_PTYPE_NULL);
	oper->src[0] = L_new_macro_operand(PLAYDOH_MAC_TEMPREG, L_CTYPE_INT,
						L_PTYPE_NULL);
	oper->src[1] = L_new_gen_int_operand(0);
	break;
    case L_CTYPE_INT:
	oper = L_create_new_op(Lop_LD_I);
	oper->proc_opc = PLAYDOHop_L_W_C1_C1;
	oper->dest[0] = L_new_register_operand(index, L_CTYPE_INT, L_PTYPE_NULL);
	oper->src[0] = L_new_macro_operand(PLAYDOH_MAC_TEMPREG, L_CTYPE_INT,
						L_PTYPE_NULL);
	oper->src[1] = L_new_gen_int_operand(0);
	break;
    case L_CTYPE_FLOAT:
    case L_CTYPE_DOUBLE:
	oper = L_create_new_op(Lop_LD_F2);
	oper->proc_opc = PLAYDOHop_FL_D_C1_C1;
	oper->dest[0] = L_new_register_operand(index, L_CTYPE_DOUBLE, L_PTYPE_NULL);
	oper->src[0] = L_new_macro_operand(PLAYDOH_MAC_TEMPREG, L_CTYPE_INT,
						L_PTYPE_NULL);
	oper->src[1] = L_new_gen_int_operand(0);
	break;
    case L_CTYPE_PREDICATE:
#if 0
	oper = L_create_new_op(Lop_PRED_LD);
	oper->dest[0] = L_new_register_operand(index, L_CTYPE_PREDICATE,
						L_PTYPE_NULL);
	oper->src[0] = L_new_macro_operand(PLAYDOH_MAC_TEMPREG, L_CTYPE_INT,
						L_PTYPE_NULL);
	oper->src[1] = L_new_gen_int_operand(0);
	break;
#endif
    default:
	L_punt("new_load_operation: illegal ctype %d", type);
    }

    oper->flags = L_SET_BIT_FLAG(oper->flags, L_OPER_SPILL_CODE|L_OPER_SAFE_PEI);

    /* Add the spill code offset attribute */
    attr = L_new_attr("offset", 1);
    L_set_int_attr_field(attr, 0, offset);
    oper->attr = L_concat_attr(oper->attr, attr);

    return oper;
}

static L_Oper *new_store_operation(int index, int type, int offset, int type_flag)
{
    L_Oper *oper;
    L_Attr *attr;

    switch (type) {
    case L_CTYPE_BTR:
	oper = L_create_new_op(Lop_ST_I);
	oper->proc_opc = PLAYDOHop_S_W_C1;
	oper->src[0] = L_new_macro_operand(PLAYDOH_MAC_TEMPREG, L_CTYPE_INT,
						L_PTYPE_NULL);
	oper->src[1] = L_new_gen_int_operand(0);
	oper->src[2] = L_new_register_operand(index, L_CTYPE_BTR, L_PTYPE_NULL);
	break;
    case L_CTYPE_INT:
	oper = L_create_new_op(Lop_ST_I);
	oper->proc_opc = PLAYDOHop_S_W_C1;
	oper->src[0] = L_new_macro_operand(PLAYDOH_MAC_TEMPREG, L_CTYPE_INT,
						L_PTYPE_NULL);
	oper->src[1] = L_new_gen_int_operand(0);
	oper->src[2] = L_new_register_operand(index, L_CTYPE_INT, L_PTYPE_NULL);
	break;
    case L_CTYPE_FLOAT:
    case L_CTYPE_DOUBLE:
	oper = L_create_new_op(Lop_ST_F2);
	oper->proc_opc = PLAYDOHop_FS_D_C1;
	oper->src[0] = L_new_macro_operand(PLAYDOH_MAC_TEMPREG, L_CTYPE_INT,
						L_PTYPE_NULL);
	oper->src[1] = L_new_gen_int_operand(0);
	oper->src[2] = L_new_register_operand(index, L_CTYPE_DOUBLE, L_PTYPE_NULL);
	break;
    case L_CTYPE_PREDICATE:
#if 0
	oper = L_create_new_op(Lop_PRED_ST);
	oper->src[0] = L_new_macro_operand(PLAYDOH_MAC_TEMPREG, L_CTYPE_INT,
						L_PTYPE_NULL);
	oper->src[1] = L_new_gen_int_operand(0);
	oper->src[2] = L_new_register_operand(index, L_CTYPE_PREDICATE,
						L_PTYPE_NULL);
	break;
#endif
    default:
	L_punt("new_store_operation: illegal ctype %d", type);
    }

    oper->flags = L_SET_BIT_FLAG(oper->flags, L_OPER_SPILL_CODE|L_OPER_SAFE_PEI);

    /* Add the spill code offset attribute */
    attr = L_new_attr("offset", 1);
    L_set_int_attr_field(attr, 0, offset);
    oper->attr = L_concat_attr(oper->attr, attr);

    return oper;
}

static L_Oper *new_blk_load_operation(Set restore_set, int offset, int type_flag)
{
    int size, *buf;
    L_Oper *oper;
    L_Attr *attr;

    oper = L_create_new_op(Lop_PRED_LD_BLK);
    oper->dest[0] = L_new_macro_operand(L_MAC_PRED_ALL, L_CTYPE_PREDICATE,
                                                        L_PTYPE_NULL);
    oper->src[0] = L_new_macro_operand(PLAYDOH_MAC_TEMPREG, L_CTYPE_INT,
						L_PTYPE_NULL);
    oper->src[1] = L_new_gen_int_operand(0);

    oper->flags = L_SET_BIT_FLAG(oper->flags, L_OPER_SPILL_CODE|L_OPER_SAFE_PEI);

    /* Add the spill code offset attribute */
    attr = L_new_attr("offset", 1);
    L_set_int_attr_field(attr, 0, offset);
    oper->attr = L_concat_attr(oper->attr, attr);

    /* Mark the first and last register to be saved by this op for the emulator */
    size = Set_size(restore_set);
    if (size<=0)
	L_punt("new_blk_load_operation: no predicates to restore");
    buf = (int *) Lcode_malloc(sizeof(int)*size);
    Set_2array(restore_set, buf);
    attr = L_new_attr("pred_refs", 3);
    L_set_int_attr_field(attr, 0, buf[0]);
    L_set_int_attr_field(attr, 1, buf[size-1]);
    L_set_int_attr_field(attr, 2, size);
    oper->attr = L_concat_attr(oper->attr, attr);
    Lcode_free(buf);

    return (oper);
}

static L_Oper *new_blk_store_operation(Set save_set, int offset, int type_flag)
{
    int size, *buf;
    L_Oper *oper;
    L_Attr *attr;

    oper = L_create_new_op(Lop_PRED_ST_BLK);
    oper->src[0] = L_new_macro_operand(PLAYDOH_MAC_TEMPREG, L_CTYPE_INT,
							L_PTYPE_NULL);
    oper->src[1] = L_new_gen_int_operand(0);
    oper->src[2] = L_new_macro_operand(L_MAC_PRED_ALL, L_CTYPE_PREDICATE,
							L_PTYPE_NULL);

    oper->flags = L_SET_BIT_FLAG(oper->flags, L_OPER_SPILL_CODE|L_OPER_SAFE_PEI);

    /* Add the spill code offset attribute */
    attr = L_new_attr("offset", 1);
    L_set_int_attr_field(attr, 0, offset);
    oper->attr = L_concat_attr(oper->attr, attr);

    /* Mark the first and last register to be saved by this op for the emulator */
    size = Set_size(save_set);
    if (size<=0)
	L_punt("new_blk_load_operation: no predicates to save");
    buf = (int *) Lcode_malloc(sizeof(int)*size);
    Set_2array(save_set, buf);
    attr = L_new_attr("pred_refs", 3);
    L_set_int_attr_field(attr, 0, buf[0]);
    L_set_int_attr_field(attr, 1, buf[size-1]);
    L_set_int_attr_field(attr, 2, size);
    oper->attr = L_concat_attr(oper->attr, attr);
    Lcode_free(buf);

    return(oper);
}

void O_register_init(void)
{
    int i, base, num_prd_caller_reg, num_prd_callee_reg, num_int_caller_reg,
		num_int_callee_reg, num_flt_caller_reg,
		num_dbl_caller_reg, num_dbl_callee_reg,
		num_btr_caller_reg, num_btr_callee_reg;
    static init = 0;
    if (init) return;
    init = 1;

    /*
     *	Define register banks.
     */
    if (Lplaydoh_num_int_reg&0x1)
	L_punt("init_register: Lplaydoh requires even number of integer regs");
    if (Lplaydoh_num_flt_reg&0x1)
	L_punt("init_register: Lplaydoh requires even number of float regs");
    if (Lplaydoh_num_dbl_reg&0x1)
	L_punt("init_register: Lplaydoh requires even number of double regs");
    if ((Lplaydoh_num_dbl_reg*2)!=Lplaydoh_num_flt_reg)
	L_punt("init_register: Lplaydoh requires float = 2x double regs ");

    num_int_caller_reg = Lplaydoh_num_int_reg/2;
    num_int_callee_reg = Lplaydoh_num_int_reg - num_int_caller_reg;

    num_flt_caller_reg = Lplaydoh_num_flt_reg/2;
    num_flt_callee_reg = Lplaydoh_num_flt_reg - num_flt_caller_reg;

    num_dbl_caller_reg = Lplaydoh_num_dbl_reg/2;
    num_dbl_callee_reg = Lplaydoh_num_dbl_reg - num_dbl_caller_reg;

    num_prd_caller_reg = Lplaydoh_num_prd_caller_reg;
    num_prd_callee_reg = Lplaydoh_num_prd_callee_reg;
    
    num_btr_caller_reg = Lplaydoh_num_btr_caller_reg;
    num_btr_callee_reg = Lplaydoh_num_btr_callee_reg;

    base = 0;

    caller_int_reg_map = MALLOC(int,num_int_caller_reg);
    for ( i = 0; i < num_int_caller_reg; i++ ) {  
	caller_int_reg_map[i] = i + 1; 
    }
    R_define_physical_bank(	R_CALLER,		/* bank saving conv */
				R_INT,			/* bank data type   */
				num_int_caller_reg,	/* num registers    */
				1,			/* register size    */
				R_OVERLAP_INT,		/* banks that overlap */
				caller_int_reg_map,	/* register map ptr */
				&caller_int_set);	/* set of caller int
							   registers used */

    base += num_int_caller_reg;

    callee_int_reg_map = MALLOC(int,num_int_callee_reg);
    for ( i = 0; i < num_int_callee_reg; i++ )  {
        callee_int_reg_map[i] = i + base + 1;
    }
    R_define_physical_bank(	R_CALLEE,
				R_INT,
				num_int_callee_reg,
				1,
				R_OVERLAP_INT,
			   	callee_int_reg_map,
				&callee_int_set);

    base += num_int_callee_reg;

    caller_flt_reg_map = MALLOC(int,num_flt_caller_reg);
    caller_dbl_reg_map = MALLOC(int,num_flt_caller_reg);
    for ( i = 0; i < num_flt_caller_reg; i++ )  {
	caller_flt_reg_map[i] = i + base + 1; 
    }
    for ( i = 0; i < num_flt_caller_reg; i++ )  {
	caller_dbl_reg_map[i] = i + base + 1;
    }
    R_define_physical_bank(	R_CALLER,
				R_FLOAT,
				num_flt_caller_reg,
				1,
				R_OVERLAP_FLOAT | R_OVERLAP_DOUBLE, 
				caller_flt_reg_map,
				&caller_float_set);

    R_define_physical_bank(	R_CALLER,
				R_DOUBLE,
				num_dbl_caller_reg,
				2,
				R_OVERLAP_FLOAT | R_OVERLAP_DOUBLE, 
				caller_dbl_reg_map,
				&caller_double_set);

    base += (num_flt_caller_reg);

    callee_flt_reg_map = MALLOC(int,num_flt_callee_reg);
    callee_dbl_reg_map = MALLOC(int,num_flt_callee_reg);
    for ( i = 0; i < num_flt_callee_reg; i++ )  {
	callee_flt_reg_map[i] = i + base + 1;
    }
    for ( i = 0; i < num_flt_callee_reg; i++ )  {
	callee_dbl_reg_map[i] = i + base + 1;
    }
    R_define_physical_bank(	R_CALLEE,
				R_FLOAT,
			        num_flt_callee_reg,
				1,
				R_OVERLAP_FLOAT | R_OVERLAP_DOUBLE, 
				callee_flt_reg_map,
				&callee_float_set);
    
    R_define_physical_bank(	R_CALLEE,
				R_DOUBLE,
				num_dbl_callee_reg,
				2,
				R_OVERLAP_FLOAT | R_OVERLAP_DOUBLE, 
				callee_dbl_reg_map,
				&callee_double_set);

    base += (num_flt_callee_reg);

    caller_prd_reg_map = MALLOC(int,num_prd_caller_reg);
    for ( i = 0; i < num_prd_caller_reg; i++ )  {
	caller_prd_reg_map[i] = i + base + 1;
    }
    R_define_physical_bank(	R_CALLER,
				R_PREDICATE,
				num_prd_caller_reg,
				1,
				R_OVERLAP_PREDICATE,
				caller_prd_reg_map,
				&caller_predicate_set);

    base += num_prd_caller_reg;

    callee_prd_reg_map = MALLOC(int,num_prd_callee_reg);
    for ( i = 0; i < num_prd_callee_reg; i++ )  {
	callee_prd_reg_map[i] = i + base + 1;
    }
    R_define_physical_bank(	R_CALLEE,
				R_PREDICATE,
				num_prd_callee_reg,
				1,
				R_OVERLAP_PREDICATE,
				callee_prd_reg_map,
				&callee_predicate_set);

    base += num_prd_callee_reg;

    caller_btr_reg_map = MALLOC(int,num_btr_caller_reg);
    for ( i = 0; i < num_btr_caller_reg; i++ )  {
	caller_btr_reg_map[i] = i + base + 1;
    }
    R_define_physical_bank(	R_CALLER,
				R_BTR,
				num_btr_caller_reg,
				1,
				R_OVERLAP_BTR,
				caller_btr_reg_map,
				&caller_btr_set);

    base += num_btr_caller_reg;
    
    callee_btr_reg_map = MALLOC(int,num_btr_callee_reg);
    for ( i = 0; i < num_btr_callee_reg; i++ )  {
	callee_btr_reg_map[i] = i + base + 1;
    }
    R_define_physical_bank(	R_CALLEE,
				R_BTR,
				num_btr_callee_reg,
				1,
				R_OVERLAP_BTR,
				callee_btr_reg_map,
				&callee_btr_set);

    base += num_btr_callee_reg;

}

void O_register_allocation(L_Func *fn, Parm_Macro_List *command_line_macro_list)
{
    L_Oper *oper;
    L_Cb *cb;
    int i, spill_space, size_int, size_flt, size_btr;
    Set fcle, icle, bcle;

    int *callee_reg_array,n_callee;

    L_check_func_hyperblock_flag(fn);

    /* Reset the register usage sets */
    caller_int_set = Set_dispose(caller_int_set);
    callee_int_set = Set_dispose(callee_int_set);
    caller_float_set = Set_dispose(caller_float_set);
    callee_float_set = Set_dispose(callee_float_set);
    caller_double_set = Set_dispose(caller_double_set);
    callee_double_set = Set_dispose(callee_double_set);
    caller_predicate_set = Set_dispose(caller_predicate_set);
    callee_predicate_set = Set_dispose(callee_predicate_set);
    caller_btr_set = Set_dispose(caller_btr_set);
    callee_btr_set = Set_dispose(callee_btr_set);
    
    spill_space = R_register_allocation(fn, command_line_macro_list);
    
    /*  IN ORDER TO BE ABLE TO PERFORM REGISTER ALLOCATION TWICE  */
    /*  THE ONLY ADDITIONAL WORK WE ARE ALLOWED TO DO HERE IS THE */
    /*  INSERTION OF THE "CALLEE" SAVED REGISTERS.                */ 
    /*  THUS THE FINAL SWAP SPACE WILL BE THE VALUE RETURNED BY   */
    /*  REGISTER ALLOCATION PLUS THE SPACE REQUIRED FOR CALLEE SVS*/

    fcle = icle = bcle = 0;
	
    /* Place Callee-saved integer registers used into the icle set */
    callee_reg_array = MALLOC(int,Set_size(callee_int_set)+
			      	  Set_size(callee_float_set)+
			          Set_size(callee_double_set)+
				  Set_size(callee_btr_set));
    n_callee = Set_2array(callee_int_set,callee_reg_array);
    for ( i = 0; i < n_callee; i++ )  {
	icle = Set_add(icle,callee_reg_array[i]);
    }

   
    /* Place the double register corresponding to Callee-saved */
    /* float register used in the fcle set		       */
    n_callee = Set_2array(callee_float_set,callee_reg_array);
    for ( i = 0; i < n_callee; i++ )  {
        int j, float_reg;
        if (callee_reg_array[i]&0x1)   {
            float_reg = callee_reg_array[i];
        }
        else  {
            float_reg = callee_reg_array[i] - 1;
        }

        /* Find the corresponding double name in the callee_dbl_reg_map */
        for ( j = 0; j < num_flt_callee_reg ; j++ )
            if ( callee_flt_reg_map[j] == float_reg )
                break;
        if ( j == num_flt_callee_reg )
            L_punt("O_register_allocation: unable to find float reg %d\n",
                   float_reg);

        /* By dividing the index into the float map by 2, we find */
        /* the proper double register.                            */
        fcle = Set_add(fcle, callee_dbl_reg_map[ j >> 1 ]);
    }

    /* Place the Callee-saved double registers used into the fcle set */
    n_callee = Set_2array(callee_double_set,callee_reg_array);
    for ( i = 0; i < n_callee; i++ )  {
        fcle = Set_add(fcle,callee_reg_array[i]);
    }
    
    /* Place the callee-saved btr registers used into the bcle set */
    n_callee = Set_2array(callee_btr_set,callee_reg_array);
    for ( i = 0; i < n_callee; i++ )  {
	bcle = Set_add(bcle,callee_reg_array[i]);
    }
    
    size_int = Set_size(icle);
    size_flt = Set_size(fcle);
    size_btr = Set_size(bcle);
    spill_space += size_int*4 + size_flt*8 + size_btr*4;
    Set_2array(fcle, callee_reg_array);
    Set_2array(icle, callee_reg_array+size_flt);
    Set_2array(bcle, callee_reg_array+size_flt+size_int);

    for (cb=fn->first_cb; cb!=NULL; cb=cb->next_cb) {
	L_Oper *next_oper, *new_oper1, *new_oper2;
	for (oper=cb->first_op; oper!=NULL; oper=next_oper) {
	    int opc, k, swap_offset;
	    next_oper = oper->next_op;	/* this is important */
	    opc = oper->opc;
	    /*
	     *	Additional save/restore.
	     */
	    switch (opc) {

	    case Lop_PROLOGUE:
		/*
		 *  Insert save code after it.
		 */
		swap_offset = spill_space;
		swap_offset += (8-(swap_offset%8));

	        if (O_require_callee_save_code(L_CTYPE_DOUBLE)) {
	            for ( k = 0; k < size_flt; k++ )  {
		        new_oper1 = O_address_add(swap_offset, NULL,
						R_CALLEE_SAVE_CODE, L_PTYPE_NULL);
		        new_oper2 = new_store_operation(callee_reg_array[k],
			  		    L_CTYPE_DOUBLE, swap_offset,
					    R_CALLEE_SAVE_CODE);
		        L_insert_oper_after(cb,oper,new_oper2);
		        L_insert_oper_after(cb,oper,new_oper1);
		        swap_offset += 8;
		    }
		}
	        if (O_require_callee_save_code(L_CTYPE_INT)) {
		    for ( k = size_flt; k < (size_flt+size_int); k++ )  {
		        new_oper1 = O_address_add(swap_offset, NULL,
						R_CALLEE_SAVE_CODE, L_PTYPE_NULL);
                        new_oper2 = new_store_operation(callee_reg_array[k],
				       L_CTYPE_INT, swap_offset, R_CALLEE_SAVE_CODE);
                        L_insert_oper_after(cb,oper,new_oper2);
                        L_insert_oper_after(cb,oper,new_oper1);
                        swap_offset += 4;
		    }
		}
	        if (O_require_callee_save_code(L_CTYPE_BTR)) {
		    for (k=(size_flt+size_int);k<(size_flt+size_int+size_btr);k++) {
		        new_oper1 = O_address_add(swap_offset, NULL,
						R_CALLEE_SAVE_CODE, L_PTYPE_NULL);
                        new_oper2 = new_store_operation(callee_reg_array[k],
				       L_CTYPE_BTR, swap_offset, R_CALLEE_SAVE_CODE);
                        L_insert_oper_after(cb,oper,new_oper2);
                        L_insert_oper_after(cb,oper,new_oper1);
                        swap_offset += 4;
		    }
		}

		if (O_require_callee_save_code(L_CTYPE_PREDICATE)) {
		    /* insert a PRED_ST_BLK if there are any callee save pred regs */
		    if (Set_size(callee_predicate_set) > 0) {
		        new_oper1 = O_address_add(swap_offset, NULL,
						   R_CALLEE_SAVE_CODE, L_PTYPE_NULL);
		        new_oper2 = new_blk_store_operation(callee_predicate_set,
						swap_offset, R_CALLEE_SAVE_CODE);
		        L_insert_oper_after(cb, oper, new_oper2);
		        L_insert_oper_after(cb, oper, new_oper1);
		        swap_offset += ((Lplaydoh_num_prd_callee_reg+Lplaydoh_num_prd_caller_reg+31)/32)*4;
		    }
		}

		new_oper1 = L_create_new_op(Lop_DEFINE);
		new_oper1->dest[0] = L_new_macro_operand(L_MAC_SWAP_SIZE,
							L_CTYPE_INT, L_PTYPE_NULL);
		new_oper1->src[0] = L_new_gen_int_operand(swap_offset);
		L_insert_oper_before(cb,oper,new_oper1);

		break;

   	    case Lop_EPILOGUE:
		/*
		 *  Insert restore code before it.
		 */
		swap_offset = spill_space;
		swap_offset += (8-(swap_offset%8));

	        if (O_require_callee_save_code(L_CTYPE_DOUBLE)) {
                    for ( k = 0; k < size_flt; k++ )  {
		        new_oper1 = O_address_add(swap_offset, NULL,
						R_CALLEE_SAVE_CODE, L_PTYPE_NULL);
                        L_insert_oper_before(cb,oper,new_oper1);
                        new_oper2 = new_load_operation(callee_reg_array[k],
				    L_CTYPE_DOUBLE, swap_offset, R_CALLEE_SAVE_CODE);
                        L_insert_oper_before(cb,oper,new_oper2);
                        swap_offset += 8;
                    }
		}
	        if (O_require_callee_save_code(L_CTYPE_INT)) {
                    for ( k = size_flt; k < (size_flt+size_int); k++ )  {
		        new_oper1 = O_address_add(swap_offset, NULL,
					R_CALLEE_SAVE_CODE, L_PTYPE_NULL);
                        L_insert_oper_before(cb,oper,new_oper1);
                        new_oper2 = new_load_operation(callee_reg_array[k],
				    L_CTYPE_INT, swap_offset, R_CALLEE_SAVE_CODE);
                        L_insert_oper_before(cb,oper,new_oper2);
                        swap_offset += 4;
		    }	
		}
	        if (O_require_callee_save_code(L_CTYPE_BTR)) {
                    for (k=(size_flt+size_int);k<(size_flt+size_int+size_btr);k++) {
		        new_oper1 = O_address_add(swap_offset, NULL,
						R_CALLEE_SAVE_CODE, L_PTYPE_NULL);
                        L_insert_oper_before(cb,oper,new_oper1);
                        new_oper2 = new_load_operation(callee_reg_array[k],
				    L_CTYPE_BTR, swap_offset, R_CALLEE_SAVE_CODE);
                        L_insert_oper_before(cb,oper,new_oper2);
                        swap_offset += 4;
		    }	
		}
	        if (O_require_callee_save_code(L_CTYPE_PREDICATE)) {
                    /* insert a PRED_ST_BLK if there are any callee save pred regs */
		    if (Set_size(callee_predicate_set) > 0) {
		        new_oper1 = O_address_add(swap_offset, NULL,
					    R_CALLEE_SAVE_CODE, L_PTYPE_NULL);
                        L_insert_oper_before(cb, oper, new_oper1);
                        new_oper2 = new_blk_load_operation(callee_predicate_set,
						swap_offset, R_CALLEE_SAVE_CODE);
                        L_insert_oper_before(cb, oper, new_oper2);
		        swap_offset += ((Lplaydoh_num_prd_callee_reg+Lplaydoh_num_prd_caller_reg+31)/32)*4;
		    }
		}

		break;

	    default:
		break;
	    }
	}
    }

    /* Hack to fixup predicate attributes for spill code inserted SAM 10-94 */
    L_fix_vpred_attrs(fn);

    /*
     *	Free up resource.
     */
    free(callee_reg_array);

    /* Reset the register usage sets */
    caller_int_set = Set_dispose(caller_int_set);
    callee_int_set = Set_dispose(callee_int_set);
    caller_float_set = Set_dispose(caller_float_set);
    callee_float_set = Set_dispose(callee_float_set);
    caller_double_set = Set_dispose(caller_double_set);
    callee_double_set = Set_dispose(callee_double_set);
    caller_predicate_set = Set_dispose(caller_predicate_set);
    callee_predicate_set = Set_dispose(callee_predicate_set);
    caller_btr_set = Set_dispose(caller_btr_set);
    callee_btr_set = Set_dispose(callee_btr_set);
}
