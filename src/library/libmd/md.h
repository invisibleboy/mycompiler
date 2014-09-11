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
 *      File:   md.h
 *
 *      Description: Header file for the interface to the internal 
 *                   representation of the IMPACT Meta-Description Language
 *
 *      Creation Date:  March 1995
 *
 *      Authors: John C. Gyllenhaal and Wen-mei Hwu
 *
 *      Revisions:
 *           Shail Aditya (HP Labs) February 1996
 *           Changed MD_get_xxx() arguments to be consistent with MD_set_xxx()
 *           Added MD_num_xxx() which returns number of sections, entries, etc.
 *
 *           John C. Gyllenhaal  May 1996
 *           Folded Shail's changes back into IMPACT's version
 *           Changed low-level representation to use hex for integers
 *           Added MD_rename_entry(), MD_delete_element(), MD_max_field_index()
 *                 MD_max_element_index(), and MD_print_md_declarations()
 *           Changed all error messages to uniformly use MD_punt()
 * 
 * 	     John C. Gyllenhaal July 1996
 *	     Implemented Shail Aditya's suggestion of adding void pointers to
 *           MD_Section and MD_Entry for the user's use.  Added functions
 *           MD_set_section_ext(), MD_get_section_ext(), MD_set_entry_ext(), 
 *           and MD_get_entry_ext().
 *
 *           John C. Gyllenhaal January 1998
 *           Implemented Marie Conte's and Andy Trick's suggestion of adding
 *           a new data type 'BLOCK' to allow an user to specify a block
 *           of binary data the same way ints, doubles, strings, and links
 *           are specified in fields.  (User readable input/output 
 *           respresents this data as a hex string.  Note that this data type
 *           may not make sense to programs running on different platforms or
 *           compiled with different compilers!)
 * 	     Also, switched to internal version of strdup (MD_strdup) in order 
 *           to handle 'out of memory' errors more gracefully.
 *
 * *
\*****************************************************************************/

#ifndef MD2_H
#define MD2_H

/* 10/29/02 REK Adding config.h */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
/* Stdio.h needed for prototypes */
#include <stdio.h>

/*
 * MD data types supported. 
 * (Use defines instead of enum to allow fitting into short int
 *  in MD_Element)
 */
#define MD_INT		1
#define MD_DOUBLE	2
#define MD_STRING	3
#define MD_LINK		4
#define MD_BLOCK	5

/* Field types supported */
typedef enum MD_FIELD_TYPE
{
    MD_REQUIRED_FIELD	= 1,
    MD_OPTIONAL_FIELD	= 2
} MD_FIELD_TYPE;

/* Used as index to declare optional element types */
#define MD_OPTIONAL_ELEMENTS	-1


/* Symbol table used for database section, entry, and field names.
 * Internal use only!  Not designed to be used by non-MD functions!
 */
typedef struct MD_Symbol
{
    char			*name;		/* Not a copy! */
    unsigned int		hash_val; 	/* Hashed value of name */
    void			*data;  	/* a MD_xxx structure */
    struct MD_Symbol_Table	*table;		/* For deletion of symbol */
    struct MD_Symbol		*next_hash;	/* For table's hash table */
    struct MD_Symbol		*prev_hash;
    struct MD_Symbol		*next_symbol;	/* For table's contents list */
    struct MD_Symbol		*prev_symbol;
    int				symbol_id;	/* Used by MD_write_md */

} MD_Symbol;

typedef struct MD_Symbol_Table
{
    struct MD		*md;		/* For error messages */
    char		*name;		/* For error messages */
    MD_Symbol		**hash;		/* Array of size hash_size */
    int			hash_size;	/* Must be power of 2 */
    int			hash_mask;	/* AND mask, (hash_size - 1) */
    int			resize_size;	/* When reached, resize hash table */
    MD_Symbol		*head_symbol;	/* Contents list */
    MD_Symbol		*tail_symbol;
    int			symbol_count;
} MD_Symbol_Table;

/* Types to fill out later */

typedef struct MD
{
    char		*name;		/* Name of MD */
    MD_Symbol_Table	*section_table;	/* Sections in the MD */
} MD;

typedef struct MD_Section
{
    MD			*md;		  /* MD that section is in */
    char 		*name;		  /* Name of section */
    MD_Symbol_Table	*field_decl_table;/* Field declared in the section */
    MD_Symbol_Table	*entry_table;	  /* Entries in the section */
    int			max_field_index;  /* Index of the max declared field */
    struct MD_Field_Decl **field_decl;	  /* Array of field decl pointers */
    int			field_array_size; /* Size of above field_decl array */
    					  /* and Entry's field array */
    MD_Symbol		*symbol;	  /* In MD's section_table */
    void		*user_ext;	  /* For user's use, NULL initially */
}
 MD_Section;

typedef struct MD_Element_Req
{
    struct MD_Field_Decl *field_decl;	/* Field decl that element_req is in */
    int			require_index;	/* Index in field_decl->require array*/
    short		type;		/* Type of element required */
    char		*desc;		/* Description (for error messages) */
    int			kleene_starred;	/* 1 if kleene_starred */
    MD_Section		**link;		/* Array of section pointers */
    int			link_array_size;/* Size/links in above link array */
} MD_Element_Req;

typedef struct MD_Field_Decl
{
    MD_Section		*section;	    /* Section that field decl is in */
    int			field_index;	    /* Index in entry->field array*/
    char 		*name;		    /* Name of field being declared */
    MD_FIELD_TYPE	type;		    /* Type of field being declared */
    int			max_require_index;  /* Index of last requirement */
    MD_Element_Req	**require;	    /* Decls of element requirements */
    int			require_array_size; /* Size of above require array */
    MD_Element_Req	*kleene_starred_req;/* NULL if no req starred */
    MD_Symbol		*symbol;            /* In section->field_decl_table */
} MD_Field_Decl;

/* 
 * Each entry has an array of pointers to fields (of size field_size).
 * The pointers in the field array will be NULL for unspecified fields.
 */
typedef struct MD_Entry
{
    MD_Section		*section;	/* Section that entry is in */
    char 		*name;		/* Name of entry */
    struct MD_Field	**field;	/* Array of field pointers, */
					/* of size section->field_array_size */
    MD_Symbol		*symbol;	/* In MD_Section's entry_table */
    void		*user_ext;	/* For user's use, NULL initially */
} MD_Entry;


/* The block respresentation used in MD_Element (designed to fit in 8 bytes).
 * Use macro interface routines for MD_Element for acessing data.
 */
typedef struct MD_Block
{
    unsigned int	size;		/* Size of block, may be zero */
    void		*ptr;		/* Pointer to block, NULL if size = 0*/
} MD_Block;

/* The data in the database.  
 * Use macro interface routines for accessing data.
 */
typedef struct MD_Element
{
    struct MD_Field	*field;		/* Field element is in */
    unsigned short	element_index;	/* index in field->element */
    short		type;		/* The type of data accessed */
    union				
    {
	int		i;		/* INT value */
	double		d;		/* DOUBLE value */
	char   		*s;		/* STRING value */
	MD_Entry	*l;		/* LINK value */
	MD_Block	b;		/* BLOCK value */
    } value;
} MD_Element;

/*
 * Each field has an array of pointers to elements (of size array_size).
 * max_index is the index of the last element set to a value, -1 if
 * no elements have been set.
 * The pointers in element array will be NULL for unspecified elements.
 */
typedef struct MD_Field
{
    MD_Entry		*entry;		   /* Entry that field is in */
    MD_Field_Decl	*decl;	 	   /* Declaration of field contents */
    int			max_element_index; /* max_index of last set element */
    MD_Element		**element;	   /* Pointers to elements in field */
    int			element_array_size;/* Size of above element array */
} MD_Field;


/*
 * MD prototypes/macros declarations
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Start prototypes of MD interface functions */
extern MD *MD_new_md (char *name, int num_sections);
extern int MD_check_md (FILE *out, MD *md);
extern void MD_delete_md (MD *md);

extern MD_Section *MD_new_section (MD *md, char *name, int num_entries, 
			    int num_fields);

extern MD_Section *MD_first_section (MD *md);
extern MD_Section *MD_last_section (MD *md);
extern MD_Section *MD_next_section (MD_Section *section);
extern MD_Section *MD_prev_section (MD_Section *section);

extern MD_Field_Decl *MD_new_field_decl (MD_Section *section, char *name, 
					 MD_FIELD_TYPE field_type);

extern MD_Field_Decl *MD_first_field_decl (MD_Section *section);
extern MD_Field_Decl *MD_last_field_decl (MD_Section *section);
extern MD_Field_Decl *MD_next_field_decl (MD_Field_Decl *field_decl);
extern MD_Field_Decl *MD_prev_field_decl (MD_Field_Decl *field_decl);

extern void MD_require_int (MD_Field_Decl *field_decl, int element_index);
extern void MD_require_double (MD_Field_Decl *field_decl, int element_index);
extern void MD_require_string (MD_Field_Decl *field_decl, int element_index);
extern void MD_require_block (MD_Field_Decl *field_decl, int element_index);
extern void MD_require_link (MD_Field_Decl *field_decl, int element_index, 
		      MD_Section *section);
extern void MD_require_multi_target_link (MD_Field_Decl *field_decl, 
				   int element_index, int section_array_size,
				   MD_Section **section_array);
extern void MD_kleene_star_requirement (MD_Field_Decl *field_decl, 
					int element_index);

extern MD_Entry *MD_new_entry (MD_Section *section, char *name);
extern void MD_rename_entry (MD_Entry *entry, char *new_name);

extern MD_Entry *MD_first_entry (MD_Section *section);
extern MD_Entry *MD_last_entry (MD_Section *section);
extern MD_Entry *MD_next_entry (MD_Entry *entry);
extern MD_Entry *MD_prev_entry (MD_Entry *entry);

extern MD_Field *MD_new_field (MD_Entry *entry, MD_Field_Decl *decl, 
			       int num_elements);
extern void MD_delete_field (MD_Field *field);
extern void MD_delete_field_decl (MD_Field_Decl *field_decl);
extern void MD_delete_element (MD_Field *field, int index);

extern MD *MD_read_md (FILE *in, char *name);
extern void MD_write_md (FILE *out, MD *md);
extern void MD_print_md (FILE *out, MD *md, int page_width);
extern void MD_print_md_declarations (FILE *out, MD *md, int page_width);
extern void MD_print_md_template (FILE *out, MD *md);

extern void MD_print_section (FILE *out, MD_Section *section, int page_width);
extern void MD_print_section_template (FILE *out, MD_Section *section);
extern void MD_print_field_decl (FILE *out, MD_Field_Decl *field_decl,
				 int page_width);
extern void MD_print_entry (FILE *out, MD_Entry *entry, int page_width);
extern void MD_print_entry_template (FILE *out, MD_Entry *entry);

extern void MD_delete_symbol (MD_Symbol *symbol, void (*free_routine)(void *));
extern void *_MD_find_symbol_data (MD_Symbol_Table *table, char *name);

#ifdef __cplusplus
}
#endif

/* End prototypes of MD interface functions */

/* 
 * MD macros.
 * Two versions available:
 *  1) macro replaced with code optimized for speed.
 *  2) macro replaced with function call version and some function calls
 *     changed to perform extra error checking on arguments, etc..
 *
 *  Version 2 is provided to aid debugging macro arguments and to test
 *  for subtle data type mismatches.
 */
#ifndef MD_DEBUG_MACROS

#define MD_num_sections(md)		((md)->section_table->symbol_count)
#define MD_find_section(md, name) \
            ((MD_Section *)_MD_find_symbol_data((md)->section_table, name))


#define MD_delete_section(section) \
            (MD_delete_symbol(((MD_Section *)(section))->symbol, \
			      (void (*)(void *))_MD_free_section))

#define MD_num_entries(section)         ((section)->entry_table->symbol_count)

#define MD_find_entry(section, name) \
            ((MD_Entry *)_MD_find_symbol_data((section)->entry_table, name))

#define MD_delete_entry(entry) \
            (MD_delete_symbol(((MD_Entry *)(entry))->symbol, \
			      (void (*)(void *))_MD_free_entry))

#define MD_num_field_decls(section) \
	    ((section)->field_decl_table->symbol_count)

#define MD_max_field_index(section)	((section)->max_field_index)

#define MD_find_field_decl(section, name) \
          ((MD_Field_Decl *)_MD_find_symbol_data((section)->field_decl_table, \
						 name))

#define MD_find_field(entry, field_decl) \
            ((entry)->field[(field_decl)->field_index])

#define MD_num_elements(field) 		((field)->max_element_index + 1)
#define MD_max_element_index(field)	((field)->max_element_index)

#define MD_set_int(field, index, value) _MD_set_int(field, index, value)
#define MD_get_int(field, index) 	((field)->element[index]->value.i)

#define MD_set_double(field, index, value) _MD_set_double(field, index, value)
#define MD_get_double(field, index) 	((field)->element[index]->value.d)

#define MD_set_string(field, index, value) _MD_set_string(field, index, value)
#define MD_get_string(field, index) 	((field)->element[index]->value.s)

#define MD_set_block(field, index, size, ptr) \
    	    _MD_set_block(field, index, size, ptr)
#define MD_get_block_size(field, index) \
    	    ((field)->element[index]->value.b.size)
#define MD_get_block_ptr(field, index) \
    	    ((field)->element[index]->value.b.ptr)

#define MD_set_link(field, index, value) _MD_set_link(field, index, value)
#define MD_get_link(field, index) 	((field)->element[index]->value.l)

#define MD_set_section_ext(section, ext) ((section)->user_ext = (void *)(ext))
#define MD_get_section_ext(section) 	((section)->user_ext)

#define MD_set_entry_ext(entry, ext) 	((entry)->user_ext = (void *)(ext))
#define MD_get_entry_ext(entry) 	((entry)->user_ext)

#else

#define MD_num_sections(md)		_MD_num_sections(md)
#define MD_find_section(md, name)	_MD_find_section(md, name)
#define MD_delete_section(section)	_MD_delete_section(section)

#define MD_num_entries(section)		_MD_num_entries(section)
#define MD_find_entry(section, name)	_MD_find_entry(section, name)
#define MD_delete_entry(entry)		_MD_delete_entry(entry)


#define MD_num_field_decls(section)	_MD_num_field_decls(section)
#define MD_max_field_index(section)	_MD_max_field_index(section)
#define MD_find_field_decl(section, name) _MD_find_field_decl(section, name)

#define MD_find_field(entry, field_decl) _MD_find_field(entry, field_decl)

#define MD_num_elements(field) 		_MD_num_elements(field)
#define MD_max_element_index(field)	_MD_max_element_index(field)

#define MD_set_int(field, index, value) \
	    _MD_set_int_type_checking(field, index, value)
#define MD_get_int(field, index)	_MD_get_int(field, index)

#define MD_set_double(field, index, value) \
	    _MD_set_double_type_checking(field, index, value)
#define MD_get_double(field, index)	_MD_get_double(field, index)

#define MD_set_string(field, index, value) \
	    _MD_set_string_type_checking(field, index, value)
#define MD_get_string(field, index)	_MD_get_string(field, index)

#define MD_set_block(field, index, size, ptr) \
	    _MD_set_block_type_checking(field, index, size, ptr)
#define MD_get_block_size(field, index)	_MD_get_block_size(field, index)
#define MD_get_block_ptr(field, index)	_MD_get_block_ptr(field, index)

#define MD_set_link(field, index, value) \
	    _MD_set_link_type_checking(field, index, value)
#define MD_get_link(field, index)	_MD_get_link(field, index)

#define MD_set_section_ext(section, ext) \
	    _MD_set_section_ext(section, (void *)(ext))
#define MD_get_section_ext(section) 	_MD_get_section_ext(section)

#define MD_set_entry_ext(entry, ext) 	_MD_set_entry_ext(entry, (void *)(ext))
#define MD_get_entry_ext(entry) 	_MD_get_entry_ext(entry)


#endif

/* MD checking macros (only one version available) */
#define MD_check_section(out, section) \
             _MD_check_section(out, section, "MD_check_section")
#define MD_check_entry(out, entry) \
             _MD_check_entry(out, entry, "MD_check_entry")
#define MD_check_field(out, field) \
             _MD_check_field(out, field, "MD_check_field")






/* Start prototypes of functions used by MD macros.  
 * DO NOT CALL THE FUNCTIONS BELOW DIRECTLY!
 */

#ifdef __cplusplus
extern "C" {
#endif



extern int _MD_num_sections (MD *md);
extern MD_Section *_MD_find_section (MD *md, char *name);
extern void *_MD_set_section_ext(MD_Section *section, void *ext);
extern void *_MD_get_section_ext(MD_Section *section);
extern void _MD_free_section (MD_Section *section);
extern void _MD_delete_section (MD_Section *section);
extern int _MD_check_section (FILE *out, MD_Section *section, 
			      char *caller_name);


extern int _MD_num_entries (MD_Section *section);
extern MD_Entry *_MD_find_entry (MD_Section *section, char *name);
extern void *_MD_set_entry_ext(MD_Entry *entry, void *ext);
extern void *_MD_get_entry_ext(MD_Entry *entry);
extern void _MD_free_entry (MD_Entry *entry);
extern void _MD_delete_entry (MD_Entry *entry);
extern int _MD_check_entry (FILE *out, MD_Entry *entry, char *caller_name);

extern MD_Field_Decl *_MD_find_field_decl (MD_Section *section, char *name);
extern void _MD_free_field_decl (MD_Field_Decl *field_decl);

extern int _MD_num_field_decls (MD_Section *section);
extern int _MD_max_field_index (MD_Section *section);

extern MD_Field *_MD_find_field (MD_Entry *entry, MD_Field_Decl *field_decl);
extern int _MD_check_field (FILE *out, MD_Field *field, char *caller_name);

extern int _MD_num_elements (MD_Field *field);
extern int _MD_max_element_index (MD_Field *field);

extern void _MD_set_int (MD_Field *field, int index, int value);
extern void _MD_set_int_type_checking (MD_Field *field, int index, int value);
extern int _MD_get_int (MD_Field *field, int index);

extern void _MD_set_double (MD_Field *field, int index, double value);
extern void _MD_set_double_type_checking (MD_Field *field, int index,
					  double value);
extern double _MD_get_double (MD_Field *field, int index);

extern void _MD_set_string (MD_Field *field, int index, char *value);
extern void _MD_set_string_type_checking (MD_Field *field, int index,
					  char *value);
extern char *_MD_get_string (MD_Field *field, int index);


extern void _MD_set_block (MD_Field *field, int index, unsigned int size,
			   void *ptr);
extern void _MD_set_block_type_checking (MD_Field *field, int index,
					 unsigned int size, void *ptr);
extern int _MD_get_block_size (MD_Field *field, int index);
extern void *_MD_get_block_ptr (MD_Field *field, int index);

extern void _MD_set_link (MD_Field *field, int index, MD_Entry *value);
extern void _MD_set_link_type_checking (MD_Field *field, int index,
					MD_Entry *value);
extern MD_Entry *_MD_get_link (MD_Field *field, int index);

#ifdef __cplusplus
}
#endif
/* End prototypes of functions used by MD macros.  
 * DO NOT CALL THE FUNCTIONS ABOVE DIRECTLY!
 */

#endif




