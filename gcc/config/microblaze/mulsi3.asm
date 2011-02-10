###################################-*-asm*- 
# 
# Copyright (c) 2001 Xilinx, Inc.  All rights reserved. 
# 
# Xilinx, Inc. CONFIDENTIAL 
# 
# mulsi3.asm 
# 
# Multiply operation for 32 bit integers.
#	Input :	Operand1 in Reg r5
#		Operand2 in Reg r6
#	Output: Result [op1 * op2] in Reg r3
# 
# $Header: /devl/xcs/repo/env/Jobs/MDT/sw/ThirdParty/gnu/src/gcc/src-3.4/gcc/config/microblaze/mulsi3.s,v 1.1.2.6 2005/11/15 23:32:47 salindac Exp $
# 
#######################################

	.globl	__mulsi3
	.ent	__mulsi3
__mulsi3:
	.frame	r1,0,r15
	add	r3,r0,r0
	BEQI	r5,$L_Result_Is_Zero      # Multiply by Zero
	BEQI	r6,$L_Result_Is_Zero      # Multiply by Zero
	BGEId	r5,$L_R5_Pos 
	XOR	r4,r5,r6                  # Get the sign of the result
	RSUBI	r5,r5,0	                  # Make r5 positive
$L_R5_Pos:
	BGEI	r6,$L_R6_Pos
	RSUBI	r6,r6,0	                  # Make r6 positive
$L_R6_Pos:	
	bri	$L1
$L2:	
	add	r5,r5,r5
$L1:	
	srl	r6,r6
	addc	r7,r0,r0
	beqi	r7,$L2
	bneid	r6,$L2
	add	r3,r3,r5	
	blti	r4,$L_NegateResult			
	rtsd	r15,8
	nop
$L_NegateResult:
	rtsd	r15,8
	rsub	r3,r3,r0
$L_Result_Is_Zero:
	rtsd	r15,8
	addi	r3,r0,0
	.end __mulsi3
	