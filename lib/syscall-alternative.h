 // from http://vserver.13thfloor.at/Experimental/SYSCALL/syscall_shiny10.h

#ifndef	__SYSCALL_NEW_H
#define	__SYSCALL_NEW_H

/*	Copyright (C) 2005-2006 Herbert Pötzl

		global config options

	__sysc_setret	... set return value (default none)
	__sysc_seterr	... set error value (default errno)


		arch specific config

	__sysc_reg_cid	... the callid (if define), immediate otherwise

	__sysc_reg(n)	... syscall argument registers

	__sysc_reg_res	... simple result register (either that or)
	__sysc_reg_ret	... complex result register (and)
	__sysc_reg_err	... complex error register

	__sysc_cmd_pre	... preparation command(s)	__sysc_pre(n)
	__sysc_cmd_sys	... the syscall			__sysc_cmd(n)
	__sysc_cmd_fin	... syscall finalization	__sysc_fin(n)

	__sysc_clobber	... clobbered registers (default memory)
	__sysc_max_err	... maximum error number (for simple result)

	__sysc_errc(r,e)	... error condition (e cmplx)
	__sysc_retv(t,r,e)	... syscall return value (e cmplx)

		if gcc cannot be trusted

	__sysc_load(r,a)	... asm code for register load
	__sysc_save(r,a)	... asm code to save result(s)
	__sysc_limm(r,i)	... asm code to load immediate
	__sysc_rcon(n)		... syscall register constraint
	__sysc_regs		... list of input regs for clobber
	__sysc_type		... register type
	__sysc_aout		... asm code output constraint

		if all else fails

	__sc_asmload(n,N,...)	... asm code to prepare arguments
	__sc_asmsysc(n,N)	... asm code to execute syscall
	__sc_asmsave(n)		... asm code to store results

*/




/*	*****************************************
	ALPHA	ALPHA	ALPHA	ALPHA		*
	alpha kernel interface 			*/

#if 	defined(__alpha__)

/*	The Alpha calling convention doesn't use the stack until
	after the first six arguments have been passed in registers.

	scnr:	v0($0)
	args:	a1($16), a2($17), a3($18), a4($19), a5($20), a6($21)
	sret:	r0($0)
	serr:	e0($19) (!=0, err=sret)
	call:	callsys
	clob:	memory
*/

#define	__sysc_reg_cid	"$0"
#define	__sysc_reg_ret	"$0"
#define	__sysc_reg_err	"$19"
#define	__sysc_cmd_sys	"callsys"

#define	__sysc_reg(n)	__arg_##n\
	("$16", "$17", "$18", "$19", "$20", "$21")

#define	__sysc_clobber	__sysc_regs,					\
	"$1", "$2", "$3", "$4", "$5", "$6", "$7", "$8",			\
	"$22", "$23", "$24", "$25", "$27", "$28", "memory"		\



/*	*****************************************
	ARM	ARM	ARM	ARM		*
	arm kernel interface 			*/

#elif	defined(__arm__)

/*	The Arm calling convention uses stack args after four arguments
	but the Linux kernel gets up to seven arguments in registers.
	
	scnr:	imm
	args:	a1(r0), a2(r1), a3(r2), a4(r3), a5(r4), a6(r5),
	sret:	r0(r0)
	serr:	(sret >= (unsigned)-EMAXERRNO)
	call:	swi
	clob:	memory
*/

#define	__sysc_max_err	125
#define	__sysc_reg_res	"r0"
#define	__sysc_cmd_sys	"swi	%0"

#define	__sysc_reg(n)	__arg_##n\
	("r0", "r1", "r2", "r3", "r4", "r5")

#define	__sysc_clobber	__sysc_regs, "memory"

#warning syscall arch arm not tested yet



/*	*****************************************
	CRIS	CRIS	CRIS	CRIS		*
	cris v10 kernel interface 		*/

#elif	defined(__cris__)

/*	The Cris calling convention uses stack args after four arguments
	but the Linux kernel gets up to six arguments in registers.

	scnr:	id(r9)
	args:	a1(r10), a2(r11), a3(r12), a4(r13), a5(mof), a6(srp),
	sret:	r0(r10)
	serr:	(sret >= (unsigned)-EMAXERRNO)
	call:	break 13
	clob:	memory
*/

#define	__sysc_max_err	125
#define	__sysc_reg_cid	"r9"
#define	__sysc_reg_res	"r0"
#define	__sysc_cmd_sys	"break 13"

#define	__sysc_regs	"r10", "r11", "r12", "r13"
#define	__sysc_reg(n)	__arg_##n\
	("r10", "r11", "r12", "r13", "r0", "srp")

#define	__sysc_pre(n)	__casm(n,5,0,"move r0,mof",)

#define	__sysc_clobber	__sysc_regs, "memory"

#warning syscall arch cris not tested yet



/*	*****************************************
	FRV	FRV	FRV	FRV		*
	frv kernel interface 		*/

#elif	defined(__frv__)

/*	The C calling convention on FR-V uses the gr8-gr13 registers
	for the first six arguments, the remainder is spilled onto the
	stack. the linux kernel syscall interface does so too.
	
	scnr:	id(gr7)
	args:	a1(gr8), a2(gr9), a3(gr10), a4(gr11), a5(gr12), a6(gr13)
	sret:	r0(gr8)
	serr:	(sret >= (unsigned)-EMAXERRNO)
	call:	tra gr0,gr0
	clob:	memory
*/

#define	__sysc_max_err	125
#define	__sysc_reg_cid	"gr7"
#define	__sysc_reg_res	"gr8"
#define	__sysc_cmd_sys	"tira	gr0,#0"

#define	__sysc_reg(n)	__arg_##n\
	("gr8", "gr9", "gr10", "gr11", "gr12", "gr13")

#define	__sysc_clobber	__sysc_regs, "memory"

#warning syscall arch frv not tested yet



/*	*****************************************
	H8300	H8300	H8300	H8300		*
	h8/300 kernel interface 		*/

#elif	defined(__H8300__)

/*	The H8/300 C calling convention passes the first three
	arguments in registers. However the linux kernel calling
	convention passes the first six arguments in registers
	er1-er6

	scnr:	id(er0)
	args:	a1(er1), a2(er2), a3(er3), a4(er4), a5(er5), a6(er6)
	sret:	r0(er0)
	serr:	(sret >= (unsigned)-EMAXERRNO)
	call:	trapa #0
	clob:	memory
*/

#define	__sysc_max_err	125
#define	__sysc_reg_res	"er0"

#define	__sysc_reg(n)	__arg_##n\
	("er1", "er2", "er3", "er4", "er5", "er6")

#define	__sysc_clobber	"memory"

#define	__sc_asmload(n,N,...)	__sc_asm	(			\
	__casm(n,1,1,	"mov.l	%0,er1"		,			)\
	__casm(n,2,1,	"mov.l	%1,er2"		,			)\
	__casm(n,3,1,	"mov.l	%2,er3"		,			)\
	__casm(n,4,1,	"mov.l	%3,er4"		,			)\
	__casm(n,5,1,	"mov.l	%4,er5"		,			)\
	__casm(n,6,1,	"mov.l	er6,@-sp"	,			)\
	__casm(n,6,1,	"mov.l	%5,er6"		,			)\
	""::__sc_iregs(n,__VA_ARGS__):__sysc_regs)

#define	__sysc_cmd_pre	"mov.l	%0,er0"
#define	__sysc_cmd_sys	"trapa	#0"
#define	__sysc_fin(n)	__casm(n,6,0,"mov.l	@sp+,er6",)

#warning syscall arch h8300 not tested yet



/*	*****************************************
	HPPA	HPPA	HPPA	HPPA		*
	hppa/64 kernel interface 		*/

#elif	defined(__hppa__)

/*	The hppa calling convention uses r26-r23 for the first 4
	arguments, the rest is spilled onto the stack. However the
	Linux kernel passes the first six arguments in the registers
	r26-r21.

	The system call number MUST ALWAYS be loaded in the delay
	slot of the ble instruction, or restarting system calls
	WILL NOT WORK.

	scnr:	id(r20)
	args:	a1(r26), a2(r25), a3(r24), a4(r23), a5(r22), a6(r21)
	sret:	r0(r28)
	serr:	(sret >= (unsigned)-EMAXERRNO)
	call:	ble  0x100(%%sr2, %%r0)
	clob:	r1, r2, (r4), r20, r29, r31, memory
	picr:	pr(r19)
*/

#define	__sysc_max_err	4095
#define	__sysc_reg_res	"r28"

#define	__sysc_reg(n)	__arg_##n\
	("r26", "r25", "r24", "r23", "r22", "r21")

#define	__sysc_cmd_sys	"ble 0x100(%%sr2,%%r0)"

#define	__sysc_pre(n)							\
	__pasm(n,1,1,	"copy %%r19, %%r4"	,			)

#define	__sysc_fin(n)							\
	__casm(n,1,1,	"ldi %0,%%r20"		,			)\
	__pasm(n,1,1,	"copy %%r4, %%r19"	,			)

#ifndef	__PIC__
#define	__sysc_clobber	__sysc_regs,					\
	"r1", "r2", "r20", "r29", "r31", "memory"
#else
#define	__sysc_clobber	__sysc_regs,					\
	"r1", "r2", "r4", "r20", "r29", "r31", "memory"
#endif


/*	*****************************************
	I386	I386	I386	I386		*
	i386 kernel interface 			*/

#elif	defined(__i386__)

/*	The x86 calling convention uses stack args for all arguments,
	but the Linux kernel passes the first six arguments in the
	following registers: ebx, ecx, edx, esi, edi, ebp.
	
	scnr:	id(eax)
	args:	a1(ebx), a2(ecx), a3(edx), a4(esi), a5(edi), a6(ebp) 
	sret:	r0(eax)
	serr:	(sret >= (unsigned)-EMAXERRNO)
	call:	int 0x80
	picr:	pr(ebx)
	clob:	memory
*/

#define	__sysc_max_err	129
#define	__sysc_reg_res	"eax"
#define	__sysc_cmd_sys	"int	$0x80"

#ifndef	__PIC__
#define	__sysc_regs	"ebx", "ecx", "edx", "esi", "edi"
#else
#define	__sysc_regs	"ecx", "edx", "esi", "edi"
#endif

#define	__sc_asmload(n,N,...)	__sc_asm	(			\
	__casm(n,6,1,	"movl	%5,%%eax"	,			)\
	__casm(n,5,1,	"movl	%4,%%edi"	,			)\
	__casm(n,4,1,	"movl	%3,%%esi"	,			)\
	__casm(n,3,1,	"movl	%2,%%edx"	,			)\
	__casm(n,2,1,	"movl	%1,%%ecx"	,			)\
	__pasm(n,1,1,	"pushl	%%ebx"		,			)\
	__casm(n,1,1,	"movl	%0,%%ebx"	,			)\
	__casm(n,6,1,	"pushl	%%ebp"		,			)\
	""::__sc_iregs(n,__VA_ARGS__):__sysc_clobber)

#define	__sc_asmsave(n)

#define	__sysc_pre(n)							\
	__casm(n,6,1,	"movl	%%eax,%%ebp"	,			)\
	__casm(n,0,1,	"movl	%1,%%eax"	,			)\

#define	__sysc_fin(n)							\
	__casm(n,6,1,	"popl	%%ebp"		,			)\
	__pasm(n,1,1,	"popl	%%ebx"		,			)\

#define	__sysc_aout 	"=a"(__res)
#define	__sysc_clobber	__sysc_regs, "memory"


/*	*****************************************
	IA64	IA64	IA64	IA64		*
	ia64 kernel interface 			*/

#elif	defined(__ia64__)

/*	The ia64 calling convention uses out0-out7 to pass the first
	eight arguments (mapped via register windows).

	scnr:	id(r15)
	args:	a1(out0), a2(out1), ... a5(out4), a6(out5)
	sret:	r0(r8)
	serr:	e0(r10)
	call:	break 0x100000
	clob:	out6/7, r2/3/9, r11-r14, r16-r31, p6-p15, f6-f15, b6/7
*/


#define	__sysc_reg_ret	"r8"
#define	__sysc_reg_err	"r10"
#define	__sysc_reg_cid	"r15"
#define	__sysc_cmd_sys	"break.i	0x100000"

#define	__sysc_errc(r,e)	((e) == -1)

#define	__sysc_reg(n)	__arg_##n\
	("out0", "out1", "out2", "out3", "out4", "out5")

#define	__sysc_clobber	__sysc_regs,					\
	"out6", "out7", "r2", "r3", "r9", "r11", "r12", "r13",		\
	"r14", "r16", "r17", "r18", "r19", "r20", "r21", "r22", 	\
	"r23", "r24", "r25", "r26", "r27", "r28", "r29", "r30",		\
	"r31", "p6", "p7", "p8", "p9", "p10", "p11", "p12", "p13",	\
	"p14", "p15", "f6", "f7", "f8", "f9", "f10", "f11", "f12",	\
	"f13", "f14", "f15", "f16", "b6", "b7", "cc", "memory"

#warning syscall arch ia64 not tested yet



/*	*****************************************
	M32R	M32R	M32R	M32R		*
	m32r kernel interface 			*/

#elif	defined(__M32R__)

/*	The m32r calling convention uses r0-r7 to pass the first
	eight arguments (mapped via register windows).

	scnr:	id(r0)
	args:	a1(r1), a2(r2), a3(r3), a4(r4), a5(r5), a6(r6)
	sret:	r0(r0)
	serr:	(sret >= (unsigned)-EMAXERRNO)
	call:	trap #2
	clob:	out6/7, r2/3/9, r11-r14, r16-r31, p6-p15, f6-f15, b6/7
*/

#define	__sysc_max_err	125
#define	__sysc_reg_cid	"r7"
#define	__sysc_reg_res	"r0"
#define	__sysc_cmd_sys	"trap #2"

#define	__sysc_reg(n)	__arg_##n\
	("r0", "r1", "r2", "r3", "r4", "r5")

#define	__sysc_clobber	__sysc_regs, "memory"

#warning syscall arch m32r not tested yet



/*	*****************************************
	M68K	M68K	M68K	M68K		*
	m68k kernel interface 			*/

#elif	defined(__m68000__)

#error syscall arch m68k not implemented yet



/*	*****************************************
	MIPS	MIPS	MIPS	MIPS		*
	mips kernel interface 			*/

#elif	defined(__mips__)

/*	The ABIO32 calling convention uses a0-a3  to pass the first
	four arguments, the rest is passed on the userspace stack.  The 5th arg
	starts at 16($sp).

	ABIN32 and ABI64 pass 6 args in a0-a3, t0-t1.

	scnr:	id(v0)
	args:	a1(a0), a2(a1), a3(a2), a4(a3), a5(16($sp)), a6(20($sp))
	sret:	r0(v0)
	serr:	e0(a3)
	call:	syscall
	clob:	at, v0, t0-t7, t8-t9
*/

#define	__sysc_reg_cid	"v0"
#define	__sysc_reg_ret	"v0"
#define	__sysc_reg_err	"a3"
#define	__sysc_cmd_sys	"syscall"

#define	__sysc_reg(n) __arg_##n\
	("a0","a1","a2","a3", "t0", "t1")

#define	__sysc_clobber "$1", "$3", "$8", "$9", "$10", "$11", "$12",	\
	"$13", "$14", "$15", "$24", "$25", "memory"

#if _MIPS_SIM == _ABIO32
#define	__sysc_pre(n) 							\
	__casm(n,5,1,"addiu $sp,$sp,-32",)				\
	__casm(n,6,1,"sw $9,20($sp)",)					\
	__casm(n,5,1,"sw $8, 16($sp)",)
#define	__sysc_fin(n) 							\
	__casm(n,5,1,"addiu $sp,$sp,32",)
#elif (_MIPS_SIM == _ABIN32) || (_MIPS_SIM == _ABI64)
#warning syscall arch mips with ABI N32 and 64 not tested yet
#else
#error unknown mips ABI version
#endif


/*	*****************************************
	PPC	PPC	PPC	PPC		*
	ppc/64 kernel interface 		*/

#elif	defined(__powerpc__)

/*	The powerpc calling convention uses r3-r10 to pass the first
	eight arguments, the remainder is spilled onto the stack.
	
	scnr:	id(r0)
	args:	a1(r3), a2(r4), a3(r5), a4(r6), a5(r7), a6(r8)
	sret:	r0(r3)
	serr:	(carry)
	call:	sc
	clob:	cr0, ctr
*/


#define	__sysc_reg_cid	"r0"
#define	__sysc_reg_ret	"r3"
#define	__sysc_reg_err	"r0"

#define	__sysc_errc(r,e)	((e) & 0x10000000)

#define	__sysc_reg(n)	__arg_##n\
	("r3", "r4", "r5", "r6", "r7", "r8")

#define	__sysc_cmd_sys	"sc"
#define	__sysc_cmd_fin	"mfcr 0"

#define	__sysc_clobber	__sysc_regs,					\
	"r9", "r10", "r11", "r12", "cr0", "ctr", "memory"



/*	*****************************************
	S390	S390	S390	S390		*
	s390/x kernel interface 		*/

#elif	defined(__s390__)

/*	The s390x calling convention passes the first five arguments
	in r2-r6, the remainder is spilled onto the stack. However
	the Linux kernel passes the first six arguments in r2-r7.
	
	scnr:	imm, id(r1)
	args:	a1(r2), a2(r3), a3(r4), a4(r5), a5(r6), a6(r7)
	sret:	r0(r2)
	serr:	(sret >= (unsigned)-EMAXERRNO)
	call:	svc
	clob:	memory
*/

#define	__sysc_max_err	4095
#define	__sysc_reg_cid	"r1"
#define	__sysc_reg_res	"r2"
#define	__sysc_cmd_sys	"svc	0"

#define	__sysc_regtyp	unsigned long

#define	__sysc_reg(n)	__arg_##n\
	("r2", "r3", "r4", "r5", "r6", "r7")

#define	__sysc_clobber	__sysc_regs, "memory"



/*	*****************************************
	SH	SH	SH	SH		*
	sh kernel interface 			*/

#elif	defined(__sh__) && !defined(__SH5__)

/*	The SuperH calling convention passes the first four arguments
	in r4-r7, the remainder is spilled onto the stack. However
	the Linux kernel passes the remainder in r0-r2.

	scnr:	id(r3)
	args:	a1(r4), a2(r5), a3(r6), a4(r7), a5(r0), a6(r1)
	sret:	r0(r0)
	serr:	(sret >= (unsigned)-EMAXERRNO)
	call:	trapa #0x1x (x=#args)
	clob:	memory
*/

#define	__sysc_max_err	4095
#define	__sysc_reg_cid	"r3"
#define	__sysc_reg_res	"r0"

#define	__sysc_reg(n)	__arg_##n\
	("r4", "r5", "r6", "r7", "r0", "r1")

#define	__sysc_cmd(n)	"trapa	#0x1" __stringify(n)

#define	__rep_6(x)	x x x x x x
#define	__sysc_cmd_fin	__rep_6("or	r0,r0\n\t")

#define	__sysc_clobber	__sysc_regs, "memory"

#warning syscall arch sh not tested yet



/*	*****************************************
	SH64	SH64	SH64	SH64		*
	sh64 kernel interface 			*/

#elif defined(__sh__) && defined(__SH5__)

/*	The SuperH-5 calling convention passes the first eight
	arguments in r2-r9

	scnr:	id(r9)
	args:	a1(r2), a2(r3), a3(r4), a4(r5), a5(r6), a6(r7)
	sret:	r0(r9)
	serr:	(sret >= (unsigned)-EMAXERRNO)
	call:	trapa #0x1x (x=#args)
	clob:	memory
*/

#define	__sysc_max_err	4095
#define	__sysc_reg_res	"r9"
#define	__sysc_cmd_sys	"trapa	r9"

#define	__sysc_reg(n)	__arg_##n\
	("r2", "r3", "r4", "r5", "r6", "r7")

#define	__sc_asmsysc(n,N)	__sc_asm_vol	(			\
	__casm(n,0,1,	"movi %0,r9"		,			)\
	__casm(n,0,1,	__sc_cmds(n,N)		,			)\
	""::"i"(__sc_id(N) | 0x1##n << 16) : __sysc_clobber)

#define	__sysc_clobber	__sysc_regs, "memory"

#warning syscall arch sh64 not tested yet



/*	*****************************************
	SPARC64	SPARC64	SPARC64	SPARC64		*
	sparc64 kernel interface 		*/

#elif	defined(__sparc__)

/*	The sparc/64 calling convention uses o0-o5 to pass the first
	six arguments (mapped via register windows).

	scnr:	id(g1)
	args:	a1(o0), a2(o1), a3(o2), a4(o3), a5(o4), a6(o5)
	sret:	r0(o0)
	serr:	(carry)
	call:	ta 0x6d, t 0x10
	clob:	g1-g6, g7?, o7?, f0-f31, cc
*/

#define	__sysc_max_err	515
#define	__sysc_reg_cid	"g1"
#define	__sysc_reg_ret	"o0"
#define	__sysc_reg_err	"l1"

#define	__sysc_reg(n)	__arg_##n\
	("o0", "o1", "o2", "o3", "o4", "o5")

#ifdef	__arch64__
#define	__sysc_cmd_sys	"ta	0x6d"
#else
#define	__sysc_cmd_sys	"t	0x10"
#endif

#define	__sysc_cmd_fin	"addx	%%g0,%%g0,%%l1"


#define	__sysc_clobber	__sysc_regs,					\
	"g2", "g3", "g4", "g5", "g6",					\
	"f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8",		\
	"f9", "f10", "f11", "f12", "f13", "f14", "f15", "f16",		\
	"f17", "f18", "f19", "f20", "f21", "f22", "f23", "f24",		\
	"f25", "f26", "f27", "f28", "f29", "f30", "f31", "f32",		\
	"f34", "f36", "f38", "f40", "f42", "f44", "f46", "f48",		\
	"f50", "f52", "f54", "f56", "f58", "f60", "f62",		\
	"cc", "memory"

#warning syscall arch sparc not tested yet



/*	*****************************************
	V850	V850	V850	V850		*
	v850 kernel interface 			*/

#elif	defined(__v850__)

/*	The V850 calling convention passes the first four arguments
	in registers r6-r9, the rest is spilled onto the stack.
	but the Linux kernel interface uses r6-r9 and r13/14.

	scnr:	id(r12)
	args:	a1(r6), a2(r7), a3(r8), a4(r9), a5(r13), a6(r14)
	sret:	r0(r10)
	serr:	(sret >= (unsigned)-EMAXERRNO)
	call:	trap 0, trap 1
	clob:	r1, r5, r11, r15-r19
*/

#define	__sysc_max_err	515
#define	__sysc_reg_cid	"r12"
#define	__sysc_reg_res	"r10"

#define	__sysc_reg(n)	__arg_##n\
	("r6", "r7", "r8", "r9", "r13", "r14")

#define	__sysc_cmd(n)	__casm(n,4,0,"trap 1","trap 0")

#define	__sysc_clobber	__sysc_regs,					\
	"r1", "r5", "r11", "r15", "r16", "r17", "r18", "r19", "memory"

#warning syscall arch v850 not tested yet



/*	*****************************************
	X86_64	X86_64	X86_64	X86_64		*
	x86_64 kernel interface 		*/

#elif	defined(__x86_64__)

/*	The x86_64 calling convention uses rdi, rsi, rdx, rcx, r8, r9
	but the Linux kernel interface uses rdi, rsi, rdx, r10, r8, r9.
	
	scnr:	id(rax)
	args:	a1(rdi), a2(rsi), a3(rdx), a4(r10), a5(r8), a6(r9)
	sret:	r0(rax)
	serr:	(err= sret > (unsigned)-EMAXERRNO)
	call:	syscall
	clob:	rcx, r11
*/

#define	__sysc_max_err	4095
#define	__sysc_reg_cid	"rax"
#define	__sysc_reg_res	"rax"
#define	__sysc_cmd_sys	"syscall"

#define	__sysc_reg(n)	__arg_##n\
	("rdi", "rsi", "rdx", "r10", "r8", "r9")

#define	__sysc_clobber	__sysc_regs,					\
	"cc", "r11", "rcx", "memory"

#define	__sysc_aout 	"=a"(__res)

#else
#error unknown kernel arch
#endif





	/* argument list */

#define	__lst_6(x,a1,a2,a3,a4,a5,a6)	__lst_5(x,a1,a2,a3,a4,a5),x(6,a6)
#define	__lst_5(x,a1,a2,a3,a4,a5)	__lst_4(x,a1,a2,a3,a4),x(5,a5)
#define	__lst_4(x,a1,a2,a3,a4)		__lst_3(x,a1,a2,a3),x(4,a4)
#define	__lst_3(x,a1,a2,a3)		__lst_2(x,a1,a2),x(3,a3)
#define	__lst_2(x,a1,a2)		__lst_1(x,a1),x(2,a2)
#define	__lst_1(x,a1)			__lst_0(x,*)x(1,a1)
#define	__lst_0(x,a0)

	/* argument concat */

#define	__con_6(x,a1,a2,a3,a4,a5,a6)	__con_5(x,a1,a2,a3,a4,a5)x(6,a6)
#define	__con_5(x,a1,a2,a3,a4,a5)	__con_4(x,a1,a2,a3,a4)x(5,a5)
#define	__con_4(x,a1,a2,a3,a4)		__con_3(x,a1,a2,a3)x(4,a4)
#define	__con_3(x,a1,a2,a3)		__con_2(x,a1,a2)x(3,a3)
#define	__con_2(x,a1,a2)		__con_1(x,a1)x(2,a2)
#define	__con_1(x,a1)			__con_0(x,*)x(1,a1)
#define	__con_0(x,a0)

	/* argument selection */

#define	__arg_0(...)
#define	__arg_1(a1,...)			a1
#define	__arg_2(a1,a2,...)		a2
#define	__arg_3(a1,a2,a3,...)		a3
#define	__arg_4(a1,a2,a3,a4,...)	a4
#define	__arg_5(a1,a2,a3,a4,a5,...)	a5
#define	__arg_6(a1,a2,a3,a4,a5,a6)	a6



#ifdef	__PIC__
#define	__pic(v)		v
#define	__nopic(v)
#else
#define	__pic(v)
#define	__nopic(v)		v
#endif

#define	__casm_nl(v)		v "\n\t"

#define	__casm(n,a,r,v,w)	__casm_##n##a(v,w,r)

#define	__pasm(n,a,r,v,w)	__pic(__casm(n,a,r,v,w))
#define	__Pasm(n,a,r,v,w)	__nopic(__casm(n,a,r,v,w))

#define	__casm_use(q,r,v)	v __casm_use_##q##r(__casm_nl(""))

#define	__casm_use_10(v)
#define	__casm_use_11(v)	v
#define	__casm_use_12(v)
#define	__casm_use_13(v)	v

#define	__casm_use_20(v)
#define	__casm_use_21(v)
#define	__casm_use_22(v)	v
#define	__casm_use_23(v)	v


#define	__casm_00(v,w,r)	__casm_use(1,r,v)
#define	__casm_01(v,w,r)	__casm_use(2,r,w)
#define	__casm_02(v,w,r)	__casm_use(2,r,w)
#define	__casm_03(v,w,r)	__casm_use(2,r,w)
#define	__casm_04(v,w,r)	__casm_use(2,r,w)
#define	__casm_05(v,w,r)	__casm_use(2,r,w)
#define	__casm_06(v,w,r)	__casm_use(2,r,w)

#define	__casm_10(v,w,r)	__casm_use(1,r,v)
#define	__casm_11(v,w,r)	__casm_use(1,r,v)
#define	__casm_12(v,w,r)	__casm_use(2,r,w)
#define	__casm_13(v,w,r)	__casm_use(2,r,w)
#define	__casm_14(v,w,r)	__casm_use(2,r,w)
#define	__casm_15(v,w,r)	__casm_use(2,r,w)
#define	__casm_16(v,w,r)	__casm_use(2,r,w)

#define	__casm_20(v,w,r)	__casm_use(1,r,v)
#define	__casm_21(v,w,r)	__casm_use(1,r,v)
#define	__casm_22(v,w,r)	__casm_use(1,r,v)
#define	__casm_23(v,w,r)	__casm_use(2,r,w)
#define	__casm_24(v,w,r)	__casm_use(2,r,w)
#define	__casm_25(v,w,r)	__casm_use(2,r,w)
#define	__casm_26(v,w,r)	__casm_use(2,r,w)

#define	__casm_30(v,w,r)	__casm_use(1,r,v)
#define	__casm_31(v,w,r)	__casm_use(1,r,v)
#define	__casm_32(v,w,r)	__casm_use(1,r,v)
#define	__casm_33(v,w,r)	__casm_use(1,r,v)
#define	__casm_34(v,w,r)	__casm_use(2,r,w)
#define	__casm_35(v,w,r)	__casm_use(2,r,w)
#define	__casm_36(v,w,r)	__casm_use(2,r,w)

#define	__casm_40(v,w,r)	__casm_use(1,r,v)
#define	__casm_41(v,w,r)	__casm_use(1,r,v)
#define	__casm_42(v,w,r)	__casm_use(1,r,v)
#define	__casm_43(v,w,r)	__casm_use(1,r,v)
#define	__casm_44(v,w,r)	__casm_use(1,r,v)
#define	__casm_45(v,w,r)	__casm_use(2,r,w)
#define	__casm_46(v,w,r)	__casm_use(2,r,w)

#define	__casm_50(v,w,r)	__casm_use(1,r,v)
#define	__casm_51(v,w,r)	__casm_use(1,r,v)
#define	__casm_52(v,w,r)	__casm_use(1,r,v)
#define	__casm_53(v,w,r)	__casm_use(1,r,v)
#define	__casm_54(v,w,r)	__casm_use(1,r,v)
#define	__casm_55(v,w,r)	__casm_use(1,r,v)
#define	__casm_56(v,w,r)	__casm_use(2,r,w)

#define	__casm_60(v,w,r)	__casm_use(1,r,v)
#define	__casm_61(v,w,r)	__casm_use(1,r,v)
#define	__casm_62(v,w,r)	__casm_use(1,r,v)
#define	__casm_63(v,w,r)	__casm_use(1,r,v)
#define	__casm_64(v,w,r)	__casm_use(1,r,v)
#define	__casm_65(v,w,r)	__casm_use(1,r,v)
#define	__casm_66(v,w,r)	__casm_use(1,r,v)

#define	__casm_cn_0
#define	__casm_cn_1		,
#define	__casm_cn_2		,
#define	__casm_cn_3		,
#define	__casm_cn_4		,
#define	__casm_cn_5		,
#define	__casm_cn_6		,



#define	__sc_asm		__asm__
#define	__sc_asm_vol		__asm__ __volatile__

#ifndef	__sysc_setret
#define	__sysc_setret(v)	do { } while(0)
#endif

#ifndef	__sysc_seterr
#define	__sysc_seterr(e)	do { errno = (e); } while(0)
#endif

#ifndef	__stringify0
#define	__stringify0(val)	#val
#endif

#ifndef	__stringify
#define	__stringify(val)	__stringify0(val)
#endif


#if	!defined(__sysc_load) && !defined(__sysc_save)
#if	!defined(__sysc_limm) && !defined(__sc_asmload)
#define	__sc_trust
#endif
#endif

#if	defined(__sysc_reg_ret) && defined(__sysc_reg_err)
#define	__sc_complex
#endif


#ifndef	__sysc_type
#define	__sysc_type		long
#endif

#define	__sc_cast(v)		(__sysc_type)(v)


#define	__sc_reg(n)		register __sysc_type n
#define	__sc_asm_reg(n,r)	register __sysc_type n __sc_asm (r)
#define	__sc_asm_val(n,r,v)	__sc_asm_reg(n,r) = __sc_cast(v)

#ifndef	__sysc_load
#define	__sc_inp_def(n,v)	__sc_asm_val(__sc_a##n, __sysc_reg(n), v);
#else
#define	__sc_inp_def(n,value)
#endif

#if	!defined(__sysc_save) && !defined(__sysc_aout)
#define	__sc_res_def(n,r)	__sc_asm_reg(n, r);
#else
#define	__sc_res_def(n,r)	__sc_reg(n);
#endif


#define	__sc_rreg(n,v)		"r"(__sc_a##n)
#define	__sc_creg(n,v)		__sysc_rcon(n)(__sc_cast(v))

#ifdef	__sc_trust
#define	__sc_iregs(n,...)	__lst_##n(__sc_rreg,__VA_ARGS__)
#define	__sc_input(n,...)	__con_##n(__sc_inp_def,__VA_ARGS__)
#else
#define	__sc_iregs(n,...)	__lst_##n(__sc_creg,__VA_ARGS__)
#define	__sc_input(n,...)
#endif



#define	__sc_list(x)		x(1), x(2), x(3), x(4), x(5), x(6)

#ifndef	__sysc_regs
#define	__sysc_regs		__sc_list(__sysc_reg)
#endif

#ifndef	__sysc_rcon
#define	__sysc_rcon(n)		"g"
#endif


#ifdef	__sc_complex	/* complex result */

#ifndef	__sysc_errc
#define	__sysc_errc(ret, err) (err)
#endif

#ifndef	__sysc_retv
#define	__sysc_retv(type, ret, err)					\
	__sysc_setret(ret);						\
	if (__sysc_errc(ret, err)) {					\
		int __err = (ret);					\
		__sysc_seterr(__err);					\
		ret = -1;						\
	}								\
	return (type)(ret)
#endif

#define	__sc_results							\
	__sc_res_def(__err, __sysc_reg_err)				\
	__sc_res_def(__ret, __sysc_reg_ret)

#define	__sc_oregs	"=r"(__ret), "=r"(__err)

#if	defined(__sc_trust) || !defined(__sysc_save)
#define	__sc_saveres	__sc_dummy_save(1)
#else
#define	__sc_saveres							\
	__casm_nl(__sysc_save(__sysc_reg_ret,"%0"))			\
	__casm_nl(__sysc_save(__sysc_reg_err,"%1"))
#endif

#define	__sc_return(t)	__sysc_retv(t, __ret, __err)

#else			/* simple result  */

#ifndef	__sysc_errc
#define	__sysc_errc(res)						\
	((unsigned __sysc_type)(res) >= 				\
		(unsigned __sysc_type)(-(__sysc_max_err)))
#endif

#ifndef	__sysc_retv
#define	__sysc_retv(type, res)						\
	__sysc_setret(res);						\
	if (__sysc_errc(res)) {						\
		int __err = -(res);					\
		__sysc_seterr(__err);					\
		res = -1;						\
	}								\
	return (type)(res)
#endif


#define	__sc_results							\
	__sc_res_def(__res, __sysc_reg_res)

#define	__sc_oregs	"=r"(__res)

#if	defined(__sc_trust) || !defined(__sysc_save)
#define	__sc_saveres	__sc_dummy_save(0)
#else
#define	__sc_saveres	__casm_nl(__sysc_save(__sysc_reg_res,"%0"))
#endif

#define	__sc_return(t)	__sysc_retv(t, __res)

#endif			/* simple/complex */


#define	__sc_dummy_load(n)	"/* gcc dummy load " 			\
	__casm(n,0,0,"%0 ",) __casm(n,1,0,"%1 ",) __casm(n,2,0,"%2 ",)	\
	__casm(n,3,0,"%3 ",) __casm(n,4,0,"%4 ",) __casm(n,5,0,"%5 ",)	\
	__casm(n,6,0,"%6 ",) "*/"

#ifdef	__sysc_aout
#define	__sc_dummy_save(n)
#define __sc_asmsave(n)
#else
#define	__sc_dummy_save(n)	"/* gcc dummy save " 			\
	__casm(n,0,0,"%0 ",) __casm(n,1,0,"%1 ",) "*/"
#endif

#define	__comment(name)		"\t/* kernel sys_" 			\
	#name "[" __stringify(__sc_id(name)) "] */"


#define	__sc_id(N)		__NR_##N

#ifndef	__sysc_reg_cid
#define	__sc_cid(N)		"i"(__sc_id(N))
#define	__sc_load_cid		""
#define	__sc_callid(N)
#else
#define	__sc_cid(N)		"r"(__cid)
#define	__sc_load_cid		__sysc_limm(__sysc_reg_cid,"%0")
#define	__sc_callid(N)							\
	__sc_asm_val(__cid, __sysc_reg_cid, __sc_id(N));
#endif

#ifndef	__sysc_cmd_pre
#define	__sc_cmd_pre		""
#else
#define	__sc_cmd_pre		__casm_nl(__sysc_cmd_pre)
#endif

#ifndef	__sysc_cmd_fin
#define	__sc_cmd_fin		""
#else
#define	__sc_cmd_fin		__sysc_cmd_fin
#endif

#ifndef	__sysc_pre
#define	__sysc_pre(n)		__sc_cmd_pre
#endif

#ifndef	__sysc_cmd
#define	__sysc_cmd(n)		__sysc_cmd_sys
#endif

#ifndef	__sysc_fin
#define	__sysc_fin(n)		__sc_cmd_fin
#endif

#define	__sc_cmds(n,name)						\
	__sysc_pre(n)							\
	__casm_nl(__sysc_cmd(n) __comment(name))			\
	__sysc_fin(n)

#ifndef	__sc_asmload
#ifdef	__sc_trust
#define	__sc_asmload(n,N,...)	__sc_asm(				\
	__sc_dummy_load(n)						\
	::__sc_cid(N) __casm_cn_##n __sc_iregs(n,__VA_ARGS__))
#else
#define	__sc_asmload(n,N,...)	__sc_asm(				\
	__casm(n,1,1,	__sysc_load(__sysc_reg(1),"%1"),		)\
	__casm(n,2,1,	__sysc_load(__sysc_reg(2),"%2"),		)\
	__casm(n,3,1,	__sysc_load(__sysc_reg(3),"%3"),		)\
	__casm(n,4,1,	__sysc_load(__sysc_reg(4),"%4"),		)\
	__casm(n,5,1,	__sysc_load(__sysc_reg(5),"%5"),		)\
	__casm(n,6,1,	__sysc_load(__sysc_reg(6),"%6"),		)\
	__sc_load_cid	::__sc_cid(N) __casm_cn_##n			\
	__sc_iregs(n,__VA_ARGS__):__sysc_regs)
#endif
#endif

#ifndef	__sysc_aout
#define	__sysc_aout
#endif

#ifndef	__sc_asmsysc
#define	__sc_asmsysc(n,N)	__sc_asm_vol(				\
	__casm(n,0,0,	__sc_cmds(n,N)		,			)\
	:__sysc_aout:"i"(__sc_id(N)) : __sysc_clobber)
#endif

#ifndef	__sc_asmsave
#define	__sc_asmsave(n)		__sc_asm(				\
	__sc_saveres		:__sc_oregs)
#endif




#define	__sc_body(n, type, name, ...)					\
{									\
	__sc_results __sc_callid(name) __sc_input(n, __VA_ARGS__)	\
	__sc_asmload(n, name, __VA_ARGS__);				\
	__sc_asmsysc(n, name);						\
	__sc_asmsave(n);						\
	__sc_return(type);						\
}



#define	_syscall0(type, name)						\
type name(void)								\
__sc_body(0, type, name, *)

#define	_syscall1(type, name, type1, arg1)				\
type name(type1 arg1)							\
__sc_body(1, type, name, arg1)

#define	_syscall2(type, name, type1, arg1, type2, arg2)			\
type name(type1 arg1, type2 arg2)					\
__sc_body(2, type, name, arg1, arg2)

#define	_syscall3(type, name, type1, arg1, type2, arg2, type3, arg3)	\
type name(type1 arg1, type2 arg2, type3 arg3)				\
__sc_body(3, type, name, arg1, arg2, arg3)

#define	_syscall4(type, name, type1, arg1, type2, arg2, type3, arg3,	\
			      type4, arg4)				\
type name(type1 arg1, type2 arg2, type3 arg3, type4 arg4)		\
__sc_body(4, type, name, arg1, arg2, arg3, arg4)

#define	_syscall5(type, name, type1, arg1, type2, arg2, type3, arg3,	\
			      type4, arg4, type5, arg5)			\
type name(type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5)	\
__sc_body(5, type, name, arg1, arg2, arg3, arg4, arg5)

#define	_syscall6(type, name, type1, arg1, type2, arg2, type3, arg3,	\
			type4, arg4, type5, arg5, type6, arg6)		\
type name(type1 arg1, type2 arg2, type3 arg3,				\
	  type4 arg4, type5 arg5, type6 arg6)				\
__sc_body(6, type, name, arg1, arg2, arg3, arg4, arg5, arg6)


#endif	/* __SYSCALL_NEW_H */
