###################################-*-asm*- 
# 
# Copyright (c) 2001 Xilinx, Inc.  All rights reserved. 
# 
# Xilinx, Inc. CONFIDENTIAL 
# 
# umodsi3.asm 
#
# Unsigned modulo operation for 32 bit integers.
#	Input :	op1 in Reg r5
#		op2 in Reg r6
#	Output: op1 mod op2 in Reg r3
# 
# Unsigned mod operation.
# 
# $Header: /devl/xcs/repo/env/Jobs/MDT/sw/ThirdParty/gnu/src/gcc/src-3.4/gcc/config/microblaze/umodsi3.s,v 1.1.2.6 2005/11/15 23:32:47 salindac Exp $
# 
#######################################
	
	.globl	__umodsi3
	.ent	__umodsi3
	.type	__umodsi3,@function
__umodsi3:
	.frame	r1,0,r15	

	addik	r1,r1,-12
	swi	r29,r1,0
	swi	r30,r1,4
	swi	r31,r1,8

	BEQI	r6,$LaDiv_By_Zero         # Div_by_Zero   # Division Error
	BEQId	r5,$LaResult_Is_Zero     # Result is Zero 
	ADDIK 	r3,r0,0                  # Clear div
	ADDIK 	r30,r0,0     	# clear mod
	ADDIK 	r29,r0,32       # Initialize the loop count

# Check if r6 and r5 are equal # if yes, return 0
	rsub 	r18,r5,r6
	beqi	r18,$LaRETURN_HERE

# Check if (uns)r6 is greater than (uns)r5. In that case, just return r5
	xor	r18,r5,r6
	bgeid	r18,16
	addik	r3,r5,0
	blti	r6,$LaRETURN_HERE
	bri	$LCheckr6
	rsub	r18,r5,r6 # MICROBLAZEcmp
	bgti	r18,$LaRETURN_HERE

# If r6 [bit 31] is set, then return result as r5-r6
$LCheckr6:
	bgtid	r6,$LaDIV0
	addik	r3,r0,0
	addik	r18,r0,0x7fffffff
	and	r5,r5,r18
	and 	r6,r6,r18
	brid	$LaRETURN_HERE
	rsub	r3,r6,r5
# First part: try to find the first '1' in the r5
$LaDIV0:
	BLTI	r5,$LaDIV2
$LaDIV1:
	ADD 	r5,r5,r5     # left shift logical r5
	BGEID 	r5,$LaDIV1   #
	ADDIK 	r29,r29,-1
$LaDIV2:
	ADD 	r5,r5,r5     # left shift logical  r5 get the '1' into the Carry
	ADDC 	r3,r3,r3     # Move that bit into the Mod register
	rSUB 	r31,r6,r3    # Try to subtract (r3 a r6)
	BLTi 	r31,$LaMOD_TOO_SMALL
	OR  	r3,r0,r31    # Move the r31 to mod since the result was positive
	ADDIK 	r30,r30,1
$LaMOD_TOO_SMALL:
	ADDIK 	r29,r29,-1
	BEQi 	r29,$LaLOOP_END
	ADD 	r30,r30,r30 # Shift in the '1' into div
	BRI 	$LaDIV2     # Div2
$LaLOOP_END:
	BRI 	$LaRETURN_HERE
$LaDiv_By_Zero:
$LaResult_Is_Zero:
	or 	r3,r0,r0   # set result to 0
$LaRETURN_HERE:
# Restore values of CSRs and that of r3 and the divisor and the dividend
	lwi 	r29,r1,0
	lwi 	r30,r1,4
	lwi 	r31,r1,8
	rtsd 	r15,8
	addik 	r1,r1,12
.end __umodsi3
	.size	__umodsi3, . - __umodsi3
