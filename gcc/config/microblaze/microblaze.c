/* Copyright (C) 1989, 90, 91, 93-98, 1999 Free Software Foundation, Inc.
   This file is part of GNU CC.

   GNU CC is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   GNU CC is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU CC; see the file COPYING.  If not, write to
   the Free Software Foundation, 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/***************************************************************************************-*-C-*- 
 * 
 * Copyright (c) 2001 Xilinx, Inc.  All rights reserved. 
 * 
 * microblaze.c : Home
 * 
 * MicroBlaze specific file. Contains functions for generating MicroBlaze code
 * Certain sections of code are from the Free Software Foundation
 * 
 * $Header: /devl/xcs/repo/env/Jobs/MDT/sw/ThirdParty/gnu/src/gcc/src-3.4/gcc/config/microblaze/microblaze.c,v 1.14.2.16 2006/05/22 15:25:27 vasanth Exp $
 * 
 *******************************************************************************/

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include <signal.h>
#include "tm.h"
#include "rtl.h"
#include "regs.h"
#include "hard-reg-set.h"
#include "real.h"
#include "insn-config.h"
#include "conditions.h"
#include "insn-flags.h"
#include "insn-attr.h"
#include "recog.h"
#include "toplev.h"
#include "tree.h"
#include "function.h"
#include "expr.h"
#include "flags.h"
#include "reload.h"
#include "output.h"
#include "ggc.h"
#include "hashtab.h"
#include "target.h"
#include "target-def.h"
#include "tm_p.h"
#include "integrate.h"

#if defined(USG) || !defined(HAVE_STAB_H)
#include "gstab.h"  /* If doing DBX on sysV, use our own stab.h.  */
#else
#include <stab.h>  /* On BSD, use the system's stab.h.  */
#endif /* not USG */

#ifdef __GNU_STAB__
#define STAB_CODE_TYPE enum __stab_debug_code
#else
#define STAB_CODE_TYPE int
#endif

/* Enumeration for all of the relational tests, so that we can build
   arrays indexed by the test type, and not worry about the order
   of EQ, NE, etc. */

enum internal_test {
  ITEST_EQ,
  ITEST_NE,
  ITEST_GT,
  ITEST_GE,
  ITEST_LT,
  ITEST_LE,
  ITEST_GTU,
  ITEST_GEU,
  ITEST_LTU,
  ITEST_LEU,
  ITEST_MAX
};

/* Forward declaration for sruct constant */
struct constant;

/* Classifies an address.

ADDRESS_INVALID
An invalid address.

ADDRESS_REG

A natural register or a register + const_int offset address.  
The register satisfies microblaze_valid_base_register_p and the 
offset is a const_arith_operand.

ADDRESS_REG_INDEX

A natural register offset by the index contained in an index register. The base
register satisfies microblaze_valid_base_register_p and the index register
satisfies microblaze_valid_index_register_p

ADDRESS_CONST_INT

A signed 16/32-bit constant address.

ADDRESS_SYMBOLIC:

A constant symbolic address or a (register + symbol).  */

enum microblaze_address_type {
  ADDRESS_INVALID,
  ADDRESS_REG,
  ADDRESS_REG_INDEX,
  ADDRESS_CONST_INT,
  ADDRESS_SYMBOLIC,
  ADDRESS_GOTOFF,
  ADDRESS_PLT
};

/* Classifies symbols

SYMBOL_TYPE_GENERAL
        
A general symbol. */
enum microblaze_symbol_type {
  SYMBOL_TYPE_INVALID,
  SYMBOL_TYPE_GENERAL
};

/* Classification of a Microblaze address */
struct microblaze_address_info
{
  enum microblaze_address_type type;
  rtx regA;                       /* Contains valid values on ADDRES_REG, ADDRESS_REG_INDEX, ADDRESS_SYMBOLIC */
  rtx regB;                       /* Contains valid values on ADDRESS_REG_INDEX */
  rtx offset;                     /* Contains valid values on ADDRESS_CONST_INT and ADDRESS_REG */
  rtx symbol;                     /* Contains valid values on ADDRESS_SYMBOLIC */
  enum microblaze_symbol_type symbol_type;
};

static void microblaze_encode_section_info	PARAMS ((tree, rtx, int));
static void microblaze_globalize_label          PARAMS ((FILE*, const char*));
static void microblaze_function_prologue        PARAMS ((FILE*, int));
static void microblaze_function_epilogue        PARAMS ((FILE*, HOST_WIDE_INT));
static char* microblaze_fill_delay_slot         PARAMS ((char *, enum delay_type ,rtx [],rtx ));
static void microblaze_count_memory_refs        PARAMS ((rtx, int));
static rtx embedded_pic_fnaddr_reg              PARAMS ((void));
static void microblaze_internal_label           PARAMS ((FILE *, const char*, unsigned long));
static bool microblaze_rtx_costs                PARAMS ((rtx, int, int, int*));
static int microblaze_address_cost              PARAMS ((rtx));
static int microblaze_address_insns             PARAMS ((rtx, enum machine_mode));
static void microblaze_asm_constructor          PARAMS ((rtx, int));
static void microblaze_asm_destructor           PARAMS ((rtx, int));
static void microblaze_select_section           PARAMS ((tree, int, unsigned HOST_WIDE_INT));
static void microblaze_select_rtx_section       PARAMS ((enum machine_mode, rtx, unsigned HOST_WIDE_INT));
bool microblaze_legitimate_address_p            PARAMS ((enum machine_mode, rtx, int ));
rtx  microblaze_legitimize_address              PARAMS ((rtx , rtx, enum machine_mode));
int microblaze_regno_ok_for_base_p              PARAMS ((int, int));
static const char* microblaze_mode_to_mem_modifier    PARAMS ((int, enum machine_mode));
static bool microblaze_valid_base_register_p    PARAMS ((rtx, enum machine_mode, int));
static bool microblaze_valid_index_register_p   PARAMS ((rtx, enum machine_mode, int));
static bool microblaze_classify_address         PARAMS ((struct microblaze_address_info *, rtx, enum machine_mode, int));
HOST_WIDE_INT compute_frame_size                PARAMS ((HOST_WIDE_INT));
int double_memory_operand                       PARAMS ((rtx, enum machine_mode));
void microblaze_output_filename                 PARAMS ((FILE*, const char*));
HOST_WIDE_INT microblaze_initial_elimination_offset 
                                                PARAMS ((int, int));
int microblaze_sched_use_dfa_pipeline_interface PARAMS ((void));
void microblaze_function_end_prologue           PARAMS ((FILE *));
//static enum internal_test map_test_to_internal_test	
//                                                PARAMS ((enum rtx_code));
static void microblaze_block_move_straight      PARAMS ((rtx, rtx, HOST_WIDE_INT));
static void microblaze_adjust_block_mem         PARAMS ((rtx, HOST_WIDE_INT, rtx *, rtx *));
static void microblaze_block_move_loop          PARAMS ((rtx, rtx, HOST_WIDE_INT));
bool        microblaze_expand_block_move        PARAMS ((rtx, rtx, rtx, rtx));
static void save_restore_insns			PARAMS ((int));
//static rtx add_constant				PARAMS ((struct constant **, rtx, enum machine_mode));
//static void dump_constants			PARAMS ((struct constant *, rtx));
static int microblaze_version_to_int            PARAMS ((const char *));
static int microblaze_version_compare           PARAMS ((const char *, const char *));
void microblaze_order_regs_for_local_alloc 	PARAMS ((void));
void print_operand 				PARAMS ((FILE *, rtx, int));
void print_operand_address 			PARAMS ((FILE *, rtx));
void final_prescan_insn 			PARAMS ((rtx, rtx *, int));
void microblaze_internal_label 			PARAMS ((FILE *, const char*, unsigned long));
void microblaze_output_float 			PARAMS ((FILE *, REAL_VALUE_TYPE value));
int microblaze_can_use_return_insn 		PARAMS ((void));
enum reg_class microblaze_secondary_reload_class PARAMS ((enum reg_class, enum machine_mode, rtx, int));
int simple_memory_operand 			PARAMS ((rtx, enum machine_mode));
void trace 					PARAMS ((const char *, const char *, const char *));
void gen_conditional_branch 			PARAMS ((rtx *, enum rtx_code));
void init_cumulative_args 			PARAMS ((CUMULATIVE_ARGS *,tree, rtx));
int function_arg_partial_bytes 			PARAMS ((CUMULATIVE_ARGS *, enum machine_mode, tree, int));
HOST_WIDE_INT microblaze_debugger_offset 	PARAMS ((rtx, HOST_WIDE_INT));
void microblaze_output_lineno 			PARAMS ((FILE *, int));
void microblaze_internal_label 			PARAMS ((FILE *, const char*, unsigned long));
void microblaze_output_double 			PARAMS ((FILE *, REAL_VALUE_TYPE));
static int microblaze_save_volatiles 		PARAMS ((tree func));
const char* microblaze_move_2words 		PARAMS ((rtx *, rtx));
void microblaze_declare_object 			PARAMS ((FILE *, char *, char *, char *, int));
int microblaze_valid_machine_decl_attribute 	PARAMS ((tree, tree, tree, tree));
static bool microblaze_handle_option 		PARAMS ((size_t, const char *, int));
int microblaze_is_interrupt_handler		PARAMS ((void));
int microblaze_const_double_ok 			PARAMS ((rtx, enum machine_mode));
static int microblaze_must_save_register 	PARAMS ((int));
void output_ascii 				PARAMS ((FILE *, const char *, int));
static bool microblaze_classify_unspec 		PARAMS ((struct microblaze_address_info *, rtx));
static bool microblaze_elf_in_small_data_p      PARAMS ((tree));

/* Global variables for machine-dependent things.  */

static unsigned int microblaze_select_flags = 0;

/* Toggle which pipleline interface to use */
int microblaze_sched_use_dfa = 0;

/* Threshold for data being put into the small data/bss area, instead
   of the normal data area (references to the small data/bss area take
   1 instruction, and use the global pointer, references to the normal
   data area takes 2 instructions).  */
int microblaze_section_threshold = -1;

/* Prevent scheduling potentially exception causing instructions in delay slots.
   -mcpu=v3.00.a or v4.00.a turns this on.
*/
int microblaze_no_unsafe_delay;

/* Count the number of .file directives, so that .loc is up to date.  */
int num_source_filenames = 0;

/* Count the number of sdb related labels are generated (to find block
   start and end boundaries).  */
int sdb_label_count = 0;

/* Next label # for each statement for Silicon Graphics IRIS systems. */
int sym_lineno = 0;

/* Non-zero if inside of a function, because the stupid asm can't
   handle .files inside of functions.  */
int inside_function = 0;

/* Name of the file containing the current function.  */
const char *current_function_file = "";

int file_in_function_warning = FALSE;

/* Whether to suppress issuing .loc's because the user attempted
   to change the filename within a function.  */
int ignore_line_number = FALSE;

/* Number of nested .set noreorder, noat, nomacro, and volatile requests.  */
int set_noreorder;
int set_noat;
int set_nomacro;
int set_volatile;


/* Count of delay slots and how many are filled.  */
int dslots_load_total;
int dslots_load_filled;
int dslots_jump_total;
int dslots_jump_filled;

/* # of nops needed by previous insn */
int dslots_number_nops;

/* Number of 1/2/3 word references to data items (ie, not brlid's).  */
int num_refs[3];

/* registers to check for load delay */
rtx microblaze_load_reg, microblaze_load_reg2, microblaze_load_reg3, microblaze_load_reg4;

/* Cached operands, and operator to compare for use in set/branch on
   condition codes.  */
rtx branch_cmp[2];

/* what type of branch to use */
enum cmp_type branch_type;

/* Number of previously seen half-pic pointers and references.  */
static int prev_half_pic_ptrs = 0;
static int prev_half_pic_refs = 0;

/* Which CPU pipeline do we use. We haven't really standardized on a CPU version 
   having only a particular type of pipeline. There can still be options on the CPU 
   to scale pipeline features up or down. :( Bad Presentation (??), so we let the
   MD file rely on the value of this variable instead 
   Making PIPE_5 the default. It should be backward optimal with PIPE_3 MicroBlazes */
enum pipeline_type microblaze_pipe = MICROBLAZE_PIPE_5;

/* Generating calls to position independent functions?  */
enum microblaze_abicalls_type microblaze_abicalls;

/* High and low marks for floating point values which we will accept
   as legitimate constants for LEGITIMATE_CONSTANT_P.  These are
   initialized in override_options.  */
REAL_VALUE_TYPE dfhigh, dflow, sfhigh, sflow;

/* Mode used for saving/restoring general purpose registers.  */
static enum machine_mode gpr_mode;

/* Array giving truth value on whether or not a given hard register
   can support a given mode.  */
char microblaze_hard_regno_mode_ok[(int)MAX_MACHINE_MODE][FIRST_PSEUDO_REGISTER];

/* Current frame information calculated by compute_frame_size.  */
struct microblaze_frame_info current_frame_info;

/* Zero structure to initialize current_frame_info.  */
struct microblaze_frame_info zero_frame_info;

/* Temporary filename used to buffer .text until end of program
   for -mgpopt.  */
//static char *temp_filename;

/* Pseudo-reg holding the address of the current function when
   generating embedded PIC code.  Created by LEGITIMIZE_ADDRESS, used
   by microblaze_finalize_pic if it was created.  */
rtx embedded_pic_fnaddr_rtx;

struct string_constant
{
  struct string_constant *next;
  char *label;
};

static struct string_constant *string_constants;

/* List of all MICROBLAZE punctuation characters used by print_operand.  */
char microblaze_print_operand_punct[256];

/* Map GCC register number to debugger register number.  */
int microblaze_dbx_regno[FIRST_PSEUDO_REGISTER];

/* Hardware names for the registers.  If -mrnames is used, this
   will be overwritten with microblaze_sw_reg_names.  */

char microblaze_reg_names[][8] =
{
  "r0",   "r1",   "r2",   "r3",   "r4",   "r5",   "r6",   "r7",
  "r8",   "r9",   "r10",  "r11",  "r12",  "r13",  "r14",  "r15",
  "r16",  "r17",  "r18",  "r19",  "r20",  "r21",  "r22",  "r23",
  "r24",  "r25",  "r26",  "r27",  "r28",  "r29",  "r30",  "r31",
  "$f0",  "$f1",  "$f2",  "$f3",  "$f4",  "$f5",  "$f6",  "$f7",
  "$f8",  "$f9",  "$f10", "$f11", "$f12", "$f13", "$f14", "$f15",
  "$f16", "$f17", "$f18", "$f19", "$f20", "$f21", "$f22", "$f23",
  "$f24", "$f25", "$f26", "$f27", "$f28", "$f29", "$f30", "$f31",
  "hi",   "lo",   "accum","rmsr", "$fcc1","$fcc2","$fcc3","$fcc4",
  "$fcc5","$fcc6","$fcc7","$ap",  "$rap", "$frp"
};

/* MicroBlaze software names for the registers, used to overwrite the
   microblaze_reg_names array.  */

char microblaze_sw_reg_names[][8] =
{
  "r0",   "r1",   "r2",   "r3",   "r4",   "r5",   "r6",   "r7",
  "r8",   "r9",   "r10",  "r11",  "r12",  "r13",  "r14",  "r15",
  "r16",  "r17",  "r18",  "r19",  "r20",  "r21",  "r22",  "r23",
  "r24",  "r25",  "r26",  "r27",  "r28",  "r29",  "r30",  "r31",
  "$f0",  "$f1",  "$f2",  "$f3",  "$f4",  "$f5",  "$f6",  "$f7",
  "$f8",  "$f9",  "$f10", "$f11", "$f12", "$f13", "$f14", "$f15",
  "$f16", "$f17", "$f18", "$f19", "$f20", "$f21", "$f22", "$f23",
  "$f24", "$f25", "$f26", "$f27", "$f28", "$f29", "$f30", "$f31",
  "hi",   "lo",   "accum","rmsr", "$fcc1","$fcc2","$fcc3","$fcc4",
  "$fcc5","$fcc6","$fcc7","$ap",  "$rap", "$frp"
};

/* Map hard register number to register class */
enum reg_class microblaze_regno_to_class[] =
{
  GR_REGS,	GR_REGS,	GR_REGS,	GR_REGS,
  GR_REGS,	GR_REGS,	GR_REGS,	GR_REGS,
  GR_REGS,	GR_REGS,	GR_REGS,	GR_REGS,
  GR_REGS,	GR_REGS,	GR_REGS,	GR_REGS,
  GR_REGS,	GR_REGS,	GR_REGS,	GR_REGS,
  GR_REGS,	GR_REGS,	GR_REGS,	GR_REGS,
  GR_REGS,	GR_REGS,	GR_REGS,	GR_REGS,
  GR_REGS,	GR_REGS,	GR_REGS,	GR_REGS,
  FP_REGS,	FP_REGS,	FP_REGS,	FP_REGS,
  FP_REGS,	FP_REGS,	FP_REGS,	FP_REGS,
  FP_REGS,	FP_REGS,	FP_REGS,	FP_REGS,
  FP_REGS,	FP_REGS,	FP_REGS,	FP_REGS,
  FP_REGS,	FP_REGS,	FP_REGS,	FP_REGS,
  FP_REGS,	FP_REGS,	FP_REGS,	FP_REGS,
  FP_REGS,	FP_REGS,	FP_REGS,	FP_REGS,
  FP_REGS,	FP_REGS,	FP_REGS,	FP_REGS,
  HI_REG,	LO_REG,		HILO_REG,	ST_REGS,
  ST_REGS,	ST_REGS,	ST_REGS,	ST_REGS,
  ST_REGS,	ST_REGS,	ST_REGS,	GR_REGS,
  GR_REGS,    GR_REGS
};

/* Map register constraint character to register class.  */
enum reg_class microblaze_char_to_class[256] =
{
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
  NO_REGS,	NO_REGS,	NO_REGS,	NO_REGS,
};

int div_count = 0;
extern void rodata_section(void);
extern void sbss_section(void);
extern void bss_section(void);
extern void sdata2_section(void);
int get_base_reg(rtx);
static int printed = 0;
enum load_store {LOAD = 0, STORE=1};
char *format_load_store(char*, enum load_store,  enum machine_mode ,  rtx,int);
// static int prev_offset;

/* True if the current function is an interrupt handler
   (either via #pragma or an attribute specification).  */
int interrupt_handler;
int save_volatiles;

/* Microblaze specific machine attributes.
 * interrupt_handler - Interrupt handler attribute to add interrupt prologue and epilogue and use appropriate interrupt return.
 * save_volatiles    - Similiar to interrupt handler, but use normal return.
 */
const struct attribute_spec microblaze_attribute_table[] = 
{
  /* name             , min_len, max_len, decl_req, type_req, fn_type, req_handler */
  {"interrupt_handler", 0, 0, true, false, false, NULL},
  {"save_volatiles"   , 0, 0, true, false, false, NULL},
  { NULL,        0, 0, false, false, false, NULL }
};

static int microblaze_interrupt_function_p (tree);
static int microblaze_save_volatiles (tree);

#undef TARGET_ENCODE_SECTION_INFO
#define TARGET_ENCODE_SECTION_INFO      microblaze_encode_section_info

#undef TARGET_ASM_GLOBALIZE_LABEL
#define TARGET_ASM_GLOBALIZE_LABEL      microblaze_globalize_label

#undef TARGET_ASM_FUNCTION_PROLOGUE
#define TARGET_ASM_FUNCTION_PROLOGUE    microblaze_function_prologue

#undef TARGET_ASM_FUNCTION_EPILOGUE
#define TARGET_ASM_FUNCTION_EPILOGUE    microblaze_function_epilogue

#undef TARGET_ASM_INTERNAL_LABEL
#define TARGET_ASM_INTERNAL_LABEL       microblaze_internal_label

#undef TARGET_RTX_COSTS          
#define TARGET_RTX_COSTS                microblaze_rtx_costs

#undef TARGET_ADDRESS_COST              
#define TARGET_ADDRESS_COST             microblaze_address_cost

#undef TARGET_ATTRIBUTE_TABLE           
#define TARGET_ATTRIBUTE_TABLE          microblaze_attribute_table

#undef TARGET_ASM_CONSTRUCTOR
#define TARGET_ASM_CONSTRUCTOR          microblaze_asm_constructor

#undef TARGET_ASM_DESTRUCTOR
#define TARGET_ASM_DESTRUCTOR           microblaze_asm_destructor

#undef TARGET_IN_SMALL_DATA_P
#define TARGET_IN_SMALL_DATA_P          microblaze_elf_in_small_data_p

#undef TARGET_ASM_SELECT_RTX_SECTION   
#define TARGET_ASM_SELECT_RTX_SECTION   microblaze_select_rtx_section

#undef TARGET_ASM_SELECT_SECTION
#define TARGET_ASM_SELECT_SECTION       microblaze_select_section

#undef TARGET_HAVE_SRODATA_SECTION
#define TARGET_HAVE_SRODATA_SECTION     true

#undef TARGET_SCHED_USE_DFA_PIPELINE_INTERFACE
#define TARGET_SCHED_USE_DFA_PIPELINE_INTERFACE \
                                        microblaze_sched_use_dfa_pipeline_interface

#undef TARGET_ASM_FUNCTION_END_PROLOGUE 
#define TARGET_ASM_FUNCTION_END_PROLOGUE \
                                        microblaze_function_end_prologue 

#undef TARGET_HANDLE_OPTION
#define TARGET_HANDLE_OPTION		microblaze_handle_option

#undef TARGET_DEFAULT_TARGET_FLAGS
#define TARGET_DEFAULT_TARGET_FLAGS	TARGET_DEFAULT

#undef TARGET_ARG_PARTIAL_BYTES
#define TARGET_ARG_PARTIAL_BYTES	function_arg_partial_bytes

#undef TARGET_PROMOTE_FUNCTION_RETURN
#define TARGET_PROMOTE_FUNCTION_RETURN 	hook_bool_tree_true

struct gcc_target targetm = TARGET_INITIALIZER;

rtx
microblaze_return_addr_rtx (int count, rtx frameaddr ATTRIBUTE_UNUSED)
{
  rtx x;

  x = (count == 0) ? gen_rtx_PLUS (Pmode,
          get_hard_reg_initial_val(Pmode, MB_ABI_SUB_RETURN_ADDR_REGNUM),
               GEN_INT (8)) : (rtx) 0;
  return x;
}

/* Return truth value if a CONST_DOUBLE is ok to be a legitimate constant.  */

int
microblaze_const_double_ok (rtx op, enum machine_mode mode)
{
  REAL_VALUE_TYPE d;

  if (GET_CODE (op) != CONST_DOUBLE)
    return 0;

  if (mode == VOIDmode)
    return 1;

  if (mode != SFmode && mode != DFmode)
    return 0;

  if (op == CONST0_RTX (mode))
    return 1;

  REAL_VALUE_FROM_CONST_DOUBLE (d, op);

  if (REAL_VALUE_ISNAN (d))
    return FALSE;

  if (REAL_VALUE_NEGATIVE (d))
    d = REAL_VALUE_NEGATE (d);

  if (mode == DFmode)
  {
    if (REAL_VALUES_LESS (d, dfhigh)
        && REAL_VALUES_LESS (dflow, d))
      return 1;
  }
  else
  {
    if (REAL_VALUES_LESS (d, sfhigh)
        && REAL_VALUES_LESS (sflow, d))
      return 1;
  }

  return 0;
}

/* Return truth value if a memory operand fits in a single instruction
   (ie, register + small offset) or (register + register).  */

int
simple_memory_operand (rtx op, enum machine_mode mode ATTRIBUTE_UNUSED)
{
  rtx addr, plus0, plus1;

  /* Eliminate non-memory operations */
  if (GET_CODE (op) != MEM)
    return 0;

  /* dword operations really put out 2 instructions, so eliminate them.  */
  /* ??? This isn't strictly correct.  It is OK to accept multiword modes
     here, since the length attributes are being set correctly, but only
     if the address is offsettable.  */
  if (GET_MODE_SIZE (GET_MODE (op)) > UNITS_PER_WORD)
    return 0;

 
  /* Decode the address now.  */
  addr = XEXP (op, 0);
  switch (GET_CODE (addr))

  {
    case REG:
      return 1;

      /*    case CONST_INT:
            return SMALL_INT (op);
      */
    case PLUS:
      plus0 = XEXP (addr, 0);
      plus1 = XEXP (addr, 1);

      if (GET_CODE (plus0) == REG && GET_CODE (plus1) == CONST_INT && SMALL_INT (plus1)) {
        return 1;
      } else if (GET_CODE (plus1) == REG  && GET_CODE (plus0) == CONST_INT){
        return 1;
      } else if (GET_CODE (plus0) == REG  && GET_CODE (plus1) == REG) {
        return 1;
      } else 
        return 0;

    case SYMBOL_REF:
      return 0;

    default:
      break;
  }

  return 0;
}

/* Return nonzero for a memory address that can be used to load or store
   a doubleword.  */

int
double_memory_operand (rtx op, enum machine_mode mode)
{
  rtx addr;
   
  if (GET_CODE (op) != MEM
      || ! memory_operand (op, mode))
  {
    /* During reload, we accept a pseudo register if it has an
       appropriate memory address.  If we don't do this, we will
       wind up reloading into a register, and then reloading that
       register from memory, when we could just reload directly from
       memory.  */
    if (reload_in_progress
        && GET_CODE (op) == REG
        && REGNO (op) >= FIRST_PSEUDO_REGISTER
        && reg_renumber[REGNO (op)] < 0
        && reg_equiv_mem[REGNO (op)] != 0
        && double_memory_operand (reg_equiv_mem[REGNO (op)], mode))
      return 1;
    return 0;
  }

  /* Make sure that 4 added to the address is a valid memory address.
     This essentially just checks for overflow in an added constant.  */

  addr = XEXP (op, 0);

  if (CONSTANT_ADDRESS_P (addr))
    return 1;

  return memory_address_p ((GET_MODE_CLASS (mode) == MODE_INT
                            ? SImode
                            : SFmode),
                           plus_constant (addr, 4));  
}


/* This hook is called many times during insn scheduling. If the hook 
   returns nonzero, the automaton based pipeline description is used 
   for insn scheduling. Otherwise the traditional pipeline description 
   is used. The default is usage of the traditional pipeline description. */
int microblaze_sched_use_dfa_pipeline_interface (void)
{
  return microblaze_sched_use_dfa; 
}

/* Write a message to stderr (for use in macros expanded in files that do not
   include stdio.h).  */

void
trace (const char *s, const char *s1, const char *s2)
{
    fprintf (stderr, s, s1, s2);
}


/* Implement REG_OK_FOR_BASE_P -and- REG_OK_FOR_INDEX_P  */
int
microblaze_regno_ok_for_base_p (int regno, int strict)
{
  if (regno >= FIRST_PSEUDO_REGISTER)
  {
    if (!strict)
      return true;
    regno = reg_renumber[regno];
  }

  /* These fake registers will be eliminated to either the stack or
     hard frame pointer, both of which are usually valid base registers.
     Reload deals with the cases where the eliminated form isn't valid.  */
  if (regno == ARG_POINTER_REGNUM || regno == FRAME_POINTER_REGNUM)
    return true;

  return GP_REG_P (regno);
}

/* Return true if X is a valid base register for the given mode.
   Allow only hard registers if STRICT.  */

static bool
microblaze_valid_base_register_p (rtx x, enum machine_mode mode ATTRIBUTE_UNUSED, int strict)
{
  if (!strict && GET_CODE (x) == SUBREG)
    x = SUBREG_REG (x);

  return (GET_CODE (x) == REG
	  && microblaze_regno_ok_for_base_p (REGNO (x), strict));
}

static bool
microblaze_classify_unspec (struct microblaze_address_info *info, rtx x)
{
  info->symbol_type = SYMBOL_TYPE_GENERAL;
  info->symbol = XVECEXP(x,0,0);

  if (XINT(x,1) == UNSPEC_GOTOFF) {
    info->regA =  gen_rtx_REG (SImode, PIC_OFFSET_TABLE_REGNUM);
    info->type = ADDRESS_GOTOFF;
  } else if (XINT(x,1) == UNSPEC_PLT) {
    info->type = ADDRESS_PLT;
  } else {
    return false;
  }
  return true;
}


/* Return true if X is a valid index register for the given mode.
   Allow only hard registers if STRICT.  */

static bool
microblaze_valid_index_register_p (rtx x, enum machine_mode mode ATTRIBUTE_UNUSED, int strict)
{
  if (!strict && GET_CODE (x) == SUBREG)
    x = SUBREG_REG (x);

  return (GET_CODE (x) == REG
          /* A base register is good enough to be an index register on Microblaze */
	  && microblaze_regno_ok_for_base_p (REGNO (x), strict));            
}

/* Return true if X is a valid address for machine mode MODE.  If it is,
   fill in INFO appropriately.  STRICT is true if we should only accept
   hard base registers.  

      type                     regA      regB    offset      symbol

   ADDRESS_INVALID             NULL      NULL     NULL        NULL

   ADDRESS_REG                 %0        NULL     const_0 /   NULL
                                                  const_int
   ADDRESS_REG_INDEX           %0        %1       NULL        NULL

   ADDRESS_SYMBOLIC            r0 /      NULL     NULL        symbol    
                           sda_base_reg 

   ADDRESS_CONST_INT           r0       NULL      const       NULL

   For modes spanning multiple registers (DFmode in 32-bit GPRs,
   DImode, TImode), indexed addressing cannot be used because
   adjacent memory cells are accessed by adding word-sized offsets
   during assembly output.  */

static bool
microblaze_classify_address (struct microblaze_address_info *info, rtx x,
                             enum machine_mode mode, int strict)
{
  rtx xplus0;
  rtx xplus1;
    
  info->type = ADDRESS_INVALID;
  info->regA = NULL;
  info->regB = NULL;
  info->offset = NULL;
  info->symbol = NULL;
  info->symbol_type = SYMBOL_TYPE_INVALID;

  switch (GET_CODE (x))
  {
    case REG:
    case SUBREG:
    {
      info->type = ADDRESS_REG;
      info->regA = x;
      info->offset = const0_rtx;
      return microblaze_valid_base_register_p (info->regA, mode, strict);
    }   
    case PLUS:
    {
      xplus0 = XEXP (x, 0);				
      xplus1 = XEXP (x, 1);				
            
      if (microblaze_valid_base_register_p (xplus0, mode, strict)) {								
        info->type = ADDRESS_REG;
        info->regA = xplus0;

        if (GET_CODE (xplus1) == CONST_INT) {
          info->offset = xplus1;
          return true;							
        } else if (GET_CODE (xplus1) == UNSPEC) {
          return microblaze_classify_unspec(info, xplus1);
        } else if ((GET_CODE (xplus1) == SYMBOL_REF ||
                    GET_CODE (xplus1) == LABEL_REF) &&
                   flag_pic == 2) {
          return false;
        } else if (GET_CODE (xplus1) == SYMBOL_REF || 
                   GET_CODE (xplus1) == LABEL_REF  || 
                   GET_CODE (xplus1) == CONST) {                   
/*           if ((GET_CODE (xplus1) == SYMBOL_REF) &&                  /\* Prevent small data symbols from being accessed off an incompatible base register *\/ */
/*               ((get_base_reg (xplus1) == MB_ABI_BASE_REGNUM)  */
/*                ? 0  */
/*                : get_base_reg (xplus1) != REGNO (info->regA))) */
/*             return false; */
          if (GET_CODE (XEXP (xplus1, 0)) == UNSPEC)
            return microblaze_classify_unspec(info, XEXP (xplus1, 0));
          else if (flag_pic == 2) {
            return false;
          }
          info->type = ADDRESS_SYMBOLIC;
          info->symbol = xplus1;
          /* info->regA =  gen_rtx_raw_REG (mode, get_base_reg (xplus1)); */
          info->symbol_type = SYMBOL_TYPE_GENERAL;
          return true;
        } else if (GET_CODE (xplus1) == REG && microblaze_valid_index_register_p (xplus1, mode, strict) &&
                   (GET_MODE_SIZE (mode) <= UNITS_PER_WORD)) {      /* Restrict larger than word-width modes from using an index register */
          info->type = ADDRESS_REG_INDEX;
          info->regB = xplus1;
          return true;
        }
      }
      break;
    }
    case CONST_INT:
    {
      info->regA = gen_rtx_raw_REG (mode, 0);
      info->type = ADDRESS_CONST_INT;
      info->offset = x;
      return true;
    }    
    case CONST:
    case LABEL_REF:
    case SYMBOL_REF:
    {
      info->type = ADDRESS_SYMBOLIC;
      info->symbol_type = SYMBOL_TYPE_GENERAL;
      info->symbol = x;
      info->regA =  gen_rtx_raw_REG (mode, get_base_reg (x));
      if (HALF_PIC_P () || HALF_PIC_ADDRESS_P (x))
        return false;
            
      if (GET_CODE (x) == CONST) {
        return !(flag_pic && pic_address_needs_scratch (x));
      } else if (flag_pic == 2) {
        return false;
      }

      return true;           
    }

    case UNSPEC:
    {
      if (reload_in_progress)
        regs_ever_live[PIC_OFFSET_TABLE_REGNUM] = 1;
      return microblaze_classify_unspec(info, x);
    }

    default:
      return false;
  }
    
  return false;
}

/* This function is used to implement GO_IF_LEGITIMATE_ADDRESS.  It
   returns a nonzero value if X is a legitimate address for a memory
   operand of the indicated MODE.  STRICT is nonzero if this function
   is called during reload.  */

bool
microblaze_legitimate_address_p (enum machine_mode mode, rtx x, int strict)
{
  struct microblaze_address_info addr;

  return microblaze_classify_address (&addr, x, mode, strict);
}


/* Try machine-dependent ways of modifying an illegitimate address
   to be legitimate.  If we find one, return the new, valid address.
   This is used from only one place: `memory_address' in explow.c.

   OLDX is the address as it was before break_out_memory_refs was
   called.  In some cases it is useful to look at this to decide what
   needs to be done.

   MODE is passed so that this function can use GO_IF_LEGITIMATE_ADDRESS.

   It is always safe for this function to do nothing.  It exists to
   recognize opportunities to optimize the output.

   For the MICROBLAZE, transform:

   memory(X + <large int>)

   into:

   Y = <large int> & ~0x7fff;
   Z = X + Y
   memory (Z + (<large int> & 0x7fff));

   This is for CSE to find several similar references, and only use one Z.

   When PIC, convert addresses of the form memory (symbol+large int) to
   memory (reg+large int).  */
rtx
microblaze_legitimize_address (rtx x, rtx oldx ATTRIBUTE_UNUSED,
                               enum machine_mode mode ATTRIBUTE_UNUSED)
{									
  register rtx xinsn = x, result;						
									
  if (TARGET_DEBUG_B_MODE)						
  {									
    GO_PRINTF ("\n========== LEGITIMIZE_ADDRESS\n");			
    GO_DEBUG_RTX (xinsn);						
  }									
									
  if (GET_CODE (xinsn) == CONST						
      && flag_pic && pic_address_needs_scratch (xinsn))		        
  {									
    rtx ptr_reg = gen_reg_rtx (Pmode);				
    rtx constant = XEXP (XEXP (xinsn, 0), 1);				
									
    emit_move_insn (ptr_reg, XEXP (XEXP (xinsn, 0), 0));		
									
    result = gen_rtx_PLUS (Pmode, ptr_reg, constant);			
    if (SMALL_INT (constant))						
      return result;							
    /* Otherwise we fall through so the code below will fix the	
       constant.  */							
    xinsn = result;							
  }									
									
  if (GET_CODE (xinsn) == PLUS)						
  {									
    register rtx xplus0 = XEXP (xinsn, 0);				
    register rtx xplus1 = XEXP (xinsn, 1);				
    register enum rtx_code code0 = GET_CODE (xplus0);			
    register enum rtx_code code1 = GET_CODE (xplus1);			
									
    if (code0 != REG && code1 == REG)					
    {								
      xplus0 = XEXP (xinsn, 1);					
      xplus1 = XEXP (xinsn, 0);					
      code0 = GET_CODE (xplus0);					
      code1 = GET_CODE (xplus1);					
    }								
									
    if (code0 == REG && REG_OK_FOR_BASE_P (xplus0)		        
        && code1 == CONST_INT && !SMALL_INT (xplus1))			
    {								
      rtx int_reg = gen_reg_rtx (Pmode);				
      rtx ptr_reg = gen_reg_rtx (Pmode);				
									
      emit_move_insn (int_reg,					
                      GEN_INT (INTVAL (xplus1) & ~ 0x7fff));	
									
      emit_insn (gen_rtx_SET (VOIDmode,				
                          ptr_reg,					
                          gen_rtx_PLUS (Pmode, xplus0, int_reg)));	
									
      result = gen_rtx_PLUS (Pmode, ptr_reg,				
                        GEN_INT (INTVAL (xplus1) & 0x7fff));		
      return result;							
    }								

    if (code0 == REG && REG_OK_FOR_BASE_P (xplus0)
        && flag_pic == 2)
    {
      if (reload_in_progress)
        regs_ever_live[PIC_OFFSET_TABLE_REGNUM] = 1;
      if (code1 == CONST)
      {
        xplus1 = XEXP(xplus1, 0);
        code1 = GET_CODE(xplus1);
      }
      if (code1 == SYMBOL_REF)
      {
        result = gen_rtx_UNSPEC (Pmode, gen_rtvec (1, xplus1), UNSPEC_GOTOFF);
        result = gen_rtx_CONST (Pmode, result);
        result = gen_rtx_PLUS (Pmode, pic_offset_table_rtx, result);
        result = gen_const_mem (Pmode, result);
        result = gen_rtx_PLUS (Pmode, xplus0, result);
        return result;
      }
    }
  }									
 									
  if (GET_CODE (xinsn) == SYMBOL_REF)
  {
//    fprintf(stderr, "legitimize_address (sym):\n");
//    debug_rtx(xinsn);
    if (reload_in_progress)
      regs_ever_live[PIC_OFFSET_TABLE_REGNUM] = 1;
    result = gen_rtx_UNSPEC (Pmode, gen_rtvec (1, xinsn), UNSPEC_GOTOFF);
    result = gen_rtx_CONST (Pmode, result);
    result = gen_rtx_PLUS (Pmode, pic_offset_table_rtx, result);
    result = gen_const_mem (Pmode, result);
    return result;
  }
									
  if (TARGET_DEBUG_B_MODE)						
    GO_PRINTF ("LEGITIMIZE_ADDRESS could not fix.\n");			

  return NULL_RTX;
}


/* Returns an operand string for the given instruction's delay slot,
   after updating filled delay slot statistics.

   We assume that operands[0] is the target register that is set.

   In order to check the next insn, most of this functionality is moved
   to FINAL_PRESCAN_INSN, and we just set the global variables that
   it needs.  */

/* ??? This function no longer does anything useful, because final_prescan_insn
   now will never emit a nop.  */

char *
microblaze_fill_delay_slot (
  char *ret,			/* normal string to return */
  enum delay_type type,		/* type of delay */
  rtx operands[],		/* operands to use */
  rtx cur_insn)			/* current insn */
{
  register rtx set_reg;
  register enum machine_mode mode;
  register rtx next_insn = cur_insn ? NEXT_INSN (cur_insn) : NULL_RTX;
  register int num_nops;

  if (type == DELAY_LOAD || type == DELAY_FCMP)
    num_nops = 1;

  else if (type == DELAY_HILO)
    num_nops = 2;

  else
    num_nops = 0;

  /* Make sure that we don't put nop's after labels.  */
  next_insn = NEXT_INSN (cur_insn);
  while (next_insn != 0 && GET_CODE (next_insn) == NOTE)
    next_insn = NEXT_INSN (next_insn);

  dslots_load_total += num_nops;
  if (TARGET_DEBUG_F_MODE
      || !optimize
      || type == DELAY_NONE
      || operands == 0
      || cur_insn == 0
      || next_insn == 0
      || GET_CODE (next_insn) == CODE_LABEL
      || (set_reg = operands[0]) == 0)
  {
    dslots_number_nops = 0;
    microblaze_load_reg  = 0;
    microblaze_load_reg2 = 0;
    microblaze_load_reg3 = 0;
    microblaze_load_reg4 = 0;
    return ret;
  }

  set_reg = operands[0];
  if (set_reg == 0)
    return ret;

  while (GET_CODE (set_reg) == SUBREG)
    set_reg = SUBREG_REG (set_reg);

  mode = GET_MODE (set_reg);
  dslots_number_nops = num_nops;
  microblaze_load_reg = set_reg;
  if (GET_MODE_SIZE (mode)
      > (FP_REG_P (REGNO (set_reg)) ? UNITS_PER_FPREG : UNITS_PER_WORD))
    microblaze_load_reg2 = gen_rtx_REG (SImode, REGNO (set_reg) + 1);
  else
    microblaze_load_reg2 = 0;

  if (type == DELAY_HILO)
  {
    microblaze_load_reg3 = gen_rtx_REG (SImode, MD_REG_FIRST);
    microblaze_load_reg4 = gen_rtx_REG (SImode, MD_REG_FIRST+1);
  }
  else
  {
    microblaze_load_reg3 = 0;
    microblaze_load_reg4 = 0;
  }

  return ret;
}


/* Determine whether a memory reference takes one (based off of the GP
   pointer), two (normal), or three (label + reg) instructions, and bump the
   appropriate counter for -mstats.  */

void
microblaze_count_memory_refs (rtx op, int num)
{
  int additional = 0;
  int n_words = 0;
  rtx addr, plus0, plus1;
  enum rtx_code code0, code1;
  int looping;

  if (TARGET_DEBUG_B_MODE)
  {
    fprintf (stderr, "\n========== microblaze_count_memory_refs:\n");
    debug_rtx (op);
  }

  /* Skip MEM if passed, otherwise handle movsi of address.  */
  addr = (GET_CODE (op) != MEM) ? op : XEXP (op, 0);

  /* Loop, going through the address RTL.  */
  do
  {
    looping = FALSE;
    switch (GET_CODE (addr))
    {
      case REG:
      case CONST_INT:
        break;

      case PLUS:
        plus0 = XEXP (addr, 0);
        plus1 = XEXP (addr, 1);
        code0 = GET_CODE (plus0);
        code1 = GET_CODE (plus1);

        if (code0 == REG)
        {
          additional++;
          addr = plus1;
          looping = 1;
          continue;
        }

        if (code0 == CONST_INT)
        {
          addr = plus1;
          looping = 1;
          continue;
        }

        if (code1 == REG)
        {
          additional++;
          addr = plus0;
          looping = 1;
          continue;
        }

        if (code1 == CONST_INT)
        {
          addr = plus0;
          looping = 1;
          continue;
        }

        if (code0 == SYMBOL_REF || code0 == LABEL_REF || code0 == CONST)
        {
          addr = plus0;
          looping = 1;
          continue;
        }

        if (code1 == SYMBOL_REF || code1 == LABEL_REF || code1 == CONST)
        {
          addr = plus1;
          looping = 1;
          continue;
        }

        break;

      case LABEL_REF:
        n_words = 2;		/* always 2 words */
        break;

      case CONST:
        addr = XEXP (addr, 0);
        looping = 1;
        continue;

      case SYMBOL_REF:
        n_words = SYMBOL_REF_FLAG (addr) ? 1 : 2;
        break;

      default:
        break;
    }
  }
  while (looping);

  if (n_words == 0)
    return;

  n_words += additional;
  if (n_words > 3)
    n_words = 3;

  num_refs[n_words-1] += num;
}


rtx
embedded_pic_fnaddr_reg (void)
{
#if 0
  if (cfun->machine->embedded_pic_fnaddr_rtx == NULL)
  {
    rtx seq;

    cfun->machine->embedded_pic_fnaddr_rtx = gen_reg_rtx (Pmode);

    /* Output code at function start to initialize the pseudo-reg.  */
    /* ??? We used to do this in FINALIZE_PIC, but that does not work for
       inline functions, because it is called after RTL for the function
       has been copied.  The pseudo-reg in embedded_pic_fnaddr_rtx however
       does not get copied, and ends up not matching the rest of the RTL.
       This solution works, but means that we get unnecessary code to
       initialize this value every time a function is inlined into another
       function.  */
    start_sequence ();
    emit_insn (gen_get_fnaddr (cfun->machine->embedded_pic_fnaddr_rtx,
                               XEXP (DECL_RTL (current_function_decl), 0)));
    seq = get_insns ();
    end_sequence ();
    push_topmost_sequence ();
    emit_insn_after (seq, get_insns ());
    pop_topmost_sequence ();
  }

  return cfun->machine->embedded_pic_fnaddr_rtx;
#endif
  /* Returning NULL Pointer. Not a safe way to do this */
  return NULL;
}

/* Return RTL for the offset from the current function to the argument.
   X is the symbol whose offset from the current function we want.  */

/* Looks like this is not called anywhere */
rtx
embedded_pic_offset (rtx x)
{
  /* Make sure it is emitted.  */
  embedded_pic_fnaddr_reg ();

  return
    gen_rtx_CONST (Pmode,
                   gen_rtx_MINUS (Pmode, x,
                                  XEXP (DECL_RTL (current_function_decl), 0)));
}


static const char* 
microblaze_mode_to_mem_modifier (int load, enum machine_mode mode)
{
  switch (mode) {

    case QImode:
      if (load)
        return "bu";
      else
        return "b";
    case HImode:
      if (load)
        return "hu";
      else
        return "h";
    default:
      return "w";
  }
}

/* Return the appropriate instructions to move one operand to another.  */

const char *
microblaze_move_1word (
  rtx operands[],
  rtx insn,
  int unsignedp ATTRIBUTE_UNUSED)
{
  char *ret=0; 
  rtx op0 = operands[0];
  rtx op1 = operands[1];
  enum rtx_code code0 = GET_CODE (op0);
  enum rtx_code code1 = GET_CODE (op1);
  enum machine_mode mode0 = GET_MODE (op0);
#if 0
  enum machine_mode mode1 = GET_MODE (op1);
#endif
  int subreg_word0 = 0;
  int subreg_word1 = 0;
  enum delay_type delay = DELAY_NONE;
  int i;
  int empty_ret = 0;

  ret = (char*)xmalloc(100);                                                          /* Allocate 50 bytes for the assembly instruction */
   
  for(i = 0 ; i < 100; i++) ret[i]='\0';

  while (code0 == SUBREG)
  {
    subreg_word0 += subreg_regno_offset (REGNO (SUBREG_REG (op0)),
                                         GET_MODE (SUBREG_REG (op0)),
                                         SUBREG_BYTE (op0),
                                         GET_MODE (op0));
    op0 = SUBREG_REG (op0);
    code0 = GET_CODE (op0);
  }

  while (code1 == SUBREG)
  {
    subreg_word1 += subreg_regno_offset (REGNO (SUBREG_REG (op1)),
                                         GET_MODE (SUBREG_REG (op1)),
                                         SUBREG_BYTE (op1),
                                         GET_MODE (op1));
    op1 = SUBREG_REG (op1);
    code1 = GET_CODE (op1);
  }

  /* For our purposes, a condition code mode is the same as SImode.  */
  if (mode0 == CCmode)
    mode0 = SImode;

  if (code0 == REG) {
    int regno0 = REGNO (op0) + subreg_word0;

    if (code1 == REG) {
      int regno1 = REGNO (op1) + subreg_word1;

      /* Just in case, don't do anything for assigning a register
         to itself, unless we are filling a delay slot.  */
      if (regno0 == regno1 && set_nomacro == 0){
        empty_ret = 1;
	ret[0] = '\0';
      }
	  
      else if (GP_REG_P (regno0) && GP_REG_P (regno1) )
        strcpy(ret, "addk\t%0,%1,r0");
      else if (GP_REG_P (regno0) && ST_REG_P (regno1))
        strcpy(ret, "mfs\t%0,%1");
      else if (ST_REG_P (regno0) && GP_REG_P (regno1))
        strcpy(ret, "mts\t%0,%1");
    } else if (code1 == MEM) {
      rtx offset = const0_rtx;
      op1 = eliminate_constant_term (XEXP (op1, 0), &offset);
          
      delay = DELAY_NONE;

      if (TARGET_STATS)
        microblaze_count_memory_refs (op1, 1);

      if (GP_REG_P (regno0))
      {
        struct microblaze_address_info info;
        if (!microblaze_classify_address (&info, XEXP (operands[1], 0), GET_MODE (operands[1]), 1)) 
          fatal_insn ("insn contains an invalid address !", insn);

        switch (info.type) {
          case ADDRESS_REG:
            if ((GET_CODE (info.offset) == CONST_INT)) 
              sprintf (ret, "l%si\t%%0,%%1", microblaze_mode_to_mem_modifier (1, GET_MODE (operands[1])));
            break;
          case ADDRESS_REG_INDEX:
            sprintf (ret, "l%s\t%%0,%%1", microblaze_mode_to_mem_modifier (1, GET_MODE (operands[1])));
            break;
          case ADDRESS_CONST_INT:
          case ADDRESS_SYMBOLIC:
          case ADDRESS_GOTOFF:
            sprintf (ret, "l%si\t%%0,%%1", microblaze_mode_to_mem_modifier (1, GET_MODE (operands[1])));
            break;
	  case ADDRESS_INVALID:
	  case ADDRESS_PLT:
	    fatal_insn("Invalid address", operands[1]);
	    break;
        }
        return ret;
      }
    } else if ((code1 == CONST_INT) || 
               (code1 == CONST_DOUBLE && GET_MODE (op1) == VOIDmode)) {
      if (code1 == CONST_DOUBLE) {
        /* This can happen when storing constants into long long
           bitfields.  Just store the least significant word of
           the value.  */
        operands[1] = op1 = GEN_INT (CONST_DOUBLE_LOW (op1));
      }

      if (INTVAL (op1) == 0 )
      {
        if (GP_REG_P (regno0))
          strcpy(ret, "addk\t%0,r0,%z1");
      } else if (GP_REG_P (regno0))
      {
        strcpy(ret, "addik\t%0,r0,%1\t# %X1");
      }
    }
    /* Sid [07/16/01] Need to get DOUBLE and FLOATS out . Instead have either normal moves or function calls */
    else if (code1 == CONST_DOUBLE && mode0 == SFmode)
    {
      if (op1 == CONST0_RTX (SFmode))
      {
        if (GP_REG_P (regno0))
          strcpy(ret, "addk\t%0,r0,%.");
      }

      else
      {
        delay = DELAY_NONE;
        {
          unsigned int value_long;
          int i;
          REAL_VALUE_TYPE value;

          REAL_VALUE_FROM_CONST_DOUBLE(value,operands[1]);
          REAL_VALUE_TO_TARGET_SINGLE (value, value_long);
		
          ret = (char*)xmalloc(30); 
                    
          for(i = 0; i < 30; i++) 
            ret[i] = 0;

          sprintf (ret, "addik\t%%0,r0,0x%x", value_long);
        }
	      
      }
    }

    else if (code1 == LABEL_REF)
    {

#if 0
      int base_reg = get_base_reg (XEXP (operands[1], 0));
#endif
      
      if (TARGET_STATS)
        microblaze_count_memory_refs (op1, 1);
      
      ret = (char*)xmalloc (strlen ("addik\t%%0,r13,%%a1")); /* need r2 as the reg, but use r13 to get more space */
      sprintf (ret, "addik\t%%0,%%a1");
    }

    else if (code1 == SYMBOL_REF || code1 == CONST)
    {
      if ((HALF_PIC_P () && CONSTANT_P (op1) && HALF_PIC_ADDRESS_P (op1)) ||
	 flag_pic == 2)
      {
        rtx offset = const0_rtx;

        if (GET_CODE (op1) == CONST)
          op1 = eliminate_constant_term (XEXP (op1, 0), &offset);

        if (GET_CODE (op1) == SYMBOL_REF)
        {
	  if (flag_pic == 2)
          {
	    rtx temp;
            temp = gen_rtx_UNSPEC (Pmode, gen_rtvec (1, op1), UNSPEC_GOTOFF);
            temp = gen_rtx_CONST (Pmode, temp);
            temp = gen_rtx_PLUS (Pmode, pic_offset_table_rtx, temp);
            temp = gen_const_mem (Pmode, temp);
            if (TARGET_STATS)
              microblaze_count_memory_refs (temp, 1);
            if (reload_in_progress)
              regs_ever_live[PIC_OFFSET_TABLE_REGNUM] = 1;
            sprintf (ret, "l%si\t%%0,%%1", microblaze_mode_to_mem_modifier (1, GET_MODE (operands[1])));
            operands[1] = temp;
          }
          else
          {
            operands[2] = HALF_PIC_PTR (op1);

            if (TARGET_STATS)
              microblaze_count_memory_refs (operands[2], 1);

            if (INTVAL (offset) == 0)
            {
              /*		      delay = DELAY_LOAD;*/
              delay = DELAY_NONE;
              strcpy (ret, "lw\t%0,%2");
            }
            else
            {
              /* XLNX [Check out]*/
              dslots_load_total++;
              operands[3] = offset;
              strcpy (ret, "lwi\t%0,%2,0\n\tadd\t%0,%0,%3");
            }
          }
        }
      }
      else
      {
#if 0
        int base_reg = get_base_reg (operands[1]);
#endif
        sprintf (ret,"addik\t%%0,%%a1");
        
        if (TARGET_STATS)
          microblaze_count_memory_refs (op1, 1);
      }
    }

    else if (code1 == UNSPEC && XINT (op1,1) == UNSPEC_PLT)
    {
      XINT (op1,1) = UNSPEC_GOTOFF; /* rewrite into GOTOFF */
      if (GP_REG_P (regno0))
      {
        strcpy(ret, "lwi\t%0,%1");
        return ret;
      }
    }

    else if (code1 == PLUS)
    {
      rtx add_op0 = XEXP (op1, 0);
      rtx add_op1 = XEXP (op1, 1);

      if (GET_CODE (XEXP (op1, 1)) == REG
          && GET_CODE (XEXP (op1, 0)) == CONST_INT)
        add_op0 = XEXP (op1, 1), add_op1 = XEXP (op1, 0);

      operands[2] = add_op0;
      operands[3] = add_op1;
      strcpy (ret, "addk%:\t%0,%2,%3");
    }

    else if (code1 == HIGH)
    {
      operands[1] = XEXP (op1, 0);
      strcpy (ret, "lui\t%0,%%hi(%1)");
    }
  }

  else if (code0 == MEM)
  {
        
    struct microblaze_address_info info;
    if (!microblaze_classify_address (&info, XEXP (operands[0], 0), GET_MODE (operands[0]), 1)) 
      fatal_insn ("insn contains an invalid addresss !", insn);
        
    switch (info.type) {
      case ADDRESS_REG:
          sprintf (ret, "s%si\t%%z1,%%0", microblaze_mode_to_mem_modifier (0, GET_MODE (operands[0])));
        break;
      case ADDRESS_REG_INDEX:
        sprintf (ret, "s%s\t%%z1,%%0", microblaze_mode_to_mem_modifier (0, GET_MODE (operands[0])));
        break;
      case ADDRESS_CONST_INT:
      case ADDRESS_SYMBOLIC:
      case ADDRESS_GOTOFF:
        sprintf (ret, "s%si\t%%z1,%%0", microblaze_mode_to_mem_modifier (0, GET_MODE (operands[0])));
        break;
      case ADDRESS_INVALID:
      case ADDRESS_PLT:
	fatal_insn ("invalid address", insn);
	break;
    }
    return ret;
  }

  if (ret == 0)
  {
    fatal_insn ("Bad move", insn);
    return 0;
  }
  else
    if (!empty_ret && strlen(ret) == 0){
      fatal_insn ("Bad move", insn);
      return 0;
    }

  if (delay != DELAY_NONE)
    return microblaze_fill_delay_slot (ret, delay, operands, insn);

  return ret;
}


/* Return the appropriate instructions to move 2 words */

const char*
microblaze_move_2words (
  rtx operands[],
  rtx insn)
{
  static char ret[100];
  rtx op0 = operands[0];
  rtx op1 = operands[1];
  enum rtx_code code0 = GET_CODE (operands[0]);
  enum rtx_code code1 = GET_CODE (operands[1]);
  int subreg_word0 = 0;
  int subreg_word1 = 0;
  enum delay_type delay = DELAY_NONE;

  ret[0] = 0;

  while (code0 == SUBREG)
  {
    subreg_word0 += /* SUBREG_REG (op0) MJE */ 1;
    op0 = SUBREG_REG (op0);
    code0 = GET_CODE (op0);
  }

  if (code1 == SIGN_EXTEND)
  {
    op1 = XEXP (op1, 0);
    code1 = GET_CODE (op1);
  }

  while (code1 == SUBREG)
  {
    subreg_word1 += /* SUBREG_REG (op1) MJE */ 1;
    op1 = SUBREG_REG (op1);
    code1 = GET_CODE (op1);
  }
      
  /* Sanity check.  */
  if (GET_CODE (operands[1]) == SIGN_EXTEND
      && code1 != REG
      && code1 != CONST_INT
      /* The following three can happen as the result of a questionable
         cast.  */
      && code1 != LABEL_REF
      && code1 != SYMBOL_REF
      && code1 != CONST)
    abort ();

  if (code0 == REG)
  {
    int regno0 = REGNO (op0) + subreg_word0;

    if (code1 == REG)
    {
      int regno1 = REGNO (op1) + subreg_word1;

      /* Just in case, don't do anything for assigning a register
         to itself, unless we are filling a delay slot.  */
      if (regno0 == regno1 && set_nomacro == 0)
        strcpy (ret, "");
      else if (regno0 != (regno1+1))
        strcpy (ret, "addk\t%0,r0,%1\n\taddk\t%D0,r0,%D1");

      else
        strcpy (ret, "addk\t%D0,r0,%D1\n\taddk\t%0,r0,%1");
    }

    else if (code1 == CONST_DOUBLE)
    {
      /* Move zero from $0 unless recipient
         is 64-bit fp reg, in which case generate a constant.  */
      if (op1 != CONST0_RTX (GET_MODE (op1)))
      {
        if (GET_MODE (op1) == DFmode)
        {
          /*		  delay = DELAY_LOAD;*/
          delay = DELAY_NONE;

#ifdef TARGET_FP_CALL_32
          if (FP_CALL_GP_REG_P (regno0))
          {
            strcpy (ret, "li.d\t%0,%1\n\tdsll\t%D0,%0,32\n\tdsrl\t%D0,32\n\tdsrl\t%0,32");
          }
          else
#endif
          {
            long int values[2];
            REAL_VALUE_TYPE value;
            REAL_VALUE_FROM_CONST_DOUBLE(value,operands[1]);
            REAL_VALUE_TO_TARGET_DOUBLE (value, values);

	
            sprintf (ret, "addik\t%%0,r0,0x%lx \n\taddik\t%%D0,r0,0x%lx #Xfer Lo", 
                     values[0], values[1]);
            printed = 1;
          }
        }

        else
        {
          split_double (op1, operands + 2, operands + 3);
          /*		  strcpy (ret, "MICROBLAZEli\t%0,%2\n\tMICROBLAZEli\t%D0,%3 #li1");*/
          /*   	          fprintf(stderr,"li ==> la\n");*/
          strcpy (ret, "addik\t%0,r0,%2\n\taddik\t%D0,r0,%3 #li => la");
        }
      }

      else
      {
        if (GP_REG_P (regno0))
          strcpy (ret, "addk\t%0,r0,%.\n\taddk\t%D0,r0,%.");
      }
    }

    else if (code1 == CONST_INT && INTVAL (op1) == 0 )
    {
      if (GP_REG_P (regno0))
        strcpy (ret, "addk\t%0,r0,%.\n\taddk\t%D0,r0,%.");
    }
	
    else if (code1 == CONST_INT && GET_MODE (op0) == DImode
             && GP_REG_P (regno0)){
      if (HOST_BITS_PER_WIDE_INT < 64) {
        operands[2] = GEN_INT (INTVAL (operands[1]) >= 0 ? 0 : -1);
        /*	      strcpy (ret, "MICROBLAZEli\t%M0,%2\n\tMICROBLAZEli\t%L0,%1 #li7");*/
        strcpy (ret, "addik\t%M0,r0,%2\n\taddik\t%L0,r0,%1");
      }
      else {
        /* We use multiple shifts here, to avoid warnings about out
           of range shifts on 32 bit hosts.  */
        operands[2] = GEN_INT (INTVAL (operands[1]) >> 16 >> 16);
        operands[1]
          = GEN_INT (INTVAL (operands[1]) << 16 << 16 >> 16 >> 16);
        /*strcpy (ret, "MICROBLAZEli\t%M0,%2\n\tMICROBLAZEli\t%L0,%1 #li8");*/
        strcpy (ret, "addik\t%M0,r0,%2\n\taddik\t%L0,r0,%1");
      }
    }

    else if (code1 == MEM){
      /*	  delay = DELAY_LOAD;*/
      delay = DELAY_NONE;

      if (TARGET_STATS)
        microblaze_count_memory_refs (op1, 2);

      if (double_memory_operand (op1, GET_MODE (op1))) {
        operands[2] = adjust_address (op1, GET_MODE(op1),4);
            
        /* if operands[1] is REG or op1 = MEM, which points to REG */
        if (GET_CODE(op1) == REG || GET_CODE (XEXP (op1,0)) == REG)
          strcpy (ret, (reg_mentioned_p (op0, op1)
                 ? "lwi\t%D0,%2\n\tlwi\t%0,%1"
                 : "lwi\t%0,%1\n\tlwi\t%D0,%2"));
        else
          if (GET_CODE(op1) == SYMBOL_REF || GET_CODE (XEXP (op1, 0)) == SYMBOL_REF) {
            int ret_reg;
            if (GET_CODE(op1) == SYMBOL_REF)
              ret_reg = get_base_reg(op1);
            else
              ret_reg = get_base_reg (XEXP (op1,0));
            
            if (reg_mentioned_p (op0, op1)){
              sprintf (ret,"lwi\t%%D0,%%2\n\tlwi\t%%0,%%1");
            }
            else
              sprintf (ret,"lwi\t%%0,%%1\n\tlwi\t%%D0,%%2 #MICROBLAZE-CHECK");
          }
          else if (GET_CODE(op1) == CONST || GET_CODE (XEXP (op1,0)) == CONST || GET_CODE (XEXP (op1, 0)) == CONST_INT) {
            strcpy (ret, (reg_mentioned_p (op0, op1)
                   ? "lwi\t%D0,%2\n\tlwi\t%0,%1"
                   : "lwi\t%0,%1\n\tlwi\t%D0,%2"));
          }
          else {
            strcpy (ret, (reg_mentioned_p (op0, op1)
                   ? "lwi\t%D0,%2\n\tlwi\t%0,%1"
                   : "lwi\t%0,%1\n\tlwi\t%D0,%2"));
          }
      }
    }
      
    else if (code1 == LABEL_REF)
    {
      if (TARGET_STATS)
        microblaze_count_memory_refs (op1, 2);
    }
    else if (code1 == SYMBOL_REF || code1 == CONST)
    {
      if (TARGET_STATS)
        microblaze_count_memory_refs (op1, 2);
    }
  }

  else if (code0 == MEM)
  {
    if (code1 == REG)
    {
      int regno1 = REGNO (op1) + subreg_word1;

      if (FP_REG_P (regno1))
        strcpy (ret, "s.d\t%1,%0");

      else if (double_memory_operand (op0, GET_MODE (op0)))
      {
#if 0
        int op0_base_reg = get_base_reg(op0);
#endif
        operands[2] = adjust_address(op0,GET_MODE (op0), 4);
	      
        /* if operands[0] happens to be plus, we shdn't add r0 to the resulting output */
        /* Check this code properly..Seems to be complicated */
        if (GET_CODE (XEXP (operands[0], 0)) == PLUS) 
          if (GET_CODE (XEXP (operands[2], 0)) == PLUS)
            strcpy (ret, "swi\t%1,%0\n\tswi\t%D1,%2");
          else
            strcpy (ret, "swi\t%1,%0\n\tswi\t%D1,%2");
        else
          if ((GET_CODE (XEXP (operands[0], 0)) == SYMBOL_REF) ||
              (GET_CODE (XEXP (operands[0], 0)) == CONST)) {
            sprintf (ret, "swi\t%%1,%%0\n\tswi\t%%D1,%%2");
          }
          else
            if (GET_CODE (XEXP (operands[2], 0)) == PLUS)
              strcpy (ret, "swi\t%1,%0\n\tswi\t%D1,%2");
            else
              strcpy (ret, "swi\t%1,%0\n\tswi\t%D1,%2");

      }
    }

    else if (((code1 == CONST_INT && INTVAL (op1) == 0)
              || (code1 == CONST_DOUBLE
                  && op1 == CONST0_RTX (GET_MODE (op1))))
             && (double_memory_operand (op0, GET_MODE (op0))))
    {
      operands[2] = adjust_address(op0, GET_MODE(op0),4);
      /*         operands[2] = adj_offsettable_operand (op0, 4);*/
      strcpy (ret, "swi\t%.,%0\n\tswi\t%.,%2");
    }

    if (TARGET_STATS)
      microblaze_count_memory_refs (op0, 2);
  }

  if (ret[0] == 0)
  {
    fatal_insn ("Bad move", insn);
    return 0;
  }

  if (delay != DELAY_NONE)
    return microblaze_fill_delay_slot (ret, delay, operands, insn);

  return ret;
}

/* Block Moves */

#define MAX_MOVE_REGS 8
#define MAX_MOVE_BYTES (MAX_MOVE_REGS * UNITS_PER_WORD)

/* Emit straight-line code to move LENGTH bytes from SRC to DEST.
   Assume that the areas do not overlap.  */

static void
microblaze_block_move_straight (rtx dest, rtx src, HOST_WIDE_INT length)
{
  HOST_WIDE_INT offset, delta;
  unsigned HOST_WIDE_INT bits;
  int i;
  enum machine_mode mode;
  rtx *regs;

  bits = BITS_PER_WORD;
  mode = mode_for_size (bits, MODE_INT, 0);
  delta = bits / BITS_PER_UNIT;

  /* Allocate a buffer for the temporary registers.  */
  regs = alloca (sizeof (rtx) * length / delta);

  /* Load as many BITS-sized chunks as possible.  Use a normal load if
     the source has enough alignment, otherwise use left/right pairs.  */
  for (offset = 0, i = 0; offset + delta <= length; offset += delta, i++)
    {
      regs[i] = gen_reg_rtx (mode);
      emit_move_insn (regs[i], adjust_address (src, mode, offset));
    }

  /* Copy the chunks to the destination.  */
  for (offset = 0, i = 0; offset + delta <= length; offset += delta, i++)
    emit_move_insn (adjust_address (dest, mode, offset), regs[i]);

  /* Mop up any left-over bytes.  */
  if (offset < length)
    {
      src = adjust_address (src, BLKmode, offset);
      dest = adjust_address (dest, BLKmode, offset);
      move_by_pieces (dest, src, length - offset,
		      MIN (MEM_ALIGN (src), MEM_ALIGN (dest)), 0);
    }
}

/* Helper function for doing a loop-based block operation on memory
   reference MEM.  Each iteration of the loop will operate on LENGTH
   bytes of MEM.

   Create a new base register for use within the loop and point it to
   the start of MEM.  Create a new memory reference that uses this
   register.  Store them in *LOOP_REG and *LOOP_MEM respectively.  */

static void
microblaze_adjust_block_mem (rtx mem, HOST_WIDE_INT length,
                             rtx *loop_reg, rtx *loop_mem)
{
  *loop_reg = copy_addr_to_reg (XEXP (mem, 0));

  /* Although the new mem does not refer to a known location,
     it does keep up to LENGTH bytes of alignment.  */
  *loop_mem = change_address (mem, BLKmode, *loop_reg);
  set_mem_align (*loop_mem, MIN (MEM_ALIGN (mem), length * BITS_PER_UNIT));
}


/* Move LENGTH bytes from SRC to DEST using a loop that moves MAX_MOVE_BYTES
   per iteration.  LENGTH must be at least MAX_MOVE_BYTES.  Assume that the
   memory regions do not overlap.  */

static void
microblaze_block_move_loop (rtx dest, rtx src, HOST_WIDE_INT length)
{
  rtx label, src_reg, dest_reg, final_src;
  HOST_WIDE_INT leftover;

  leftover = length % MAX_MOVE_BYTES;
  length -= leftover;

  /* Create registers and memory references for use within the loop.  */
  microblaze_adjust_block_mem (src, MAX_MOVE_BYTES, &src_reg, &src);
  microblaze_adjust_block_mem (dest, MAX_MOVE_BYTES, &dest_reg, &dest);

  /* Calculate the value that SRC_REG should have after the last iteration
     of the loop.  */
  final_src = expand_simple_binop (Pmode, PLUS, src_reg, GEN_INT (length),
				   0, 0, OPTAB_WIDEN);

  /* Emit the start of the loop.  */
  label = gen_label_rtx ();
  emit_label (label);

  /* Emit the loop body.  */
  microblaze_block_move_straight (dest, src, MAX_MOVE_BYTES);

  /* Move on to the next block.  */
  emit_move_insn (src_reg, plus_constant (src_reg, MAX_MOVE_BYTES));
  emit_move_insn (dest_reg, plus_constant (dest_reg, MAX_MOVE_BYTES));

  /* Emit the loop condition.  */
  emit_insn (gen_cmpsi (src_reg, final_src));
  emit_jump_insn (gen_bne (label));

  /* Mop up any left-over bytes.  */
  if (leftover)
    microblaze_block_move_straight (dest, src, leftover);
}

/* Expand a movmemsi instruction.  */

bool
microblaze_expand_block_move (rtx dest, rtx src, rtx length, rtx align_rtx)
{
    
  if (GET_CODE (length) == CONST_INT)
    {
      HOST_WIDE_INT bytes = INTVAL (length);
      int align           = INTVAL (align_rtx);

      if (align > UNITS_PER_WORD) 
        {
          align = UNITS_PER_WORD;       /* We can't do any better */
        }
      else if (align < UNITS_PER_WORD) 
        {
            if (INTVAL (length) <= MAX_MOVE_BYTES) 
             {
                move_by_pieces (dest, src, bytes, align, 0); 
                return true;
             }
            else
                return false;
        }

      if (INTVAL (length) <= 2 * MAX_MOVE_BYTES)
	{
	  microblaze_block_move_straight (dest, src, INTVAL (length));
	  return true;
	}
      else if (optimize)
	{
	  microblaze_block_move_loop (dest, src, INTVAL (length));
	  return true;
	}
    }
  return false;
}

bool 
microblaze_rtx_costs (
  rtx x,
  int code,
  int outer_code ATTRIBUTE_UNUSED,
  int *total)
{
  enum machine_mode mode = GET_MODE (x);

  switch (code) {
    case MEM:							
    {									
      int num_words = (GET_MODE_SIZE (mode) > UNITS_PER_WORD) ? 2 : 1; 
      if (simple_memory_operand (x, mode)) 
        *total = COSTS_N_INSNS (2 * num_words);			
      else
        *total = COSTS_N_INSNS (2 * (2 * num_words));				
        
      return true;
    }									       
    case NOT: 
    {
      if (mode == DImode) {
        *total = COSTS_N_INSNS (2);
      } else 
        *total = COSTS_N_INSNS (1);
      return false;
    }   
    case AND:								
    case IOR:								
    case XOR:								
    {
      if (mode == DImode) {
        *total = COSTS_N_INSNS (2);						
      } else 
        *total = COSTS_N_INSNS (1);

      return false;
    }
    case ASHIFT:								
    case ASHIFTRT:							
    case LSHIFTRT:	
    {
      if (TARGET_BARREL_SHIFT) {
        if (microblaze_version_compare (microblaze_select_cpu, "v5.00.a") >= 0)
          *total = COSTS_N_INSNS (1);                                         
        else
          *total = COSTS_N_INSNS (2);
      }
      else if (GET_CODE (XEXP (x, 1)) == CONST_INT) 
        *total = COSTS_N_INSNS (INTVAL (XEXP (x, 1)));
      else 
        *total = COSTS_N_INSNS (32 * 4);                /* Double the worst cost of shifts when there is no barrel shifter and the shift amount is in a reg */
      return true;
    }
    case PLUS:								
    case MINUS:								
    {									
      if (mode == SFmode || mode == DFmode)				
      {						
        if (TARGET_HARD_FLOAT)
          *total = COSTS_N_INSNS (6);					
	return true;
      } 
      else if (mode == DImode)
      {
        *total = COSTS_N_INSNS (4);					
	return true;
      } 
      else
      {
        *total = COSTS_N_INSNS (1);
	return true;
      }

      return false;
    }									
    case NEG:								
    {
      if (mode == DImode) 
        *total = COSTS_N_INSNS (4);

      return false;
    }
    case MULT:								
    {									
      if (mode == SFmode) {
        if (TARGET_HARD_FLOAT)
          *total = COSTS_N_INSNS (6);					
      }
      else if (!TARGET_SOFT_MUL) {                                                       if (microblaze_version_compare (microblaze_select_cpu, "v5.00.a") >= 0)
         *total = COSTS_N_INSNS (1);
      else
          *total = COSTS_N_INSNS (3);
      }
      else *total =  COSTS_N_INSNS (10);
      return true;
    }									
    case DIV:								
    case UDIV:								
    {									
      if (mode == SFmode)						
      {			
        if (TARGET_HARD_FLOAT)
          *total = COSTS_N_INSNS (23);					
      }								
      return false;
    }									
    case SIGN_EXTEND:							
    {
      *total = COSTS_N_INSNS (1);						
      return false;
    }
    case ZERO_EXTEND:							
    {
      *total = COSTS_N_INSNS (1);
      return false;
    }
  }
    
  return false;
}



/* Return the number of instructions needed to load or store a value
   of mode MODE at X.  Return 0 if X isn't valid for MODE.

*/

int
microblaze_address_insns (rtx x, enum machine_mode mode)
{
  struct microblaze_address_info addr;

  if (microblaze_classify_address (&addr, x, mode, false)) {
    switch (addr.type)
    {
      case ADDRESS_REG:
        if (SMALL_INT (addr.offset)) 
          return 1;
        else
          return 2;
      case ADDRESS_CONST_INT:
        if (SMALL_INT (x))
          return 1;
        else
          return 2;
      case ADDRESS_REG_INDEX:
      case ADDRESS_SYMBOLIC:
        return 1;
      case ADDRESS_GOTOFF:
        return 2;
      default:
        break;
    }
  }
  return 0;
}

/* Provide the costs of an addressing mode that contains ADDR.
   If ADDR is not a valid address, its cost is irrelevant.  */
int
microblaze_address_cost (rtx addr)
{
  return COSTS_N_INSNS (microblaze_address_insns (addr, GET_MODE (addr)));
}

/* Return nonzero if X is an address which needs a temporary register when 
   reloaded while generating PIC code.  */

/* XLNX [08/16/01] Need to look into this*/
int
pic_address_needs_scratch (rtx x)
{
  /* An address which is a symbolic plus a non SMALL_INT needs a temp reg.  */
  if (GET_CODE (x) == CONST && GET_CODE (XEXP (x, 0)) == PLUS
      && GET_CODE (XEXP (XEXP (x, 0), 0)) == SYMBOL_REF
      && GET_CODE (XEXP (XEXP (x, 0), 1)) == CONST_INT
      && (flag_pic == 2 || ! SMALL_INT (XEXP (XEXP (x, 0), 1))))
    return 1;

  return 0;
}

/* Make normal rtx_code into something we can index from an array */

#if 0
static enum internal_test
map_test_to_internal_test (enum rtx_code test_code)
{
  enum internal_test test = ITEST_MAX;

  switch (test_code)
  {
    case EQ:  test = ITEST_EQ;  break;
    case NE:  test = ITEST_NE;  break;
    case GT:  test = ITEST_GT;  break;
    case GE:  test = ITEST_GE;  break;
    case LT:  test = ITEST_LT;  break;
    case LE:  test = ITEST_LE;  break;
    case GTU: test = ITEST_GTU; break;
    case GEU: test = ITEST_GEU; break;
    case LTU: test = ITEST_LTU; break;
    case LEU: test = ITEST_LEU; break;
    default:			break;
  }

  return test;
}
#endif

/* Emit the common code for doing conditional branches.
   operand[0] is the label to jump to.
   The comparison operands are saved away by cmp{si,di,sf,df}.  */

void
gen_conditional_branch (rtx operands[], enum rtx_code test_code)
{
  enum cmp_type type = branch_type;
  rtx cmp0 = branch_cmp[0];
  rtx cmp1 = branch_cmp[1];
  enum machine_mode mode;
#if 0
  rtx reg0, reg1;
#endif
  rtx reg0;
  rtx label1, label2;

  switch (type)
  {
    case CMP_SI:
    case CMP_DI:
      mode = type == CMP_SI ? SImode : DImode; 
      break;
   case CMP_SF:
      if (TARGET_HARD_FLOAT) {
        reg0 = gen_reg_rtx (SImode);
        /* For cmp0 != cmp1, build cmp0 == cmp1, and test for result == 0 in the following instruction. */
        emit_insn (gen_rtx_SET (VOIDmode, reg0,
                            gen_rtx_fmt_ee ((test_code == NE ? EQ : test_code), SImode, cmp0, cmp1)));

        /* Setup test and branch for following instruction
           Setup a test for zero as opposed to non-zero.
           This is more optimally implemented. */
        test_code = (test_code == NE) ? EQ : NE;
        mode = SImode;
        cmp0 = reg0;
        cmp1 = const0_rtx;
        break;
      } else
        fatal_insn ("gen_conditional_branch:", gen_rtx_fmt_ee (test_code, VOIDmode, cmp0, cmp1));
    default:
      fatal_insn ("gen_conditional_branch:", gen_rtx_fmt_ee (test_code, VOIDmode, cmp0, cmp1));
  }

  /* Generate the branch.  */

  label1 = gen_rtx_LABEL_REF (VOIDmode, operands[0]);
  label2 = pc_rtx;

  if (!(GET_CODE (cmp1) == CONST_INT && INTVAL (cmp1) == 0)) {          /* Except for branch_zero */
    emit_jump_insn (
      gen_rtx_PARALLEL (VOIDmode, 
	gen_rtvec (2,
          gen_rtx_SET (VOIDmode, pc_rtx, 
	    gen_rtx_IF_THEN_ELSE (VOIDmode, 
	      gen_rtx_fmt_ee (test_code, mode, cmp0, cmp1),
	      label1, label2)),
          gen_rtx_CLOBBER (VOIDmode, gen_rtx_REG (SImode, MB_ABI_ASM_TEMP_REGNUM)))));
  } else 
    emit_jump_insn (
      gen_rtx_SET (VOIDmode, pc_rtx, 
        gen_rtx_IF_THEN_ELSE (VOIDmode, 
	  gen_rtx_fmt_ee (test_code, mode, cmp0, cmp1),
	  label1, label2)));
}

/* Argument support functions.  */

/* Initialize CUMULATIVE_ARGS for a function.  */

void
init_cumulative_args (
  CUMULATIVE_ARGS *cum,		/* argument info to initialize */
  tree fntype,			/* tree ptr for function decl */
  rtx libname ATTRIBUTE_UNUSED)	/* SYMBOL_REF of library name or 0 */
{
  static CUMULATIVE_ARGS zero_cum;
  tree param, next_param;

  if (TARGET_DEBUG_E_MODE)
  {
    fprintf (stderr,
             "\ninit_cumulative_args, fntype = 0x%.8lx", (long)fntype);

    if (!fntype)
      fputc ('\n', stderr);

    else
    {
      tree ret_type = TREE_TYPE (fntype);
      fprintf (stderr, ", fntype code = %s, ret code = %s\n",
               tree_code_name[(int)TREE_CODE (fntype)],
               tree_code_name[(int)TREE_CODE (ret_type)]);
    }
  }

  *cum = zero_cum;

  /* Determine if this function has variable arguments.  This is
     indicated by the last argument being 'void_type_mode' if there
     are no variable arguments.  The standard MICROBLAZE calling sequence
     passes all arguments in the general purpose registers in this case. */

  for (param = fntype ? TYPE_ARG_TYPES (fntype) : 0;
       param != 0; param = next_param)
  {
    next_param = TREE_CHAIN (param);
    if (next_param == 0 && TREE_VALUE (param) != void_type_node)
      cum->gp_reg_found = 1;
  }
}

/* Advance the argument to the next argument position.  */

void
function_arg_advance (
  CUMULATIVE_ARGS *cum,		/* current arg information */
  enum machine_mode mode,	/* current arg mode */
  tree type,			/* type of the argument or 0 if lib support */
  int named)			/* whether or not the argument was named */
{
  if (TARGET_DEBUG_E_MODE)
  {
    fprintf (stderr,
             "function_adv({gp reg found = %d, arg # = %2d, words = %2d}, %4s, ",
             cum->gp_reg_found, cum->arg_number, cum->arg_words,
             GET_MODE_NAME (mode));
    fprintf (stderr, "%p", (void *)type);
    fprintf (stderr, ", %d )\n\n", named);
  }

  cum->arg_number++;
  switch (mode)
  {
    case VOIDmode:
      break;

    default:
      if (GET_MODE_CLASS (mode) != MODE_COMPLEX_INT
          && GET_MODE_CLASS (mode) != MODE_COMPLEX_FLOAT)
        abort ();

      cum->gp_reg_found = 1;
      cum->arg_words += ((GET_MODE_SIZE (mode) + UNITS_PER_WORD - 1)
                         / UNITS_PER_WORD);
      break;

    case BLKmode:
      cum->gp_reg_found = 1;
      cum->arg_words += ((int_size_in_bytes (type) + UNITS_PER_WORD - 1)
                         / UNITS_PER_WORD);
      break;

    case SFmode:
      cum->arg_words++;
      if (! cum->gp_reg_found && cum->arg_number <= 2)
        cum->fp_code += 1 << ((cum->arg_number - 1) * 2);
      break;

    case DFmode:
      cum->arg_words += 2;
      if (! cum->gp_reg_found && ! TARGET_SINGLE_FLOAT && cum->arg_number <= 2)
        cum->fp_code += 2 << ((cum->arg_number - 1) * 2);
      break;

    case DImode:
      cum->gp_reg_found = 1;
      cum->arg_words += 2;
      break;

    case QImode:
    case HImode:
    case SImode:
      cum->gp_reg_found = 1;
      cum->arg_words++;
      break;
  }
}

/* Return an RTL expression containing the register for the given mode,
   or 0 if the argument is to be passed on the stack.  */

rtx
function_arg (
  CUMULATIVE_ARGS *cum,		/* current arg information */
  enum machine_mode mode,	/* current arg mode */
  tree type,			/* type of the argument or 0 if lib support */
  int named)			/* != 0 for normal args, == 0 for ... args */
{
  rtx ret;
  int regbase = -1;
  int *arg_words = &cum->arg_words;
  int struct_p = (type != 0
                  && (TREE_CODE (type) == RECORD_TYPE
                      || TREE_CODE (type) == UNION_TYPE
                      || TREE_CODE (type) == QUAL_UNION_TYPE));

  if (TARGET_DEBUG_E_MODE)
  {
    fprintf (stderr,
             "function_arg( {gp reg found = %d, arg # = %2d, words = %2d}, %4s, ",
             cum->gp_reg_found, cum->arg_number, cum->arg_words,
             GET_MODE_NAME (mode));
    fprintf (stderr, "%p", (void *)type);
    fprintf (stderr, ", %d ) = ", named);
  }
  
  cum->last_arg_fp = 0;
  switch (mode)
  {
    case SFmode:
    case DFmode:
    case VOIDmode:
    case QImode:
    case HImode:
    case SImode:
    case DImode:
      regbase = GP_ARG_FIRST;
      break;
    default:
      if (GET_MODE_CLASS (mode) != MODE_COMPLEX_INT
          && GET_MODE_CLASS (mode) != MODE_COMPLEX_FLOAT)
        abort ();
      /* Drops through.  */
    case BLKmode:
      regbase = GP_ARG_FIRST;
      break;
  }

  if (*arg_words >= MAX_ARGS_IN_REGISTERS)
  {
    if (TARGET_DEBUG_E_MODE)
      fprintf (stderr, "<stack>%s\n", struct_p ? ", [struct]" : "");

    ret = 0;
  }
  else
  {
    if (regbase == -1)
      abort ();

    ret = gen_rtx_REG (mode, regbase + *arg_words);

    if (TARGET_DEBUG_E_MODE)
      fprintf (stderr, "%s%s\n", reg_names[regbase + *arg_words],
               struct_p ? ", [struct]" : "");

    /* The following is a hack in order to pass 1 byte structures
       the same way that the MICROBLAZE compiler does (namely by passing
       the structure in the high byte or half word of the register).
       This also makes varargs work.  If we have such a structure,
       we save the adjustment RTL, and the call define expands will
       emit them.  For the VOIDmode argument (argument after the
       last real argument), pass back a parallel vector holding each
       of the adjustments.  */

    /* ??? function_arg can be called more than once for each argument.
       As a result, we compute more adjustments than we need here.
       See the CUMULATIVE_ARGS definition in microblaze.h.  */

    /* ??? This scheme requires everything smaller than the word size to
       shifted to the left
    */

    /* XLNX [We might not need to do this for MicroBlaze . Seems to cause a problem 
       while passing small structs */
#if 0
    if (struct_p && int_size_in_bytes (type) < UNITS_PER_WORD)
    {
      rtx amount = GEN_INT (BITS_PER_WORD
                            - int_size_in_bytes (type) * BITS_PER_UNIT);
      rtx reg = gen_rtx_REG (word_mode, regbase + *arg_words);

	  
      cum->adjust[cum->num_adjusts++] = gen_ashlsi3 (reg, reg, amount);
    }
#endif
      }

  if (mode == VOIDmode)
  {
    if (cum->num_adjusts > 0)
      ret = gen_rtx_PARALLEL ((enum machine_mode) cum->fp_code,
                     gen_rtvec_v (cum->num_adjusts, cum->adjust));
  }

  return ret;
}

/* Return number of bytes of argument to put in registers. */
int
function_arg_partial_bytes (
  CUMULATIVE_ARGS *cum,		/* current arg information */
  enum machine_mode mode,	/* current arg mode */
  tree type,			/* type of the argument or 0 if lib support */
  int named ATTRIBUTE_UNUSED)   /* != 0 for normal args, == 0 for ... args */
{
  if ((mode == BLKmode
       || GET_MODE_CLASS (mode) != MODE_COMPLEX_INT
       || GET_MODE_CLASS (mode) != MODE_COMPLEX_FLOAT)
      && cum->arg_words < MAX_ARGS_IN_REGISTERS)
  {
    int words;
    if (mode == BLKmode)
      words = ((int_size_in_bytes (type) + UNITS_PER_WORD - 1)
               / UNITS_PER_WORD);
    else
      words = (GET_MODE_SIZE (mode) + UNITS_PER_WORD - 1) / UNITS_PER_WORD;

    if (words + cum->arg_words <= MAX_ARGS_IN_REGISTERS)
      return 0;		/* structure fits in registers */

    if (TARGET_DEBUG_E_MODE)
      fprintf (stderr, "function_arg_partial_nregs = %d\n",
               MAX_ARGS_IN_REGISTERS - cum->arg_words);

    return (MAX_ARGS_IN_REGISTERS - cum->arg_words) * UNITS_PER_WORD;
  }

  else if (mode == DImode && cum->arg_words == MAX_ARGS_IN_REGISTERS-1)
  {
    if (TARGET_DEBUG_E_MODE)
      fprintf (stderr, "function_arg_partial_nregs = 1\n");
        
    return UNITS_PER_WORD;
  }
    
  return 0;
}

/*  Convert a version number of the form "vX.YY.Z" to an integer encoding 
    for easier range comparison */
static int 
microblaze_version_to_int (const char *version)
{
  const char *p, *v;
  const char *tmpl = "vX.YY.Z";
  int iver = 0;
#if 0
  int pos = 0;
#endif
  
  iver = 0;
  
  p = (char *)version;
  v = tmpl;
  
  while (*v) {
    if (*v == 'X') {            /* Looking for major */
      if (!(*p >= '0' && *p <= '9'))
        return -1;
      iver += (int)(*p - '0');
      iver *= 10;
    } else if (*v == 'Y') {     /* Looking for minor */
      if (!(*p >= '0' && *p <= '9'))
        return -1;
      iver += (int)(*p - '0');
      iver *= 10;
    } else if (*v == 'Z') {     /* Looking for compat */
      if (!(*p >= 'a' && *p <= 'z'))
        return -1;
      iver *= 10;
      iver += (int)(*p - 'a');
    } else {
      if (*p != *v)
        return -1;
    }
        
    v++;
    p++;
  }
    
  if (*p)
    return -1;

  return iver;
}

/* Compare two given microblaze versions and return a verdict
 */
static int
microblaze_version_compare (const char *va, const char *vb) 
{
  return strcasecmp (va, vb);
}

/* Set up the threshold for data to go into the small data area, instead
   of the normal data area, and detect any conflicts in the switches.  */

static bool
microblaze_handle_option (size_t code, 
			  const char *arg ATTRIBUTE_UNUSED, 
		          int value ATTRIBUTE_UNUSED)
{
  switch (code)
    {
    /* Check if we are asked to not clear BSS 
       If YES, we do not place zero initialized in BSS  */
    case OPT_mno_clearbss:
      flag_zero_initialized_in_bss = 0;
      break;
    }
  return true;
}


void
override_options (void)
{
  register int i, start;
  register int regno;
  register enum machine_mode mode;
  int ver;


  microblaze_section_threshold = g_switch_set ? g_switch_value : MICROBLAZE_DEFAULT_GVALUE;

  /* Check the Microblaze CPU version for any special action that needs to be done */
  if (microblaze_select_cpu == NULL) 
    microblaze_select_cpu = MICROBLAZE_DEFAULT_CPU;
  ver = microblaze_version_to_int (microblaze_select_cpu);
  if (ver == -1) {
    error ("(%s) is an invalid argument to -mcpu=\n", microblaze_select_cpu);
  }

  ver = microblaze_version_compare (microblaze_select_cpu, "v3.00.a");
  if (ver < 0) {                                                        /* No hardware exceptions in earlier versions. So no worries */
    microblaze_select_flags &= ~(MICROBLAZE_MASK_NO_UNSAFE_DELAY);
    microblaze_no_unsafe_delay = 0;
    microblaze_pipe = MICROBLAZE_PIPE_3;
  } else if (ver == 0 || (microblaze_version_compare (microblaze_select_cpu, "v4.00.b") == 0)) {
    microblaze_select_flags |= (MICROBLAZE_MASK_NO_UNSAFE_DELAY);
    microblaze_no_unsafe_delay = 1;
    microblaze_pipe = MICROBLAZE_PIPE_3;
  } else {
    microblaze_select_flags &= ~(MICROBLAZE_MASK_NO_UNSAFE_DELAY);
    microblaze_no_unsafe_delay = 0;
    microblaze_pipe = MICROBLAZE_PIPE_5;                                /* We agree to use 5 pipe-stage model even on area optimized 3 pipe-stage variants. */
    if (microblaze_version_compare (microblaze_select_cpu, "v5.00.a") == 0 ||
        microblaze_version_compare (microblaze_select_cpu, "v5.00.b") == 0 ||
        microblaze_version_compare (microblaze_select_cpu, "v5.00.c") == 0) {
        target_flags |= MASK_PATTERN_COMPARE;                           /* Pattern compares are to be turned on by default only when compiling for MB v5.00.'z' */
    }
  }

  ver = microblaze_version_compare (microblaze_select_cpu, "v6.00.a");
  if (ver < 0) {
      if (TARGET_MULTIPLY_HIGH)
          warning (0, "-mxl-multiply-high can be used only with -mcpu=v6.00.a or greater.\n");
  }

  if (TARGET_MULTIPLY_HIGH && TARGET_SOFT_MUL)
      error ("-mxl-multiply-high requires -mno-xl-soft-mul.\n");

  /* Always use DFA scheduler */
  microblaze_sched_use_dfa = 1;
  
  /* Tell halfpic.c that we have half-pic code if we do.  */
  if (TARGET_HALF_PIC)
    HALF_PIC_INIT ();

  microblaze_abicalls = MICROBLAZE_ABICALLS_NO;
  /* printf("microblaze_abi %d microblaze_abicalls %d\n",
     microblaze_abi, microblaze_abicalls); */
  /* -membedded-pic is a form of PIC code suitable for embedded
     systems.  All calls are made using PC relative addressing, and
     all data is addressed using the $gp register.  This requires gas,
     which does most of the work, and GNU ld, which automatically
     expands PC relative calls which are out of range into a longer
     instruction sequence.  All gcc really does differently is
     generate a different sequence for a switch.  */
  if (TARGET_EMBEDDED_PIC)
  {
    flag_pic = 1;

    if (g_switch_set)
      warning (0, "-G and -membedded-pic are incompatible");

    /* Setting microblaze_section_threshold is not required, because gas
       will force everything to be GP addressable anyhow, but
       setting it will cause gcc to make better estimates of the
       number of instructions required to access a particular data
       item.  */
    microblaze_section_threshold = 0x7fffffff;
  }

  /* Initialize the high and low values for legitimate floating point
     constants.  Rather than trying to get the accuracy down to the
     last bit, just use approximate ranges.  */
  dfhigh = REAL_VALUE_ATOF ("1.0e300", DFmode);
  dflow = REAL_VALUE_ATOF ("1.0e-300", DFmode);
  sfhigh = REAL_VALUE_ATOF ("1.0e38", SFmode);
  sflow = REAL_VALUE_ATOF ("1.0e-38", SFmode);

  microblaze_print_operand_punct['?'] = 1;
  microblaze_print_operand_punct['#'] = 1;
  microblaze_print_operand_punct['&'] = 1;
  microblaze_print_operand_punct['!'] = 1;
  microblaze_print_operand_punct['*'] = 1;
  microblaze_print_operand_punct['@'] = 1;
  microblaze_print_operand_punct['.'] = 1;
  microblaze_print_operand_punct['('] = 1;
  microblaze_print_operand_punct[')'] = 1;
  microblaze_print_operand_punct['['] = 1;
  microblaze_print_operand_punct[']'] = 1;
  microblaze_print_operand_punct['<'] = 1;
  microblaze_print_operand_punct['>'] = 1;
  microblaze_print_operand_punct['{'] = 1;
  microblaze_print_operand_punct['}'] = 1;
  microblaze_print_operand_punct['^'] = 1;
  microblaze_print_operand_punct['$'] = 1;
  microblaze_print_operand_punct['+'] = 1;

  microblaze_char_to_class['d'] = GR_REGS;
  microblaze_char_to_class['f'] = NO_REGS;
  microblaze_char_to_class['h'] = HI_REG;
  microblaze_char_to_class['l'] = LO_REG;
  microblaze_char_to_class['a'] = HILO_REG;
  microblaze_char_to_class['x'] = MD_REGS;
  microblaze_char_to_class['b'] = ALL_REGS;
  microblaze_char_to_class['y'] = GR_REGS;
  microblaze_char_to_class['z'] = ST_REGS;

  /* For Divide count increments */
  microblaze_print_operand_punct['-'] = 1;  /* print divide label */
  microblaze_print_operand_punct['_'] = 1;  /* increment divide label */

  /* Set up array to map GCC register number to debug register number.
     Ignore the special purpose register numbers.  */

  for (i = 0; i < FIRST_PSEUDO_REGISTER; i++)
    microblaze_dbx_regno[i] = -1;

  start = GP_DBX_FIRST - GP_REG_FIRST;
  for (i = GP_REG_FIRST; i <= GP_REG_LAST; i++)
    microblaze_dbx_regno[i] = i + start;

  start = FP_DBX_FIRST - FP_REG_FIRST;
  for (i = FP_REG_FIRST; i <= FP_REG_LAST; i++)
    microblaze_dbx_regno[i] = i + start;


  /* Save GPR registers in word_mode sized hunks.  word_mode hasn't been
     initialized yet, so we can't use that here.  */
  gpr_mode = SImode;

  /* Set up array giving whether a given register can hold a given mode.
     At present, restrict ints from being in FP registers, because reload
     is a little enthusiastic about storing extra values in FP registers,
     and this is not good for things like OS kernels.  Also, due to the
     mandatory delay, it is as fast to load from cached memory as to move
     from the FP register.  */

  for (mode = VOIDmode;
       mode != MAX_MACHINE_MODE;
       mode = (enum machine_mode) ((int)mode + 1))
  {
    register int size              = GET_MODE_SIZE (mode);
#if 0
    register enum mode_class class = GET_MODE_CLASS (mode);
#endif

    for (regno = 0; regno < FIRST_PSEUDO_REGISTER; regno++)
    {
      register int ok;

      if (mode == CCmode) {
        ok = (ST_REG_P (regno) || GP_REG_P (regno)
              || FP_REG_P (regno));
      }
      else if (GP_REG_P (regno))
        ok = ((regno & 1) == 0 || size <= UNITS_PER_WORD); 
      else 
        ok = 0;

      microblaze_hard_regno_mode_ok[(int)mode][regno] = ok;
    }
  }
}

void
microblaze_order_regs_for_local_alloc (void)
{
  register int i;

  for (i = 0; i < FIRST_PSEUDO_REGISTER; i++)
    reg_alloc_order[i] = i;

}


/* The MICROBLAZE debug format wants all automatic variables and arguments
   to be in terms of the virtual frame pointer (stack pointer before
   any adjustment in the function), while the MICROBLAZE 3.0 linker wants
   the frame pointer to be the stack pointer after the initial
   adjustment.  So, we do the adjustment here.  The arg pointer (which
   is eliminated) points to the virtual frame pointer, while the frame
   pointer (which may be eliminated) points to the stack pointer after
   the initial adjustments.  */

HOST_WIDE_INT
microblaze_debugger_offset (
  rtx addr,
  HOST_WIDE_INT offset)
{
  rtx offset2 = const0_rtx;
  rtx reg = eliminate_constant_term (addr, &offset2);

  if (offset == 0)
    offset = INTVAL (offset2);

  if (reg == stack_pointer_rtx || reg == frame_pointer_rtx
      || reg == hard_frame_pointer_rtx)
  {
    HOST_WIDE_INT frame_size = (!current_frame_info.initialized)
      ? compute_frame_size (get_frame_size ())
      : current_frame_info.total_size;

    offset = offset - frame_size;
  }

  /* sdbout_parms does not want this to crash for unrecognized cases.  */
#if 0
  else if (reg != arg_pointer_rtx)
    fatal_insn ("microblaze_debugger_offset called with non stack/frame/arg pointer.", addr);
#endif

  return offset;
}

/* Implement INITIAL_ELIMINATION_OFFSET.  FROM is either the frame
   pointer or argument pointer or the return address pointer.  TO is either the stack pointer or
   hard frame pointer.  */

HOST_WIDE_INT
microblaze_initial_elimination_offset (int from, int to)
{  
  HOST_WIDE_INT offset;
    
  switch (from) {
    case FRAME_POINTER_REGNUM:
      offset = 0;
      break;
    case ARG_POINTER_REGNUM:
      if (to == STACK_POINTER_REGNUM || to == HARD_FRAME_POINTER_REGNUM)
        offset = compute_frame_size (get_frame_size ());				 
      else {
        abort ();
      }
      break;
    case RETURN_ADDRESS_POINTER_REGNUM:
      if (current_function_is_leaf) 						 
        offset = 0;				 			 
      else 
        offset = current_frame_info.gp_offset + 
          ((UNITS_PER_WORD - (POINTER_SIZE / BITS_PER_UNIT)));
      break;
    default:
      abort ();
  }
  return offset;
}

/* A C compound statement to output to stdio stream STREAM the
   assembler syntax for an instruction operand X.  X is an RTL
   expression.

   CODE is a value that can be used to specify one of several ways
   of printing the operand.  It is used when identical operands
   must be printed differently depending on the context.  CODE
   comes from the `%' specification that was used to request
   printing of the operand.  If the specification was just `%DIGIT'
   then CODE is 0; if the specification was `%LTR DIGIT' then CODE
   is the ASCII code for LTR.

   If X is a register, this macro should print the register's name.
   The names can be found in an array `reg_names' whose type is
   `char *[]'.  `reg_names' is initialized from `REGISTER_NAMES'.

   When the machine description has a specification `%PUNCT' (a `%'
   followed by a punctuation character), this macro is called with
   a null pointer for X and the punctuation character for CODE.

   The MICROBLAZE specific codes are:

   'X'  X is CONST_INT, prints 32 bits in hexadecimal format = "0x%08x",
   'x'  X is CONST_INT, prints 16 bits in hexadecimal format = "0x%04x",
   'd'  output integer constant in decimal,
   'z'	if the operand is 0, use $0 instead of normal operand.
   'D'  print second register of double-word register operand.
   'L'  print low-order register of double-word register operand.
   'M'  print high-order register of double-word register operand.
   'C'  print part of opcode for a branch condition.
   'N'  print part of opcode for a branch condition, inverted.
   'S'  X is CODE_LABEL, print with prefix of "LS" (for embedded switch).
   'B'  print 'z' for EQ, 'n' for NE
   'b'  print 'n' for EQ, 'z' for NE
   'T'  print 'f' for EQ, 't' for NE
   't'  print 't' for EQ, 'f' for NE
   'Z'  print register and a comma, but print nothing for $fcc0
   '?'	Print 'd' if we are to use a branch with delay slot instead of normal branch.
   '@'	Print the name of the assembler temporary register (at or rMB_ABI_ASM_TEMP_REGNUM).
   '.'	Print the name of the register with a hard-wired zero (zero or r0).
   '^'	Print the name of the pic call-through register (t9 or rMB_ABI_PIC_FUNC_REGNUM).
   '$'	Print the name of the stack pointer register (sp or rMB_ABI_STACK_POINTER_REGNUM).
   '+'	Print the name of the gp register (gp or rMB_ABI_GPRO_REGNUM).  
   '#'	Print nop if NOT in a .set noreorder section ie if the delay slot of a branch is not filled. */

void
print_operand (
  FILE *file,		/* file to write to */
  rtx op,		/* operand to print */
  int letter)		/* %<letter> or 0 */
{
  register enum rtx_code code;

  if (PRINT_OPERAND_PUNCT_VALID_P (letter))
  {
    switch (letter)
    {
      case '?':                             /* Conditionally add a 'd' to indicate filled delay slot */
        if (final_sequence != NULL)
          fputs ("d", file);
        break;

      case '#':                             /* Conditionally add a nop in unfilled delay slot */
        if (final_sequence == NULL) 
          fputs ("nop\t\t# Unfilled delay slot\n", file);
        break;

      case '@':
        fputs (reg_names [GP_REG_FIRST + MB_ABI_ASM_TEMP_REGNUM], file);
        break;

      case '^':
        fputs (reg_names [PIC_FUNCTION_ADDR_REGNUM], file);
        break;

      case '.':
        fputs (reg_names [GP_REG_FIRST + 0], file);
        break;

      case '$':
        fputs (reg_names[STACK_POINTER_REGNUM], file);
        break;

      case '+':
        fputs (reg_names[GP_REG_FIRST + MB_ABI_GPRO_REGNUM], file);
        break;

      case '-':
        fprintf(file,"%sL%d_",LOCAL_LABEL_PREFIX,div_count);
        break;

      case '_':
        div_count++;
        break;

        /*  case 'g':
            fputs("i",file);
     
            break;
        */
      default:
        error ("PRINT_OPERAND: Unknown punctuation '%c'", letter);
        break;
    }

    return;
  }

  if (! op)
  {
    error ("PRINT_OPERAND null pointer");
    return;
  }

  code = GET_CODE (op);

  if (code == SIGN_EXTEND)
    op = XEXP (op, 0), code = GET_CODE (op);

  if (letter == 'C')
    switch (code)
    {
      case EQ:	fputs ("eq",  file); break;
      case NE:	fputs ("ne",  file); break;
      case GT:	fputs ("gt",  file); break;
      case GE:	fputs ("ge",  file); break;
      case LT:	fputs ("lt",  file); break;
      case LE:	fputs ("le",  file); break;
      case GTU: fputs ("gtu", file); break;
      case GEU: fputs ("geu", file); break;
      case LTU: fputs ("ltu", file); break;
      case LEU: fputs ("leu", file); break;
      default:
        fatal_insn ("PRINT_OPERAND, invalid insn for %%C", op);
    }

  else if (letter == 'N')
    switch (code)
    {
      case EQ:	fputs ("ne",  file); break;
      case NE:	fputs ("eq",  file); break;
      case GT:	fputs ("le",  file); break;
      case GE:	fputs ("lt",  file); break;
      case LT:	fputs ("ge",  file); break;
      case LE:	fputs ("gt",  file); break;
      case GTU: fputs ("leu", file); break;
      case GEU: fputs ("ltu", file); break;
      case LTU: fputs ("geu", file); break;
      case LEU: fputs ("gtu", file); break;
      default:
        fatal_insn ("PRINT_OPERAND, invalid insn for %%N", op);
    }

  else if (letter == 'S')
  {
    char buffer[100];

    ASM_GENERATE_INTERNAL_LABEL (buffer, "LS", CODE_LABEL_NUMBER (op));
    assemble_name (file, buffer);
  }

  else if (letter == 'Z')
  {
    register int regnum;

    if (code != REG)
      abort ();

    regnum = REGNO (op);
    if (! ST_REG_P (regnum))
      abort ();

    if (regnum != ST_REG_FIRST)
      fprintf (file, "%s,", reg_names[regnum]);
  }

  else if (code == REG || code == SUBREG)
  {
    register int regnum;

    if (code == REG)
      regnum = REGNO (op);
    else
      regnum = true_regnum (op);

    if ((letter == 'M' && ! WORDS_BIG_ENDIAN)
        || (letter == 'L' && WORDS_BIG_ENDIAN)
        || letter == 'D')
      regnum++;

    fprintf (file, "%s", reg_names[regnum]);
  }

  else if (code == MEM)
    output_address (XEXP (op, 0));

  else if (code == CONST_DOUBLE
           && GET_MODE_CLASS (GET_MODE (op)) == MODE_FLOAT)
  {
/* GCC 3.4.1 
 * Removed the REAL_VALUE_TO_DECIMAL part 
 */
#if 0 
    REAL_VALUE_TYPE d;
    char s[30];
    if(!printed){
      /*fprintf(stderr,"Printing done here\n");*/
      REAL_VALUE_FROM_CONST_DOUBLE (d, op);
      REAL_VALUE_TO_DECIMAL (d, "%.20e", s);
      fprintf (file, s);
    }
#else
    char s[60];

    real_to_decimal (s, CONST_DOUBLE_REAL_VALUE (op), sizeof (s), 0, 1);
    fputs (s, file);
#endif
  }

  else if (code == UNSPEC)
  {
    print_operand_address(file, op);
  }

  else if (letter == 'x' && GET_CODE (op) == CONST_INT)
    fprintf (file, HOST_WIDE_INT_PRINT_HEX, 0xffff & INTVAL(op));

  else if (letter == 'X' && GET_CODE(op) == CONST_INT)
    fprintf (file, HOST_WIDE_INT_PRINT_HEX, INTVAL (op));

  else if (letter == 'd' && GET_CODE(op) == CONST_INT)
    fprintf (file, HOST_WIDE_INT_PRINT_DEC, (INTVAL(op)));

  else if (letter == 'z' && GET_CODE (op) == CONST_INT && INTVAL (op) == 0)
    fputs (reg_names[GP_REG_FIRST], file);

  else if (letter == 'd' || letter == 'x' || letter == 'X')
    error ("PRINT_OPERAND: letter %c was found & insn was not CONST_INT",
           letter);

  else if (letter == 'B')
    fputs (code == EQ ? "z" : "n", file);
  else if (letter == 'b')
    fputs (code == EQ ? "n" : "z", file);
  else if (letter == 'T')
    fputs (code == EQ ? "f" : "t", file);
  else if (letter == 't')
    fputs (code == EQ ? "t" : "f", file);

  else if (code == CONST && GET_CODE (XEXP (op, 0)) == REG)
  {
    print_operand (file, XEXP (op, 0), letter);
  }
  else if (letter == 'I')
    div_count++;
  else
    output_addr_const (file, op);
}

/* A C compound statement to output to stdio stream STREAM the
   assembler syntax for an instruction operand that is a memory
   reference whose address is ADDR.  ADDR is an RTL expression.

   Possible address classifications and output formats are,
   
   ADDRESS_REG                  "%0, r0"

   ADDRESS_REG with non-zero    "%0, <addr_const>"
   offset       

   ADDRESS_REG_INDEX            "rA, RB"    
                                (if rA is r0, rA and rB are swapped)

   ADDRESS_CONST_INT            "r0, <addr_const>"

   ADDRESS_SYMBOLIC             "rBase, <addr_const>"   
                                (rBase is a base register suitable for the symbol's type)
*/

void
print_operand_address (
  FILE *file,
  rtx addr)
{
  struct microblaze_address_info info;
  enum microblaze_address_type type;
  if (!microblaze_classify_address (&info, addr, GET_MODE (addr), 1)) 
    fatal_insn ("insn contains an invalid address !", addr);
  
  type = info.type;
  switch (info.type) {
    case ADDRESS_REG:
      fprintf (file, "%s,", reg_names[REGNO (info.regA)]);
      output_addr_const (file, info.offset);             
      break;
    case ADDRESS_REG_INDEX:
      if (REGNO (info.regA) == 0)                                       /* Make rB == r0 instead of rA == r0. This helps reduce read port congestion */
        fprintf (file, "%s,%s", reg_names[REGNO (info.regB)], reg_names[REGNO (info.regA)]);    
      else if (REGNO (info.regB) != 0)
        fprintf (file, "%s,%s", reg_names[REGNO (info.regB)], reg_names[REGNO (info.regA)]);        /* This is a silly swap to help Dhrystone */
      break;
    case ADDRESS_CONST_INT:
      fprintf (file, "%s,", reg_names[REGNO (info.regA)]);
      output_addr_const (file, info.offset);
      break;
    case ADDRESS_SYMBOLIC:
    case ADDRESS_GOTOFF:
    case ADDRESS_PLT:
      if (info.regA)
        fprintf (file, "%s,", reg_names[REGNO (info.regA)]);
      output_addr_const (file, info.symbol);
      if (type == ADDRESS_GOTOFF) {
        fputs("@GOT", file);
      } else if (type == ADDRESS_PLT) {
        fputs("@PLT", file);
      }
      break;
    case ADDRESS_INVALID:
      fatal_insn ("invalid address", addr);
      break;
  }
}

/* Compute a string to use as a temporary file name.  */

/* On MSDOS, write temp files in current dir
   because there's no place else we can expect to use.  */
#if __MSDOS__
#ifndef P_tmpdir
#define P_tmpdir "./"
#endif
#endif

/* Emit a new filename to a stream.  If this is MICROBLAZE ECOFF, watch out
   for .file's that start within a function.  If we are smuggling stabs, try to
   put out a MICROBLAZE ECOFF file and a stab.  */

void
microblaze_output_filename (
  FILE *stream,
  const char* name)
{
  static int first_time = 1;
  char ltext_label_name[100];

  if (first_time)
  {
    first_time = 0;
    SET_FILE_NUMBER ();
    current_function_file = name;
    fprintf (stream, "\t.file\t%d ", num_source_filenames);
    output_quoted_string (stream, name);
    putc ('\n', stream);
  }

  else if (write_symbols == DBX_DEBUG)
  {
    ASM_GENERATE_INTERNAL_LABEL (ltext_label_name, "Ltext", 0);
    fprintf (stream, "%s ", ASM_STABS_OP);
    output_quoted_string (stream, name);
    fprintf (stream, ",%d,0,0,%s\n", N_SOL, &ltext_label_name[1]);
  }

  else if (name != current_function_file
           && strcmp (name, current_function_file) != 0)
  {
    SET_FILE_NUMBER ();
    current_function_file = name;
    fprintf (stream, "\t.file\t%d", num_source_filenames);
    output_quoted_string (stream, name);
  }
}

/* Emit a linenumber.  For encapsulated stabs, we need to put out a stab
   as well as a .loc, since it is possible that MICROBLAZE ECOFF might not be
   able to represent the location for inlines that come from a different
   file.  */

void
microblaze_output_lineno (
  FILE *stream,
  int line)
{
  if (write_symbols == DBX_DEBUG)
  {
    ++sym_lineno;
    fprintf (stream, "%sLM%d:\n\t%s %d,0,%d,%sLM%d",
             LOCAL_LABEL_PREFIX, sym_lineno, ASM_STABN_OP, N_SLINE, line,
             LOCAL_LABEL_PREFIX, sym_lineno);
    fputc ('-', stream);
    assemble_name (stream, XSTR (XEXP (DECL_RTL (current_function_decl), 0), 0));
    fprintf(stream,"\n");
  }

  else
  {
    fprintf (stream, "\n\t%s.loc\t%d %d\n",
             (ignore_line_number) ? "#" : "",
             num_source_filenames, line);
  
    LABEL_AFTER_LOC (stream);
  }
}

/* Code to be executed just prior to the output of assembler code for INSN, 
   to modify the extracted operands so they will be output differently.

   Here the argument OPVEC is the vector containing the operands extracted
   from INSN, and NOPERANDS is the number of elements of the vector which
   contain meaningful data for this insn.  The contents of this vector are
   what will be used to convert the insn template into assembler code, so you
   can change the assembler output by changing the contents of the vector.

   We use it to 
    1) For the Gti pipeline, swap Ra and Rb, is Ra has the potential of being forwarded. 
       There is congestion at the read port for Rd and Ra. So we want 
       to minimize the number of times that this congestion occurs
    3) Update the delay slot statistics.  */
void
final_prescan_insn (rtx insn, 
		    rtx opvec[] ATTRIBUTE_UNUSED, 
		    int noperands ATTRIBUTE_UNUSED)
{
  /* GTi stuff here 
     -- Scan instruction sequence to see if a stall on a 'sw' can be avoided
        by swapping operands, thus using a forwarding path from a preceding
        instruction.

        FIXME
  */
  
  if (TARGET_STATS
      && (GET_CODE (insn) == JUMP_INSN || GET_CODE (insn) == CALL_INSN))
    dslots_jump_total++;
}

/* Output an element in the table of global constructors. */
void 
microblaze_asm_constructor (rtx symbol ATTRIBUTE_UNUSED, int priority) 
{ 

  const char *section = ".ctors";
  char buf[16];

  if (priority != DEFAULT_INIT_PRIORITY)
  {
    sprintf (buf, ".ctors.%.5u",
             /* Invert the numbering so the linker puts us in the proper
                order; constructors are run from right to left, and the
                linker sorts in increasing order.  */
             MAX_INIT_PRIORITY - priority);
    section = buf;
  }

  named_section_flags (section, SECTION_WRITE);
  fputs ("\t.word\t", asm_out_file);
  output_addr_const (asm_out_file, symbol);
  fputs ("\n", asm_out_file);
} 

/* Output an element in the table of global destructors. */
void 
microblaze_asm_destructor (rtx symbol, int priority) 
{ 
  const char *section = ".dtors";
  char buf[16];

  if (priority != DEFAULT_INIT_PRIORITY)
  {
    sprintf (buf, ".dtors.%.5u",
             /* Invert the numbering so the linker puts us in the proper
                order; constructors are run from right to left, and the
                linker sorts in increasing order.  */
             MAX_INIT_PRIORITY - priority);
    section = buf;
  }

  named_section_flags (section, SECTION_WRITE);
  fputs ("\t.word\t", asm_out_file);
  output_addr_const (asm_out_file, symbol);
  fputs ("\n", asm_out_file);
} 
   

/* A function to output to the stdio stream stream a label whose name is made from the string prefix 
   and the number labelno. */
void 
microblaze_internal_label (
  FILE *STREAM,
  const char* prefix,
  unsigned long labelno)
{
  fprintf (STREAM, "%s%s%ld:\n", LOCAL_LABEL_PREFIX, prefix, labelno);
}

/* Emit either a label, .comm, or .lcomm directive, and mark that the symbol
   is used, so that we don't emit an .extern for it in microblaze_asm_file_end.  */

void
microblaze_declare_object (FILE *stream, char *name, char *section, char *fmt, int size)
{

  fputs (section, stream);		/* "", "\t.comm\t", or "\t.lcomm\t" */
  assemble_name (stream, name);
  fprintf (stream, fmt, size);	/* ":\n", ",%u\n", ",%u\n" */

  if (TARGET_GP_OPT)
  {
    tree name_tree = get_identifier (name);
    TREE_ASM_WRITTEN (name_tree) = 1;
  }
}

/* Output a double precision value to the assembler.  If both the
   host and target are IEEE, emit the values in hex.  */

void
microblaze_output_double (FILE *stream, REAL_VALUE_TYPE value)
{
  union {double d; REAL_VALUE_TYPE value; } val;
#ifdef REAL_VALUE_TO_TARGET_DOUBLE
  long value_long[2];
  val.value = value;
  REAL_VALUE_TO_TARGET_DOUBLE (value, value_long);
   
  fprintf (stream, "\t.word\t0x%08lx\t\t# %.20g\n\t.word\t0x%08lx\n",
           value_long[0], val.d, value_long[1]);
#else
  val.value = value;
  fprintf (stream, "\t.double\t%.20g\n", val.d);
#endif
}

/* Output a single precision value to the assembler.  If both the
   host and target are IEEE, emit the values in hex.  */

void
microblaze_output_float (FILE *stream, REAL_VALUE_TYPE value)
{
  union {double d; REAL_VALUE_TYPE value; } val;
#ifdef REAL_VALUE_TO_TARGET_SINGLE
  long value_long;
  val.value = value;
  REAL_VALUE_TO_TARGET_SINGLE (value, value_long);
    
  fprintf (stream, "\t.word\t0x%08lx\t\t# %.12g (float)\n", value_long, val.d);
#else
  val.value = value;
  fprintf (stream, "\t.float\t%.12g\n", val.d);
#endif
}

/* Return the bytes needed to compute the frame pointer from the current
   stack pointer.

   MicroBlaze stack frames look like:



             Before call		        After call
        +-----------------------+	+-----------------------+
   high |			|       |      			|
   mem. |  local variables,     |	|  local variables,	|
        |  callee saved and     |       |  callee saved and    	|
	|  temps     		|       |  temps     	        |
        +-----------------------+	+-----------------------+
        |  arguments for called	|       |  arguments for called |
	|  subroutines		|	|  subroutines  	|
        |  (optional)           |       |  (optional)           |
        +-----------------------+	+-----------------------+
	|  Link register 	|	|  Link register        |
    SP->|                       |       |                       |
	+-----------------------+       +-----------------------+
					|		        |
                                        |  local variables,     |
                                        |  callee saved and     |
                                        |  temps                |
					+-----------------------+
                                        |   MSR (optional if,   |
                                        |   interrupt handler)  |
					+-----------------------+
					|			|
                                        |  alloca allocations   |
        				|			|
					+-----------------------+
					|			|
                                        |  arguments for called |
                                        |  subroutines          |
                                        |  (optional)           |
        				|		        |
					+-----------------------+
                                        |  Link register        |
   low                           FP,SP->|                       |
   memory        			+-----------------------+

*/

HOST_WIDE_INT
compute_frame_size (
  HOST_WIDE_INT size)                 /* # of var. bytes allocated */
{
  int regno;
  HOST_WIDE_INT total_size;           /* # bytes that the entire frame takes up */
  HOST_WIDE_INT var_size;             /* # bytes that local variables take up */
  HOST_WIDE_INT args_size;            /* # bytes that outgoing arguments take up */
  int link_debug_size;                /* # bytes for link register */
  HOST_WIDE_INT gp_reg_size;          /* # bytes needed to store calle-saved gp regs */
  long mask;                          /* mask of saved gp registers */
#if 0
  static int check = 0;
#endif

  interrupt_handler   = (microblaze_interrupt_function_p (current_function_decl));
  save_volatiles      = (microblaze_save_volatiles (current_function_decl));

  gp_reg_size = 0;
  mask = 0;
  var_size   = size;
  args_size  = current_function_outgoing_args_size;
  
  if ((args_size == 0) && current_function_calls_alloca)
    args_size = NUM_OF_ARGS * UNITS_PER_WORD;
    
  total_size = var_size + args_size;
 
  if (flag_pic == 2 /*&& !current_function_is_leaf */ )
    regs_ever_live[MB_ABI_PIC_ADDR_REGNUM] = 1; /* force setting GOT */

  /* Calculate space needed for gp registers.  */
  for (regno = GP_REG_FIRST; regno <= GP_REG_LAST; regno++)
  {
    if (MUST_SAVE_REGISTER (regno))
    {
      
      if (regno != MB_ABI_SUB_RETURN_ADDR_REGNUM)               /* Don't account for link register. It is accounted specially below */
        gp_reg_size += GET_MODE_SIZE (gpr_mode);

      mask |= (1L << (regno - GP_REG_FIRST));
    }
  }
  
  total_size += gp_reg_size;

  /* Add 4 bytes for MSR */                        
  if (interrupt_handler)
    total_size += 4; 
                                              
  /* No space to be allocated for link register in leaf functions with no other stack requirements */
  if (total_size == 0 && current_function_is_leaf)
    link_debug_size = 0;
  else
    link_debug_size = UNITS_PER_WORD;    

  total_size += link_debug_size;

  /* Save other computed information.  */
  current_frame_info.total_size = total_size;
  current_frame_info.var_size = var_size;
  current_frame_info.args_size = args_size;
  current_frame_info.gp_reg_size = gp_reg_size;
  current_frame_info.mask = mask;
  current_frame_info.initialized = reload_completed;
  current_frame_info.num_gp = gp_reg_size / UNITS_PER_WORD;
  current_frame_info.link_debug_size = link_debug_size; 

#if 0
  {
    char *fnname;

    fnname = XSTR (XEXP (DECL_RTL (current_function_decl), 0), 0);
    fprintf(stderr," ----- stack usage stats for (%s) ---- \n", fnname);
    fprintf(stderr,"total_size %d\n",total_size);
    fprintf(stderr,"var_size %d\n",var_size);
    fprintf(stderr,"args_size %d\n",args_size);
    fprintf(stderr,"gp_reg_size %d\n",gp_reg_size);
    fprintf(stderr,"mask %x\n",mask);
    fprintf(stderr,"num_gp %d\n",current_frame_info.num_gp);
    fprintf(stderr,"link_debug %d\n",link_debug_size);
    fprintf(stderr," ----- end stack usage stats ---- \n");
  }
#endif

  if (mask)
    current_frame_info.gp_offset = (total_size - gp_reg_size);      /* Offset from which to callee-save GP regs */
  else
    current_frame_info.gp_offset = 0;
  
  /* Ok, we're done.  */
  return total_size;
}

/* Common code to emit the insns (or to write the instructions to a file)
   to save/restore registers.

   Other parts of the code assume that MICROBLAZE_TEMP1_REGNUM (aka large_reg)
   is not modified within save_restore_insns.  */

#define BITSET_P(VALUE,BIT) (((VALUE) & (1L << (BIT))) != 0)

/* Save or restore instructions based on whether this is the prologue or epilogue. 
   prologue is 1 for the prologue */
static void
save_restore_insns (int prologue)
{
  rtx base_reg_rtx, reg_rtx, mem_rtx, /* msr_rtx, */ isr_reg_rtx, isr_mem_rtx, isr_msr_rtx, insn;
  long mask = current_frame_info.mask;
  HOST_WIDE_INT base_offset, gp_offset;
  int regno;

  if (frame_pointer_needed
      && ! BITSET_P (mask, HARD_FRAME_POINTER_REGNUM - GP_REG_FIRST))
    abort ();

  if (mask == 0)
    return;

  /* Save registers starting from high to low.  The debuggers prefer at least
     the return register be stored at func+4, and also it allows us not to
     need a nop in the epilog if at least one register is reloaded in
     addition to return address.  */
  
  
  /* Pick which pointer to use as a base register.  For small frames, just
     use the stack pointer.  Otherwise, use a temporary register.  Save 2
     cycles if the save area is near the end of a large frame, by reusing
     the constant created in the prologue/epilogue to adjust the stack
     frame.  */
  
  gp_offset  = current_frame_info.gp_offset;
  
  if (gp_offset <= 0)
    error ("gp_offset (%ld) is less than or equal to zero.", (long) gp_offset);
  
  base_reg_rtx = stack_pointer_rtx;
  base_offset  = 0;
  
  /* For interrupt_handlers, need to save/restore the MSR */
  if (interrupt_handler) {
    isr_mem_rtx = gen_rtx_MEM (gpr_mode, 
                           gen_rtx_PLUS (Pmode, base_reg_rtx, 
                                    GEN_INT (current_frame_info.gp_offset - UNITS_PER_WORD)));

    MEM_VOLATILE_P (isr_mem_rtx) = 1;                             /* Do not optimize in flow analysis */
    isr_reg_rtx = gen_rtx_REG (gpr_mode, MB_ABI_MSR_SAVE_REG);
    isr_msr_rtx = gen_rtx_REG (gpr_mode, ST_REG_FIRST);
  }

  if (interrupt_handler && !prologue) {
    emit_move_insn (isr_reg_rtx, isr_mem_rtx);
    emit_move_insn (isr_msr_rtx, isr_reg_rtx);
    emit_insn (gen_rtx_USE (SImode, isr_reg_rtx));                  /* Do not optimize in flow analysis */
    emit_insn (gen_rtx_USE (SImode, isr_msr_rtx));                  /* Do not optimize in flow analysis */
  } 

  for (regno = GP_REG_FIRST; regno <= GP_REG_LAST; regno++) 
  {
    if (BITSET_P (mask, regno - GP_REG_FIRST))
    {
      if (regno == MB_ABI_SUB_RETURN_ADDR_REGNUM)             /* Don't handle here. Already handled as the first register */
        continue;

      reg_rtx = gen_rtx_REG (gpr_mode, regno);  
      insn = gen_rtx_PLUS (Pmode, base_reg_rtx, GEN_INT (gp_offset));
      mem_rtx = gen_rtx_MEM (gpr_mode, insn);
      if (interrupt_handler || save_volatiles)
        MEM_VOLATILE_P (mem_rtx) = 1;                         /* Do not optimize in flow analysis */    

      if (prologue)
      {
        insn = emit_move_insn (mem_rtx, reg_rtx);
        RTX_FRAME_RELATED_P (insn) = 1;
      }
      else /* if (regno != (PIC_OFFSET_TABLE_REGNUM - GP_REG_FIRST)) */
      {
        insn = emit_move_insn (reg_rtx, mem_rtx);
      }

      REG_NOTES (insn) = gen_rtx_EXPR_LIST (REG_MAYBE_DEAD, const0_rtx, NULL_RTX);
      
      gp_offset += GET_MODE_SIZE (gpr_mode);
    }
  }

  if (interrupt_handler && prologue) {
    emit_move_insn (isr_reg_rtx, isr_msr_rtx);
    emit_move_insn (isr_mem_rtx, isr_reg_rtx);

    emit_insn (gen_rtx_USE (SImode, isr_reg_rtx));                  /* Do not optimize in flow analysis */
    emit_insn (gen_rtx_USE (SImode, isr_msr_rtx));                  /* Do not optimize in flow analysis */
  }

  /* Done saving and restoring */
}


/* Set up the stack and frame (if desired) for the function.  */
static void
microblaze_function_prologue (
  FILE *file,
  int size ATTRIBUTE_UNUSED)
{
#ifndef FUNCTION_NAME_ALREADY_DECLARED
  const char *fnname;
#endif
  long fsiz = current_frame_info.total_size;

#if 0   
  ASM_OUTPUT_SOURCE_FILENAME (file, DECL_SOURCE_FILE (current_function_decl));

#ifdef SDB_DEBUGGING_INFO
  if (debug_info_level != DINFO_LEVEL_TERSE && write_symbols == SDB_DEBUG)
    microblaze_output_lineno (file, DECL_SOURCE_LINE (current_function_decl));
#endif
#endif
  inside_function = 1;

#ifndef FUNCTION_NAME_ALREADY_DECLARED
  /* Get the function name the same way that toplev.c does before calling
     assemble_start_function.  This is needed so that the name used here
     exactly matches the name used in ASM_DECLARE_FUNCTION_NAME.  */
  fnname = XSTR (XEXP (DECL_RTL (current_function_decl), 0), 0);
  if (!flag_inhibit_size_directive)
  {
    fputs ("\t.ent\t", file);
    if (interrupt_handler && strcmp(INTERRUPT_HANDLER_NAME,fnname))  
      fputs ("_interrupt_handler", file); 
    else 
      assemble_name (file, fnname); 
    fputs ("\n", file);
    if (!interrupt_handler)
      ASM_OUTPUT_TYPE_DIRECTIVE(file, fnname, "function");
  }

  assemble_name (file, fnname);
  fputs (":\n", file);

  if (interrupt_handler && strcmp (INTERRUPT_HANDLER_NAME, fnname)) 
    fputs ("_interrupt_handler:\n",file); 
#endif

  if (!flag_inhibit_size_directive)
  {
    /* .frame FRAMEREG, FRAMESIZE, RETREG */
    fprintf (file,
             "\t.frame\t%s,%ld,%s\t\t# vars= %ld, regs= %d, args= %d\n",
             (reg_names[(frame_pointer_needed)
                        ? HARD_FRAME_POINTER_REGNUM : STACK_POINTER_REGNUM]),
             fsiz,
             reg_names[MB_ABI_SUB_RETURN_ADDR_REGNUM + GP_REG_FIRST],
             current_frame_info.var_size,
             current_frame_info.num_gp,
             current_function_outgoing_args_size);
    fprintf (file, "\t.mask\t0x%08lx\n", current_frame_info.mask);
  }
}

/* Output extra assembler code at the end of a prologue */
void
microblaze_function_end_prologue (FILE *file)
{
  if (TARGET_STACK_CHECK) {
    fprintf (file, "\t# Stack Check Stub -- Start.\n\t");
    fprintf (file, "ori\tr18,r0,_stack_end\n\t");
    fprintf (file, "cmpu\tr18,r1,r18\n\t");
    fprintf (file, "bgei\tr18,_stack_overflow_exit\n\t");
    fprintf (file, "# Stack Check Stub -- End.\n");        
  }
}

/* Expand the prologue into a bunch of separate insns.  */

void
microblaze_expand_prologue (void)
{
  int regno;
  HOST_WIDE_INT fsiz;
  const char *arg_name = 0;
  tree fndecl = current_function_decl;
  tree fntype = TREE_TYPE (fndecl);
  tree fnargs = DECL_ARGUMENTS (fndecl);
  rtx next_arg_reg;
  int i;
  tree next_arg;
  tree cur_arg;
  CUMULATIVE_ARGS args_so_far;
#if 0
  rtx reg_18_save = NULL_RTX;
#endif
  rtx mem_rtx, reg_rtx;

  /* If struct value address is treated as the first argument, make it so.  */
  if (aggregate_value_p (DECL_RESULT (fndecl), fntype)
      && ! current_function_returns_pcc_struct)                                               
  {
    tree type = build_pointer_type (fntype);
    tree function_result_decl = build_decl (PARM_DECL, NULL_TREE, type);

    DECL_ARG_TYPE (function_result_decl) = type;
    TREE_CHAIN (function_result_decl) = fnargs;
    fnargs = function_result_decl;
  }

  /* Determine the last argument, and get its name.  */

  INIT_CUMULATIVE_ARGS (args_so_far, fntype, NULL_RTX, 0, 0);
  regno = GP_ARG_FIRST;

  for (cur_arg = fnargs; cur_arg != 0; cur_arg = next_arg)
  {
    tree passed_type = DECL_ARG_TYPE (cur_arg);
    enum machine_mode passed_mode = TYPE_MODE (passed_type);
    rtx entry_parm;

    if (TREE_ADDRESSABLE (passed_type))
    {
      passed_type = build_pointer_type (passed_type);
      passed_mode = Pmode;
    }

    entry_parm = FUNCTION_ARG (args_so_far, passed_mode, passed_type, 1);

    if (entry_parm)
    {
      int words;

      /* passed in a register, so will get homed automatically */
      if (GET_MODE (entry_parm) == BLKmode)
        words = (int_size_in_bytes (passed_type) + 3) / 4;
      else
        words = (GET_MODE_SIZE (GET_MODE (entry_parm)) + 3) / 4;

      regno = REGNO (entry_parm) + words - 1;
    }
    else
    {
      regno = GP_ARG_LAST+1;
      break;
    }

    FUNCTION_ARG_ADVANCE (args_so_far, passed_mode, passed_type, 1);

    next_arg = TREE_CHAIN (cur_arg);
    if (next_arg == 0)
    {
      if (DECL_NAME (cur_arg))
        arg_name = IDENTIFIER_POINTER (DECL_NAME (cur_arg));

      break;
    }
  }

  /* In order to pass small structures by value in registers compatibly with
     the MicroBlaze compiler, we need to shift the value into the high part of the
     register.  Function_arg has encoded a PARALLEL rtx, holding a vector of
     adjustments to be made as the next_arg_reg variable, so we split up the
     insns, and emit them separately.  */

  /* IN MicroBlaze shift has been modified to be a combination of adds
     and shifts in other directions, Hence we need to change the code
     a bit */

  next_arg_reg = FUNCTION_ARG (args_so_far, VOIDmode, void_type_node, 1);
  if (next_arg_reg != 0 && GET_CODE (next_arg_reg) == PARALLEL)
  {
    rtvec adjust = XVEC (next_arg_reg, 0);
    int num = GET_NUM_ELEM (adjust);

    for (i = 0; i < num; i++)
    {
      rtx pattern = RTVEC_ELT (adjust, i);
      /* 	  if (GET_CODE (pattern) != SET */
      /* 	      || GET_CODE (SET_SRC (pattern)) != ASHIFT) */
      /* 	    fatal_insn ("Insn is not a shift", pattern); */
      /* 	  if (GET_CODE (pattern) != SET )
                  fatal_insn ("Insn is Not Set", pattern); */

      /*	  PUT_CODE (SET_SRC (pattern), ASHIFTRT);*/
      emit_insn (pattern);
    }
  }

  fsiz = compute_frame_size (get_frame_size ());

  /* If this function is a varargs function, store any registers that
     would normally hold arguments ($5 - $10) on the stack.  */
  if (((TYPE_ARG_TYPES (fntype) != 0
        && (TREE_VALUE (tree_last (TYPE_ARG_TYPES (fntype)))
            != void_type_node))
       || (arg_name != 0
           && ((arg_name[0] == '_'
                && strcmp (arg_name, "__builtin_va_alist") == 0)
               || (arg_name[0] == 'v'
                   && strcmp (arg_name, "va_alist") == 0)))))
  {
    int offset = (regno - GP_ARG_FIRST + 1) * UNITS_PER_WORD;
    rtx ptr = stack_pointer_rtx;

    /* If we are doing svr4-abi, sp has already been decremented by fsiz. */
    for (; regno <= GP_ARG_LAST; regno++)
    {
      if (offset != 0)
        ptr = gen_rtx_PLUS (Pmode, stack_pointer_rtx, GEN_INT (offset));
      emit_move_insn (gen_rtx_MEM (gpr_mode, ptr),
                      gen_rtx_REG (gpr_mode, regno));
         
      offset += GET_MODE_SIZE (gpr_mode);
    }
      
  }
  
  if (fsiz > 0)
  {
    rtx fsiz_rtx = GEN_INT (fsiz);

    rtx insn = NULL;
    insn = emit_insn (gen_subsi3 (stack_pointer_rtx, stack_pointer_rtx,
                                  fsiz_rtx));
    if (insn)
      RTX_FRAME_RELATED_P (insn) = 1;

    /* Handle SUB_RETURN_ADDR_REGNUM specially at first */
    if (!current_function_is_leaf || interrupt_handler) {
      mem_rtx = gen_rtx_MEM (gpr_mode,
                         gen_rtx_PLUS (Pmode, stack_pointer_rtx, const0_rtx));
      
      if (interrupt_handler)
        MEM_VOLATILE_P (mem_rtx) = 1;                     /* Do not optimize in flow analysis */
    
      reg_rtx = gen_rtx_REG (gpr_mode, MB_ABI_SUB_RETURN_ADDR_REGNUM);
      insn = emit_move_insn (mem_rtx, reg_rtx);
      RTX_FRAME_RELATED_P (insn) = 1;
     }

    save_restore_insns (1);                             /* _save_ registers for prologue */
  
    if (frame_pointer_needed)
    {
      rtx insn = 0;

      insn = emit_insn (gen_movsi (hard_frame_pointer_rtx,
                                   stack_pointer_rtx));
      
      if (insn)
        RTX_FRAME_RELATED_P (insn) = 1;
    }

  }

  if (flag_pic == 2 && (/*!current_function_is_leaf || */
      regs_ever_live[MB_ABI_PIC_ADDR_REGNUM])) {
    rtx insn;
    REGNO (pic_offset_table_rtx) = MB_ABI_PIC_ADDR_REGNUM;
    insn = emit_insn (gen_set_got (pic_offset_table_rtx));  /* setting GOT */
    REG_NOTES (insn) = gen_rtx_EXPR_LIST (REG_MAYBE_DEAD, const0_rtx, NULL);
  }

  /* If we are profiling, make sure no instructions are scheduled before
     the call to mcount.  */
  /* profile_block_flag killed in GCC 3.4.1 */
   
  /*   if (profile_flag || profile_block_flag)*/
  if (profile_flag)
    emit_insn (gen_blockage ());

  /* [02/01/02] This section checks the stack at runtime to see if it
     has passed the malloc_base_Addr */

  /*  GCC 3.4.1
   *  This does not work reliably. Instruction scheduling seems intent on 
   *  killing the stack check insns and later complains about deleting
   *  frame related stuff. Doing the stack check in function_end_prologue instead.
   */ 
  /* 
     if (TARGET_STACK_CHECK){
     rtx insn;
     insn = emit_insn (gen_stack_check ());
     emit_insn (gen_blockage ());
     }
  */
}



/* Do any necessary cleanup after a function to restore stack, frame,
   and regs. */

#define RA_MASK ((long) 0x80000000)	/* 1 << 31 */
#define PIC_OFFSET_TABLE_MASK (1 << (PIC_OFFSET_TABLE_REGNUM - GP_REG_FIRST))

void
microblaze_function_epilogue (
  FILE *file ATTRIBUTE_UNUSED,
  HOST_WIDE_INT size ATTRIBUTE_UNUSED)
{
  const char *fnname;

#ifndef FUNCTION_NAME_ALREADY_DECLARED
  /* Get the function name the same way that toplev.c does before calling
     assemble_start_function.  This is needed so that the name used here
     exactly matches the name used in ASM_DECLARE_FUNCTION_NAME.  */
  fnname = XSTR (XEXP (DECL_RTL (current_function_decl), 0), 0);

  if (!flag_inhibit_size_directive)
  {
    fputs ("\t.end\t", file);
    if (interrupt_handler)
      fputs("_interrupt_handler",file);
    else
      assemble_name (file, fnname);
    fputs ("\n", file);
  }
#endif

  if (TARGET_STATS)
  {
    int num_gp_regs = current_frame_info.gp_reg_size / 4;
    int num_regs = num_gp_regs;
    const char *name = fnname;
      
    if (name[0] == '*')
      name++;
      
    dslots_load_total += num_regs;
      
    fprintf (stderr,
             "%-20s fp=%c leaf=%c alloca=%c setjmp=%c stack=%4ld arg=%3d reg=%2d delay=%3d/%3dL %3d/%3dJ refs=%3d/%3d/%3d",
             name, frame_pointer_needed ? 'y' : 'n',
             (current_frame_info.mask & RA_MASK) != 0 ? 'n' : 'y',
             current_function_calls_alloca ? 'y' : 'n',
             current_function_calls_setjmp ? 'y' : 'n',
             current_frame_info.total_size,
             current_function_outgoing_args_size, num_gp_regs,
             dslots_load_total, dslots_load_filled,
             dslots_jump_total, dslots_jump_filled,
             num_refs[0], num_refs[1], num_refs[2]);

    if (HALF_PIC_NUMBER_PTRS > prev_half_pic_ptrs)
    {
      fprintf (stderr,
               " half-pic=%3d", HALF_PIC_NUMBER_PTRS - prev_half_pic_ptrs);
      prev_half_pic_ptrs = HALF_PIC_NUMBER_PTRS;
    }

    if (HALF_PIC_NUMBER_REFS > prev_half_pic_refs)
    {
      fprintf (stderr,
               " pic-ref=%3d", HALF_PIC_NUMBER_REFS - prev_half_pic_refs);
      prev_half_pic_refs = HALF_PIC_NUMBER_REFS;
    }

    fputc ('\n', stderr);
  }

  /* Reset state info for each function.  */
  inside_function = 0;
  ignore_line_number = 0;
  dslots_load_total = 0;
  dslots_jump_total = 0;
  dslots_load_filled = 0;
  dslots_jump_filled = 0;
  num_refs[0] = 0;
  num_refs[1] = 0;
  num_refs[2] = 0;
  microblaze_load_reg = 0;
  microblaze_load_reg2 = 0;
  current_frame_info = zero_frame_info;

  while (string_constants != NULL)
  {
    struct string_constant *next;

    next = string_constants->next;
    free (string_constants);
    string_constants = next;
  }

  /* Restore the output file if optimizing the GP (optimizing the GP causes
     the text to be diverted to a tempfile, so that data decls come before
     references to the data).  */
}

/* Expand the epilogue into a bunch of separate insns.  */

void
microblaze_expand_epilogue (void)
{
  HOST_WIDE_INT fsiz = current_frame_info.total_size;
  rtx fsiz_rtx = GEN_INT (fsiz);
  rtx reg_rtx;
  rtx mem_rtx;
  
#if 0
  char *fnname = XSTR (XEXP (DECL_RTL (current_function_decl), 0), 0);
#endif
    
  /* In case of interrupt handlers use addki instead of addi for changing the stack pointer value */
  
  if (microblaze_can_use_return_insn ())
  {
    emit_jump_insn (gen_return_internal (gen_rtx_REG (Pmode,
                                                  GP_REG_FIRST + MB_ABI_SUB_RETURN_ADDR_REGNUM)));
    return;
  }

  if (fsiz > 0)
  {
    /* Restore SUB_RETURN_ADDR_REGNUM at first. This is to prevent the sequence of load-followed by a use (in rtsd) 
       in every prologue. Saves a load-use stall cycle  :) 
       This is also important to handle alloca. (See comments for if (frame_pointer_needed) below */

    if (!current_function_is_leaf || interrupt_handler) {
      mem_rtx = gen_rtx_MEM (gpr_mode, gen_rtx_PLUS (Pmode, stack_pointer_rtx, const0_rtx));
      if (interrupt_handler)
        MEM_VOLATILE_P (mem_rtx) = 1;                       /* Do not optimize in flow analysis */
      reg_rtx = gen_rtx_REG (gpr_mode, MB_ABI_SUB_RETURN_ADDR_REGNUM);
      emit_move_insn (reg_rtx, mem_rtx);
    }

    /* It is important that this is done after we restore the return address register (above).
       When alloca is used, we want to restore the sub-routine return address only from the current
       stack top and not from the frame pointer (which we restore below). (frame_pointer + 0) might have
       been over-written since alloca allocates memory on the current stack */
    if (frame_pointer_needed)
      emit_insn (gen_movsi (stack_pointer_rtx, hard_frame_pointer_rtx));
    
    save_restore_insns (0);                             /* _restore_ registers for epilogue */
    emit_insn (gen_blockage ());
    emit_insn (gen_addsi3 (stack_pointer_rtx, stack_pointer_rtx, fsiz_rtx));
  }

  emit_jump_insn (gen_return_internal (gen_rtx_REG (Pmode, GP_REG_FIRST + MB_ABI_SUB_RETURN_ADDR_REGNUM)));
}


/* Return nonzero if this function is known to have a null epilogue.
   This allows the optimizer to omit jumps to jumps if no stack
   was created.  */

int
microblaze_can_use_return_insn (void)
{
  if (! reload_completed)
    return 0;

  if (regs_ever_live[MB_ABI_SUB_RETURN_ADDR_REGNUM] || profile_flag)
    return 0;

  if (current_frame_info.initialized)
    return current_frame_info.total_size == 0;

  return compute_frame_size (get_frame_size ()) == 0;
}

/* This function returns the register class required for a secondary
   register when copying between one of the registers in CLASS, and X,
   using MODE.  If IN_P is nonzero, the copy is going from X to the
   register, otherwise the register is the source.  A return value of
   NO_REGS means that no secondary register is required.  */

enum reg_class
microblaze_secondary_reload_class (
  enum reg_class class,
  enum machine_mode mode ATTRIBUTE_UNUSED,
  rtx x,
  int in_p)
{
  enum reg_class gr_regs = GR_REGS;
  int regno = -1;
  int gp_reg_p;

  if (GET_CODE (x) == SIGN_EXTEND)
  {
    int off = 0;

    x = XEXP (x, 0);

    /* We may be called with reg_renumber NULL from regclass.
       ??? This is probably a bug.  */
    if (reg_renumber)
      regno = true_regnum (x);
    else
    {
      while (GET_CODE (x) == SUBREG)
      {
#ifdef MJE_SUBREG_REG
        off += SUBREG_REG (x);
#endif
        x = SUBREG_REG (x);
      }

      if (GET_CODE (x) == REG)
        regno = REGNO (x) + off;
    }
  }

  else if (GET_CODE (x) == REG || GET_CODE (x) == SUBREG)
    regno = true_regnum (x);

  gp_reg_p = GP_REG_P (regno);

  if (MD_REG_P (regno))
  {
    return class == gr_regs ? NO_REGS : gr_regs;
  }

  /* We can only copy a value to a condition code register from a
     floating point register, and even then we require a scratch
     floating point register.  We can only copy a value out of a
     condition code register into a general register.  */
  if (class == ST_REGS)
  {
    if (in_p)
      return FP_REGS;
    return GP_REG_P (regno) ? NO_REGS : GR_REGS;
  }
  if (ST_REG_P (regno))
  {
    if (! in_p)
      return FP_REGS;
    return class == GR_REGS ? NO_REGS : GR_REGS;
  }

  return NO_REGS;
}


/* We keep a list of constants we which we have to add to internal
   constant tables in the middle of large functions.  */

struct constant
{
  struct constant *next;
  rtx value;
  rtx label;
  enum machine_mode mode;
};

/* Add a constant to the list in *PCONSTANTS.  */

#if 0
static rtx
add_constant (
  struct constant **pconstants,
  rtx val,
  enum machine_mode mode)
{
  struct constant *c;

  for (c = *pconstants; c != NULL; c = c->next)
    if (mode == c->mode && rtx_equal_p (val, c->value))
      return c->label;

  c = (struct constant *) xmalloc (sizeof *c);
  c->value = val;
  c->mode = mode;
  c->label = gen_label_rtx ();
  c->next = *pconstants;
  *pconstants = c;
  return c->label;
}
#endif


/* Exported to toplev.c.
   Do a final pass over the function, just before delayed branch scheduling.  */
void
machine_dependent_reorg (void)
{
  return;

}

/* Get the base register for accessing a value from the memory or
   Symbol ref. Used for Microblaze Small Data Area Pointer Optimization */
int
get_base_reg(rtx x)
{
  tree decl;
  int base_reg = (flag_pic ? MB_ABI_PIC_ADDR_REGNUM : MB_ABI_BASE_REGNUM);
  
  if (TARGET_XLGP_OPT
      && GET_CODE(x) == SYMBOL_REF 
      && SYMBOL_REF_SMALL_P(x)
      && (decl = SYMBOL_REF_DECL(x)) != NULL)  
  {
    if (TREE_READONLY(decl))
      base_reg = MB_ABI_GPRO_REGNUM;
    else 
      base_reg = MB_ABI_GPRW_REGNUM;
  }

  return base_reg;
}

#define SIZE_FMT 70


char *
format_load_store (char* ls_fmt, 
                   enum load_store ls_type, 
                   enum machine_mode ls_mode,
                   rtx operand,
                   int offset)
{

  int base_reg = 0;
  int i;
  const char* ld_mode_text,*st_mode_text;
  rtx temp;

  /*  if(ls_fmt) free(ls_fmt);*/
  ls_fmt = (char*)xmalloc(SIZE_FMT);
  for(i = 0 ;i < SIZE_FMT; i++) ls_fmt[i] = 0;
  /* certain operands are mem */
  while (GET_CODE(operand)== MEM)
    operand = XEXP (operand, 0);
  /* set the mode_text */
  switch(ls_mode){
    case SImode:
    case SFmode: 
      st_mode_text="w";
      ld_mode_text="w";
      break;
    case HImode: 
      st_mode_text="h";
      ld_mode_text="hu";
      break;
    case QImode: 
      st_mode_text="b";
      ld_mode_text="bu";
      break;
    default:
      break;
  }


  if (ls_type == LOAD){
    /* Get the base register for Memory operations */  
    if(GET_CODE(operand) != PLUS ){
      base_reg = get_base_reg(operand);
    }
    switch(GET_CODE(operand)){
      /* Assumed PLUS will always be reg + constant */ 
      case PLUS:
        temp = XEXP (operand, 1);
        if(INTVAL(temp) + offset)
          sprintf(ls_fmt,"l%si\t%%0,%%1",
                  ld_mode_text);
        break; 
      case SYMBOL_REF:
        sprintf(ls_fmt,"l%si\t%%0,%%1",
                ld_mode_text);
      
        break;
      case CONST:
      case CONST_INT:
        sprintf(ls_fmt,"l%si\t%%0,%%1",
                ld_mode_text);
        break; 
      case REG:
          sprintf(ls_fmt,"l%si\t%%0,%%1",
                  ld_mode_text);
        break; 
      default:
        fprintf(stderr,"Error!! unknown block move %d\n",GET_CODE(operand));
        return "MicroBlaze Code error here %0 %1 LOAD";
    } /* operands1 switch */
    return ls_fmt;
  }
  else
  {
    /* Get the base register for Memory operations */  
    if(GET_CODE(operand) != PLUS)
      base_reg = get_base_reg(operand);

    switch(GET_CODE(operand)){
      /* Assumed PLUS will always be reg + constant */ 
      case PLUS:
        temp = XEXP (operand, 1);
        if(INTVAL(temp) + offset)
          sprintf(ls_fmt,"s%si\t%%0,%%1",
                  st_mode_text);
        break;
         
      case SYMBOL_REF:
        sprintf(ls_fmt,"s%si\t%%0,%%1",
                st_mode_text);        
        break;
         
      case CONST:
      case CONST_INT:
        sprintf(ls_fmt,"s%si\t%%0,%%1",
                st_mode_text);
        break;    
      case REG:
        sprintf(ls_fmt,"s%si\t%%0,%%1",
                st_mode_text);
        break;
      default:
        fprintf(stderr,"Error!! unknown block move %d\n",GET_CODE(operand));
        return "MicroBlaze Code error here %0 %1 STORE";
    } /* operands0 switch */
    return ls_fmt;
      
  } /* store part*/
}


/* Added to handle Xilinx interrupt handler for MicroBlaze */

int
microblaze_valid_machine_decl_attribute (
  tree decl,
  tree attributes ATTRIBUTE_UNUSED,
  tree attr,
  tree args)
{
  if (args != NULL_TREE)
    return 0;

  if (is_attribute_p ("interrupt_handler", attr) ||
      is_attribute_p ("save_volatiles", attr)){
    /*    fprintf(stderr,"INTERRUPT HANDLER RECOGNIZED\n");*/
    return TREE_CODE (decl) == FUNCTION_DECL;
  }
 
  return 0;
}

/* Return nonzero if FUNC is an interrupt function as specified
   by the "interrupt" attribute.  */

/* Xilinx
 * Eventually remove both the functions below 
 */
static int
microblaze_interrupt_function_p (tree func)
{
  tree a;

  if (TREE_CODE (func) != FUNCTION_DECL)
    return 0;

  a = lookup_attribute ("interrupt_handler", DECL_ATTRIBUTES (func));
  return a != NULL_TREE;
}

static int
microblaze_save_volatiles (tree func)
{
  tree a;

  if (TREE_CODE (func) != FUNCTION_DECL)
    return 0;

  a = lookup_attribute ("save_volatiles", DECL_ATTRIBUTES (func)); 
  return a != NULL_TREE;
}

int microblaze_is_interrupt_handler(void){
  return interrupt_handler;
}

static void
microblaze_globalize_label (
  FILE *stream,
  const char *name)
{									
  fputs ("\t.globl\t", stream);					
  if (interrupt_handler && strcmp (name, INTERRUPT_HANDLER_NAME)){      
    fputs (INTERRUPT_HANDLER_NAME, stream);                          
    fputs ("\n\t.globl\t", stream);                                  
  }									
  assemble_name (stream, name);					
  fputs ("\n", stream);						
} 

/* Returns true if decl should be placed into a "small data" section. */
static bool
microblaze_elf_in_small_data_p (tree decl)
{
  if (!TARGET_XLGP_OPT)
    return false;

  /* We want to merge strings, so we never consider them small data.  */
  if (TREE_CODE (decl) == STRING_CST)
    return false;

  /* Functions are never in the small data area.  */
  if (TREE_CODE (decl) == FUNCTION_DECL)
    return false;

  if (TREE_CODE (decl) == VAR_DECL && DECL_SECTION_NAME (decl))
    {
      const char *section = TREE_STRING_POINTER (DECL_SECTION_NAME (decl));
      if (strcmp (section, ".sdata") == 0
	  || strcmp (section, ".sdata2") == 0
	  || strcmp (section, ".sbss") == 0
	  || strcmp (section, ".sbss2") == 0)
        return true;
    }
  
  HOST_WIDE_INT size = int_size_in_bytes (TREE_TYPE (decl));
  
  return (size > 0 && (unsigned HOST_WIDE_INT) size <= microblaze_section_threshold);
}


/* A C statement or statements to switch to the appropriate section
   for output of RTX in mode MODE.  You can assume that RTX is some
   kind of constant in RTL.  The argument MODE is redundant except in
   the case of a `const_int' rtx.  Select the section by calling
   `text_section' or one of the alternatives for other sections.

   Do not define this macro if you put all constants in the read-only
   data section.  */

static void
microblaze_select_rtx_section (enum machine_mode mode, rtx x, 
                               unsigned HOST_WIDE_INT align)
{
  default_elf_select_rtx_section (mode, x, align);
}

/* A C statement or statements to switch to the appropriate
   section for output of DECL.  DECL is either a `VAR_DECL' node
   or a constant of some sort.  RELOC indicates whether forming
   the initial value of DECL requires link-time relocations.  */

static void
microblaze_select_section (tree decl, int reloc, 
                           unsigned HOST_WIDE_INT align) 
{
  switch (categorize_decl_for_section (decl, reloc, align))
  {
    case SECCAT_RODATA_MERGE_STR:
    case SECCAT_RODATA_MERGE_STR_INIT:
      /* MB binutils have various issues with mergeable string sections and
         relaxation/relocation. Currently, turning mergeable sections 
         into regular readonly sections. */
      readonly_data_section ();
      return;

    default:
      default_elf_select_section (decl, reloc, align);
      return;
  }
}

/*
  Encode info about sections into the RTL based on a symbol's declaration.
  The default definition of this hook, default_encode_section_info in `varasm.c',
  sets a number of commonly-useful bits in SYMBOL_REF_FLAGS. */

static void
microblaze_encode_section_info (tree decl, rtx rtl, int first)
{
  default_encode_section_info (decl, rtl, first);
}

/* Determine of register must be saved/restored in call. */
static int
microblaze_must_save_register (int regno)
{
  if (pic_offset_table_rtx &&
      (regno == MB_ABI_PIC_ADDR_REGNUM) &&
      regs_ever_live[regno]) return 1;

  if (regs_ever_live[regno] && !call_used_regs[regno]) return 1;

  if (frame_pointer_needed && 
      (regno == HARD_FRAME_POINTER_REGNUM)) return 1;

  if (!current_function_is_leaf)
  {
      if (regno == MB_ABI_SUB_RETURN_ADDR_REGNUM) return 1;
      if ((interrupt_handler || save_volatiles) &&
          (regno >= 3 && regno <= 12)) return 1;
  }

  if (interrupt_handler) 
  {
    if ((regs_ever_live[regno]) ||
        (regno == MB_ABI_MSR_SAVE_REG) ||
        (regno == MB_ABI_ASM_TEMP_REGNUM) ||
        (regno == MB_ABI_EXCEPTION_RETURN_ADDR_REGNUM))
       return 1;
  }

  if (save_volatiles) 
  {
    if ((regs_ever_live[regno]) ||
        (regno == MB_ABI_ASM_TEMP_REGNUM) ||
        (regno == MB_ABI_EXCEPTION_RETURN_ADDR_REGNUM))
       return 1;
  }

  return 0;
}

/* Output .ascii string. */
void
output_ascii (FILE *file, const char *string, int len)
{
  register int i, cur_pos = 17;
  fprintf (file, "\t.ascii\t\"");
  for (i = 0; i < len; i++)
  {
    register int c = (unsigned char)string[i];

    switch (c)
    {
      case '\"':
      case '\\':
	putc ('\\', file);
	putc (c, file);
	cur_pos += 2;
	break;

      case '\n':
	fputs ("\\n", file);
	if (i+1 < len
	    && (((c = string[i+1]) >= '\040' && c <= '~')
		  || c == '\t'))
	  cur_pos = 32767;		/* break right here */
	else
	  cur_pos += 2;
	break;

      case '\t':
	fputs ("\\t", file);
	cur_pos += 2;
	break;

      case '\f':
	fputs ("\\f", file);
	cur_pos += 2;
	break;

      case '\b':
	fputs ("\\b", file);
	cur_pos += 2;
	break;

      case '\r':
	fputs ("\\r", file);
	cur_pos += 2;
	break;

      default:
	if (c >= ' ' && c < 0177)
	{
	  putc (c, file);
	  cur_pos++;
	}
	else
	{
	  fprintf (file, "\\%03o", c);
	  cur_pos += 4;
	}
      }

      if (cur_pos > 72 && i+1 < len)
      {
        cur_pos = 17;
        fprintf (file, "\"\n\t.ascii\t\"");
      }
    }
  fprintf (file, "\"\n");
}

static rtx 
expand_pic_symbol_ref(enum machine_mode mode, rtx op)
{
  rtx result;
  result = gen_rtx_UNSPEC (Pmode, gen_rtvec (1, op), UNSPEC_GOTOFF);
  result = gen_rtx_CONST (Pmode, result);
  result = gen_rtx_PLUS (Pmode, pic_offset_table_rtx, result);
  result = gen_const_mem (Pmode, result);
  return result;
}

bool
microblaze_expand_move (enum machine_mode mode, rtx operands[])
{
    /* If we are generating embedded PIC code, and we are referring to a
       symbol in the .text section, we must use an offset from the start
       of the function.  */
    if (TARGET_EMBEDDED_PIC
        && (GET_CODE (operands[1]) == LABEL_REF
	    || (GET_CODE (operands[1]) == SYMBOL_REF
	        && ! SYMBOL_REF_FLAG (operands[1]))))
    {
        rtx temp;

        temp = embedded_pic_offset (operands[1]);
        temp = gen_rtx_PLUS (Pmode, embedded_pic_fnaddr_rtx,
	                force_reg (SImode, temp));
        emit_move_insn (operands[0], force_reg (SImode, temp));
        return true;
    }

    /* If operands[1] is a constant address invalid for pic, then we need to
       handle it just like LEGITIMIZE_ADDRESS does.  */
    if (flag_pic)
    {
      if (GET_CODE (operands[0]) == MEM)
      {
        rtx addr = XEXP (operands[0],0);
        if (GET_CODE (addr) == SYMBOL_REF)
        {
          if (reload_in_progress)
          {
            regs_ever_live[PIC_OFFSET_TABLE_REGNUM] = 1;
          }
          rtx ptr_reg, result;

          addr = expand_pic_symbol_ref (mode, addr);
          ptr_reg = gen_reg_rtx (Pmode);
          emit_move_insn (ptr_reg, addr);
          result = gen_rtx_MEM(mode, ptr_reg);
          operands[0] = result;
        }
      }
      if (GET_CODE (operands[1]) == SYMBOL_REF || GET_CODE (operands[1]) == LABEL_REF)
      {
        rtx result;
        if (reload_in_progress)
        {
          regs_ever_live[PIC_OFFSET_TABLE_REGNUM] = 1;
        }
        result = expand_pic_symbol_ref (mode, operands[1]);
        if (GET_CODE (operands[0]) != REG) {
          rtx ptr_reg = gen_reg_rtx (Pmode);
          emit_move_insn (ptr_reg, result);
          emit_move_insn (operands[0], ptr_reg);
        }
        else
        {
          emit_move_insn (operands[0], result);
        }
        return true;
      }
      else if (GET_CODE (operands[1]) == MEM &&
               GET_CODE (XEXP(operands[1],0)) == SYMBOL_REF)
      {
        rtx result;
        rtx ptr_reg;
        if (reload_in_progress)
        {
          regs_ever_live[PIC_OFFSET_TABLE_REGNUM] = 1;
        }
        result = expand_pic_symbol_ref (mode, XEXP(operands[1],0));

        ptr_reg = gen_reg_rtx (Pmode);

        emit_move_insn (ptr_reg, result);
        result = gen_rtx_MEM(mode, ptr_reg);
        emit_move_insn (operands[0], result);
        return true;
      }
      else if (pic_address_needs_scratch (operands[1]))
      {
        rtx temp = force_reg (SImode, XEXP (XEXP (operands[1], 0), 0));
        rtx temp2 = XEXP (XEXP (operands[1], 0), 1);

        if (reload_in_progress)
          regs_ever_live[PIC_OFFSET_TABLE_REGNUM] = 1;
        /* if (! SMALL_INT (temp2))
	   temp2 = force_reg (SImode, temp2);
        */
        emit_move_insn (operands[0], gen_rtx_PLUS (SImode, temp, temp2));
        return true;
      }
    }

    if ((reload_in_progress | reload_completed) == 0
        && !register_operand (operands[0], SImode)
        && !register_operand (operands[1], SImode)
        && (GET_CODE (operands[1]) != CONST_INT
	    || INTVAL (operands[1]) != 0))
    {
        rtx temp = force_reg (SImode, operands[1]);
        emit_move_insn (operands[0], temp);
        return true;
    }
  return false;
}
