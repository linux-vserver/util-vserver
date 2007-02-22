 // from http://vserver.13thfloor.at/Experimental/SYSCALL/syscall_shiny17.h

#ifndef	__SYSCALL_NEW_H
#define	__SYSCALL_NEW_H

/*	Copyright (C) 2005-2007 Herbert Pötzl

		global config options

	__sysc_seterr	... set error value (def: errno)
	__sysc_cid(N)	... syscall 'name' id (def: __NR_<N>)

		arch specific config

	__sysc_regs	... the syscall registers (asm load)
	__sysc_cmd(n)	... the syscall
	__sysc_reg_cid	... syscall id register (asm load)
	__sysc_reg_ret	... syscall return register (asm out)
	__sysc_reg_err	... syscall error register (asm out)

	__sysc_clbrs	... the clobbered syscall registers
	__sysc_clobber	... clobbered registers (def: memory)
	__sysc_max_err	... maximum error number (def: separate)
	__sysc_errc(r,e)... error condition (def: e)

	__sysc_type	... type of syscall arguments (def: long)
	__sysc_acon(n)	... argument constraint (def: "r")
	__sysc_con_cid	... syscall id constraint (def: "i"/"r")
	__sysc_con_ret	... return value contraint (def: "=r")
	__sysc_con_err	... error value contraint (def: "=r")

		hard core replacements

	__sc_body(n,type,name,...)
	  __sc_results
	  __sc_cidvar(N)
	  __sc_input(n,...)
	  __sc_syscall(n,N,...)
	  __sc_return(t)

*/

	/* some fallback defaults */

#ifndef	__sysc_seterr
#define	__sysc_seterr(e)	do { errno = (e); } while(0)
#endif

#ifndef	__sysc_cid
#define	__sysc_cid(N)		__NR_##N
#endif


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
	move:	mov $sR,$dR
	picr:	pr($29) do we need to save that?
*/

#define	__sysc_cmd(n)	"callsys"

#define	__sysc_reg_cid	"$0"
#define	__sysc_con_cid	"v"
#define	__sysc_reg_ret	"$0"
#define	__sysc_con_ret	"=v"
#define	__sysc_reg_err	"$19"

#define	__sysc_regs	"$16", "$17", "$18", "$19", "$20", "$21"
#define	__sysc_clbrs	"$16", "$17", "$18", "memory", "$20", "$21"
#define	__sysc_clobber	"$1", "$2", "$3", "$4", "$5", "$6", "$7", "$8",	\
			"$22", "$23", "$24", "$25", "$27", "$28", "memory"


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
	move:	mov $dR,$sR
*/

#define	__sysc_max_err	125

#define	__sysc_cmd(n)	"swi	%1"

#define	__sysc_regs	"r0", "r1", "r2", "r3", "r4", "r5"
#define	__sysc_reg_ret	"r0"

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

#error syscall arch cris not implemented yet



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

#error syscall arch frv not implemented yet



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

#error syscall arch h8300 not implemented yet



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
	picr:	pr(r19) do we need to save that?
*/

#define	__sysc_max_err	4095

#define	__sysc_cmd(n)	\
	__pasm(n,1,1,	"copy %%r19, %%r4"	,)\
	__casm(n,0,1,	"ble 0x100(%%sr2,%%r0)"	,)\
	__casm(n,0,1,	"ldi %1,%%r20"		,)\
	__pasm(n,1,1,	"copy %%r4, %%r19"	,)

#define	__sysc_regs	"r26", "r25", "r24", "r23", "r22", "r21"

#ifndef	__PIC__
#define	__sysc_clobber	"r1", "r2", "r20", "r29", "r31", "memory"
#else
#define	__sysc_clobber	"r1", "r2", "r4", "r20", "r29", "r31", "memory"
#endif

#warning syscall arch hppa not tested yet



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
	move:	movl $sR,$dR
*/

#define	__sysc_max_err	129

#define	__sc_reg1(...) __sc_cast(__arg_1(__VA_ARGS__,,,,,,))
#define	__sc_reg6(...) __sc_cast(__arg_6(__VA_ARGS__,,,,,,))

#define	__scsd	struct { __sc_ldef(__a); __sc_ldef(__b); } __scs
#define	__scsa(n,...) \
	__scs.__a = __sc_reg1(__VA_ARGS__);	\
	__scs.__b = __sc_reg6(__VA_ARGS__);

#define	__sc_input(n,...) __casm(n,6,0,		\
	__scsd; __scsa(n,__VA_ARGS__),	)

#define	__cm	,
#define	__sc_null(n)	__arg_##n(		\
	__cm,__cm,__cm,__cm,__cm,__cm)

#define	__sc_rvcs(r,v)	r (__sc_cast(v))

#define	__sc_rvrd(n,N)	__arg_##n(,		\
	__cm	__sc_rvcs("c", N),		\
	__cm	__sc_rvcs("d", N),		\
	__cm	__sc_rvcs("S", N),		\
	__cm	__sc_rvcs("D", N),)

#define	__sc_arg1(n,...) __Casm(n,1,6,0,,	\
	__sc_rvcs(__pic("ri") __nopic("b"),	\
	__sc_reg1(__VA_ARGS__)),		\
	__sc_rvcs("0", &__scs))

#define	__sc_syscall(n,N,...) \
	__sc_asm_vol (__sysc_cmd(n)		\
	  : __sc_oregs				\
	  : __sc_cidval(N) __sc_null(n)		\
	    __sc_arg1(n,__VA_ARGS__)		\
	    __con_##n(__sc_rvrd,__VA_ARGS__)	\
	  : "memory" )

#define	__sysc_cmd(n)	\
	__pasm(n,1,1,	"pushl	%%ebx"		,)\
	__Pasm(n,1,5,1,,"movl	%2, %%ebx"	,)\
	__casm(n,6,1,	"pushl	%%ebp"		,)\
	__casm(n,6,1,	"movl	0(%2), %%ebx"	,)\
	__casm(n,6,1,	"movl	4(%2), %%ebp"	,)\
	__casm(n,0,1,	"movl	%1, %%eax"	,)\
	__casm(n,0,1,	"int	$0x80"		,)\
	__casm(n,6,1,	"popl	%%ebp"		,)\
	__pasm(n,1,1,	"popl	%%ebx"		,)

#define	__sysc_reg_ret	"eax"
#define	__sysc_con_ret	"=a"



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
	move:	mov %dR = %sR
*/

#define	__sysc_errc(r,e)	((e) == -1)

#define	__sysc_cmd(n)	"break.i 0x100000"

#define	__sysc_regs	"out0", "out1", "out2", "out3", "out4", "out5"
#define	__sysc_reg_cid	"r15"
#define	__sysc_reg_ret	"r8"
#define	__sysc_reg_err	"r10"

#define	__sysc_clobber	\
	"out6", "out7", "r2", "r3", "r9", "r11", "r12", "r13",		\
	"r14", "r16", "r17", "r18", "r19", "r20", "r21", "r22",		\
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
	move:	mv %dR,%sR
*/

#define	__sysc_max_err	125

#define	__sysc_cmd(n)	"trap #2"

#define	__sysc_regs	"r0", "r1", "r2", "r3", "r4", "r5"
#define	__sysc_reg_cid	"r7"
#define	__sysc_reg_ret	"r0"

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
	four arguments, the rest is passed on the userspace stack.  
	The 5th arg starts at 16($sp). The new mips calling abi uses 
	registers a0-a5, restart requires a reload of v0 (#syscall)

	ABIN32 and ABI64 pass 6 args in a0-a3, t0-t1.

	scnr:	id(v0)
	args:	a1(a0), a2(a1), a3(a2), a4(a3), a5(t0), a6(t1)
	sret:	r0(v0)
	serr:	e0(a3)
	call:	syscall
	clob:	at, v1, t2-t7, t8-t9
	move:	move	%dR,%sR
*/

#define	__sysc_cmd(n)	\
	__casm(n,0,1,	"ori	$v0,$0,%2"	,)\
	__casm(n,0,1,	"syscall"		,)

#define	__sysc_regs	"a0","a1","a2","a3", "t0", "t1"
#define	__sysc_reg_ret	"v0"
#define	__sysc_reg_err	"a3"

#define	__sysc_clobber	"$1", "$3", "$10", "$11", "$12",		\
			"$13", "$14", "$15", "$24", "$25", "memory"

#warning syscall arch mips not tested yet



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
	clob:	r9-r12, cr0, ctr
	move:	mr %dR,%sR
*/

#define	__sysc_errc(r,e)	((e) & 0x10000000)

#define	__sysc_cmd(n)	\
	__casm(n,0,1,	"sc"			,)\
	__casm(n,0,1,	"mfcr %1"		,)

#define	__sysc_regs	"r3", "r4", "r5", "r6", "r7", "r8"
#define	__sysc_reg_cid	"r0"
#define	__sysc_reg_ret	"r3"

#define	__sysc_clobber	"r9", "r10", "r11", "r12", "cr0", "ctr", "memory"



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

#define	__sysc_cmd(n)	"svc	0"

// #define	__sysc_type	unsigned long

#define	__sysc_regs	"r2", "r3", "r4", "r5", "r6", "r7"
#define	__sysc_reg_cid	"r1"
#define	__sysc_reg_ret	"r2"

#warning syscall arch s390 not tested yet



/*	*****************************************
	SH	SH	SH	SH		*
	sh kernel interface 			*/

#elif	defined(__sh__) && !defined(__SH5__)

/*	The SuperH calling convention passes the first four arguments
	in r4-r7, the remainder is spilled onto the stack. However
	the Linux kernel passes the remainder in r0-r1.

	scnr:	id(r3)
	args:	a1(r4), a2(r5), a3(r6), a4(r7), a5(r0), a6(r1)
	sret:	r0(r0)
	serr:	(sret >= (unsigned)-EMAXERRNO)
	call:	trapa #0x1x (x=#args)
	clob:	memory
	move:	ori	%sR,0,%dR
*/

#ifdef	__sh2__
#define	__sysc_arch	"trapa	#0x2"
#else
#define	__sysc_arch	"trapa	#0x1"
#endif

#define	__sysc_max_err	4095

#define	__sysc_cmd(n)	__sysc_arch #n

#define	__sysc_regs	"r4", "r5", "r6", "r7", "r0", "r1"
#define	__sysc_reg_cid	"r3"
#define	__sysc_reg_ret	"r0"

#warning syscall arch sh not tested yet



/*	*****************************************
	SH64	SH64	SH64	SH64		*
	sh64 kernel interface 			*/

#elif defined(__sh__) && defined(__SH5__)

/*	The SuperH-5 calling convention passes the first eight
	arguments in r2-r9. The Linux kernel uses only six of
	them as arguments, and the last one for the syscall id.

	scnr:	id(r9)
	args:	a1(r2), a2(r3), a3(r4), a4(r5), a5(r6), a6(r7)
	sret:	r0(r9)
	serr:	(sret >= (unsigned)-EMAXERRNO)
	call:	trapa #0x1x (x=#args)
	clob:	memory
	move:	ori	%sR,0,%dR
*/

#define	__sysc_max_err	4095

#define	__sysc_cmd(n)	\
	__casm(n,0,1,	"movi	0x1" #n ",r9"	,)\
	__casm(n,0,1,	"shori	%1,r9"		,)\
	__casm(n,0,1,	"trapa	r9"		,)

#define	__sysc_regs	"r2", "r3", "r4", "r5", "r6", "r7"
#define	__sysc_reg_ret	"r9"

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
	move:	mov	%sR,%dR
*/

#ifdef	__arch64__
#define	__sysc_arch	"ta	0x6d"
#else
#define	__sysc_arch	"ta	0x10"
#endif

#define	__sysc_cmd(n)	\
	__casm(n,0,1,	__sysc_arch		,)\
	__casm(n,0,1,	"addx	%%g0,%%g0,%1"	,)

#define	__sysc_regs	"o0", "o1", "o2", "o3", "o4", "o5"
#define	__sysc_reg_cid	"g1"
#define	__sysc_reg_ret	"o0"

#define	__sysc_clobber	"g2", "g3", "g4", "g5", "g6",			\
	"f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8",		\
	"f9", "f10", "f11", "f12", "f13", "f14", "f15", "f16",		\
	"f17", "f18", "f19", "f20", "f21", "f22", "f23", "f24",		\
	"f25", "f26", "f27", "f28", "f29", "f30", "f31", "f32",		\
	"f34", "f36", "f38", "f40", "f42", "f44", "f46", "f48",		\
	"f50", "f52", "f54", "f56", "f58", "f60", "f62",		\
	"cc", "memory"



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

#define	__sysc_cmd(n)	\
	__casm(n,4,0,	"trap 1"	,"trap 0"	)

#define	__sysc_regs	"r6", "r7", "r8", "r9", "r13", "r14"
#define	__sysc_reg_cid	"r12"
#define	__sysc_reg_ret	"r10"

#define	__sysc_clobber	"r1", "r5", "r11",				\
			"r15", "r16", "r17", "r18", "r19", "memory"

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

#define	__sysc_cmd(n)	"syscall"

#define	__sysc_regs	"rdi", "rsi", "rdx", "r10", "r8", "r9"
#define	__sysc_reg_cid	"rax"
#define	__sysc_reg_ret	"rax"
#define	__sysc_con_ret	"=a"

#define	__sysc_clobber	"cc", "r11", "rcx", "memory"

#else
#error unknown kernel arch
#endif

	
	/* implementation defaults */



#ifndef	__sysc_clobber
#define	__sysc_clobber		"memory"
#endif

#ifndef	__sysc_acon
#define	__sysc_acon(n)		"r"
#endif

#ifndef	__sysc_con_ret
#define	__sysc_con_ret		"=r"
#endif

#ifndef	__sysc_con_err
#define	__sysc_con_err		"=r"
#endif

#ifndef	__sysc_con_cid
#ifdef	__sysc_reg_cid
#define	__sysc_con_cid		"r"
#else
#define	__sysc_con_cid		"i"
#endif
#endif

#ifndef	__sysc_type
#define	__sysc_type		long
#endif

#ifdef	__sysc_regs
#define	__sysc_rega(n,...)	__arg_##n(__VA_ARGS__)
#ifndef	__sysc_reg
#define	__sysc_reg(n)		__sysc_rega(n,__sysc_regs)
#endif
#endif



	/* argument list */

#define	__lst_6(x,a1,a2,a3,a4,a5,a6)	__lst_5(x,a1,a2,a3,a4,a5),x(6,a6)
#define	__lst_5(x,a1,a2,a3,a4,a5)	__lst_4(x,a1,a2,a3,a4),x(5,a5)
#define	__lst_4(x,a1,a2,a3,a4)		__lst_3(x,a1,a2,a3),x(4,a4)
#define	__lst_3(x,a1,a2,a3)		__lst_2(x,a1,a2),x(3,a3)
#define	__lst_2(x,a1,a2)		__lst_1(x,a1),x(2,a2)
#define	__lst_1(x,a1)			__lst_0(x,*),x(1,a1)
#define	__lst_0(x,a0)

	/* argument concatenation */

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
#define	__arg_6(a1,a2,a3,a4,a5,a6,...)	a6

	/* list remainder */

#define	__rem_0(a1,a2,a3,a4,a5,a6)	,a1,a2,a3,a4,a5,a6
#define	__rem_1(a1,a2,a3,a4,a5,a6)	,a2,a3,a4,a5,a6
#define	__rem_2(a1,a2,a3,a4,a5,a6)	,a3,a4,a5,a6
#define	__rem_3(a1,a2,a3,a4,a5,a6)	,a4,a5,a6
#define	__rem_4(a1,a2,a3,a4,a5,a6)	,a5,a6
#define	__rem_5(a1,a2,a3,a4,a5,a6)	,a6
#define	__rem_6(...)


	/* conditional asm */

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


	/* special PIC handling */

#ifdef	__PIC__
#define	__pic(v)		v
#define	__nopic(v)
#else
#define	__pic(v)
#define	__nopic(v)		v
#endif

#define	__casm_nl(v)		v "\n\t"

#define	__casm(n,a,r,v,w)	__casm_##n##a(v,w,r)
#define	__Casm(n,a,b,r,u,v,w)	__casm_##n##b(w,__casm_##n##a(v,u,r),r)

#define	__pasm(n,a,r,v,w)	__pic(__casm(n,a,r,v,w))
#define	__Pasm(n,a,b,r,u,v,w)	__pic(__Casm(n,a,b,r,u,v,w))

#define	__nasm(n,a,r,v,w)	__nopic(__casm(n,a,r,v,w))
#define	__Nasm(n,a,b,r,u,v,w)	__nopic(__Casm(n,a,b,r,u,v,w))


#define	__sc_cast(v)		(__sysc_type)(v)
#define	__sc_ldef(N)		__sysc_type N
#define	__sc_rdef(N,R)		register __sc_ldef(N) __sc_asm (R)

#define	__sc_scid(N,v)		__sc_ldef(N) = __sc_cast(v)
#define	__sc_areg(N,R,v)	__sc_rdef(N,R) = __sc_cast(v)

#define	__sc_rval(n,v)		"r"(__sc_a##n)
#define	__sc_ival(n,v)		__sysc_acon(n)(__sc_cast(v))
#define	__sc_idef(n,v)		__sc_areg(__sc_a##n, __sysc_reg(n), v);

#ifdef	__sysc_clbrs
#define	__sc_cregs(n,...)	__rem_##n(__VA_ARGS__)
#else
#define	__sc_cregs(n,...)
#endif

#ifdef	__sysc_regs
#define	__sc_input(n,...)	__con_##n(__sc_idef,__VA_ARGS__)
#define	__sc_ivals(n,...)	__lst_##n(__sc_rval,__VA_ARGS__)
#else
#define	__sc_ivals(n,...)	__lst_##n(__sc_ival,__VA_ARGS__)
#endif

#ifdef	__sysc_reg_cid
#define	__sc_cidvar(N)		__sc_areg(__sc_id, \
				__sysc_reg_cid, __sysc_cid(N))
#define	__sc_cidval(N)		__sysc_con_cid (__sc_id)
#endif

#ifndef	__sc_input
#define	__sc_input(n,...)
#endif

#ifndef	__sc_cidval
#define	__sc_cidval(N)		__sysc_con_cid (__sysc_cid(N))
#endif

#ifndef	__sc_cidvar
#define	__sc_cidvar(N)
#endif


#ifdef	__sysc_reg_ret
#define	__sc_ret	__ret
#define	__sc_def_ret	__sc_ldef(ret); __sc_rdef(__sc_ret,__sysc_reg_ret)
#else
#define	__sc_ret	ret
#define	__sc_def_ret	__sc_ldef(__sc_ret)
#endif

#ifdef	__sysc_reg_err
#define	__sc_err	__err
#define	__sc_def_err	__sc_ldef(err); __sc_rdef(__sc_err,__sysc_reg_err)
#else
#define	__sc_err	err
#define	__sc_def_err	__sc_ldef(__sc_err)
#endif


#ifndef	__sysc_max_err
#define	__sc_complex
#endif

#ifdef	__sc_complex	/* complex result */

#ifndef	__sc_results
#define	__sc_results	__sc_def_ret; __sc_def_err
#endif

#ifndef	__sysc_errc
#define	__sysc_errc(ret, err) (err)
#endif

#ifndef	__sysc_retv
#define	__sysc_retv(type, ret, err)					\
	if (__sysc_errc(ret, err)) {					\
		__sysc_seterr(ret);					\
		ret = -1;						\
	}								\
	return (type)(ret)
#endif

#define	__sc_oregs	__sysc_con_ret (__sc_ret),			\
			__sysc_con_err (__sc_err)
#ifndef	__sc_return
#define	__sc_return(t)	ret = __sc_ret; err = __sc_err;			\
			__sysc_retv(t, ret, err)
#endif
#else			/* simple result  */

#ifndef	__sc_results
#define	__sc_results	__sc_def_ret
#endif

#ifndef	__sysc_errc
#define	__sysc_errc(ret)						\
	((unsigned __sysc_type)(ret) >= 				\
		(unsigned __sysc_type)(-(__sysc_max_err)))
#endif

#ifndef	__sysc_retv
#define	__sysc_retv(type, ret)						\
	if (__sysc_errc(ret)) {						\
		__sysc_seterr(-ret);					\
		ret = -1;						\
	}								\
	return (type)(ret)
#endif

#define	__sc_oregs	__sysc_con_ret (__sc_ret)
#ifndef	__sc_return
#define	__sc_return(t)	ret = __sc_ret; __sysc_retv(t, ret)
#endif
#endif			/* simple/complex */



	/* the inline syscall */

#define	__sc_asm	__asm__
#define	__sc_asm_vol	__asm__ __volatile__

#ifndef	__sc_syscall
#define	__sc_syscall(n,N,...)						\
	__sc_asm_vol (__sysc_cmd(n)					\
	  : __sc_oregs							\
	  : __sc_cidval(N) __sc_ivals(n,__VA_ARGS__)			\
	  : __sysc_clobber __sc_cregs(n,__sysc_clbrs))
#endif

#ifndef	__sc_body
#define	__sc_body(n, type, name, ...)					\
{									\
	__sc_results;__sc_cidvar(name);					\
	__sc_input(n,__VA_ARGS__)					\
	__sc_syscall(n,name,__VA_ARGS__);				\
	__sc_return(type);						\
}
#endif

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
