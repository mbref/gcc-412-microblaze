/* Definitions of target machine for GNU compiler, for Xilinx MicroBlaze.
   Copyright (C) 2000, 2001, 2002, 2003, 2004
   Free Software Foundation, Inc.
   Contributed by Richard Kenner (kenner@vlsi1.ultra.nyu.edu)

   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.

   GCC is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with GCC; see the file COPYING.  If not, write to the
   Free Software Foundation, 59 Temple Place - Suite 330, Boston,
   MA 02111-1307, USA.  */

#ifndef __MICROBLAZE_PROTOS__
#define __MICROBLAZE_PROTOS__

#ifdef RTX_CODE
extern void barrel_shift_left_imm(rtx operands[]);
extern void shift_left_imm(rtx operands[]);
extern void shift_right_imm(rtx operands[]);
extern rtx embedded_pic_offset       PARAMS ((rtx));
extern int pic_address_needs_scratch PARAMS ((rtx));
extern void expand_block_move        PARAMS ((rtx *));
extern const char* microblaze_move_1word PARAMS ((  rtx[], rtx, int));
extern void shift_left_imm  PARAMS ((rtx []));
extern void microblaze_expand_prologue (void);
extern void microblaze_expand_epilogue (void);
extern void shift_double_left_imm    PARAMS ((rtx []));
extern void override_options (void);
extern void machine_dependent_reorg PARAMS ((void));
extern rtx microblaze_legitimize_address PARAMS ((rtx , rtx,
                                                  enum machine_mode));
extern rtx microblaze_return_addr_rtx (int count,
                                       rtx frameaddr ATTRIBUTE_UNUSED);
extern int microblaze_const_double_ok PARAMS ((rtx, enum machine_mode));
extern void init_cumulative_args  PARAMS ((CUMULATIVE_ARGS *,tree, rtx));
extern int microblaze_can_use_return_insn PARAMS ((void));
extern void microblaze_order_regs_for_local_alloc PARAMS ((void));
extern HOST_WIDE_INT microblaze_debugger_offset PARAMS ((rtx, HOST_WIDE_INT));
extern void output_ascii  PARAMS ((FILE *, const char *, int));
extern void final_prescan_insn    PARAMS ((rtx, rtx *, int));
extern void print_operand PARAMS ((FILE *, rtx, int));
extern void print_operand_address PARAMS ((FILE *, rtx));
int double_memory_operand                       PARAMS ((rtx, enum machine_mode));
bool microblaze_legitimate_address_p            PARAMS ((enum machine_mode, rtx, int ));
int simple_memory_operand                       PARAMS ((rtx, enum machine_mode));
const char* microblaze_move_2words              PARAMS ((rtx *, rtx));
int microblaze_is_interrupt_handler             PARAMS ((void));
enum reg_class microblaze_secondary_reload_class PARAMS ((enum reg_class, enum machine_mode, rtx, int));
int microblaze_regno_ok_for_base_p              PARAMS ((int, int));
HOST_WIDE_INT microblaze_initial_elimination_offset
                                                PARAMS ((int, int));
void microblaze_declare_object                  PARAMS ((FILE *, char *, char *, char *, int));
#endif  /* RTX_CODE */

#ifdef TREE_CODE
extern void function_arg_advance (CUMULATIVE_ARGS *, enum machine_mode,
				  tree, int);
extern rtx function_arg (CUMULATIVE_ARGS *, enum machine_mode, tree, int);
#endif /* TREE_CODE */

#endif  /* __MICROBLAZE_PROTOS__ */
