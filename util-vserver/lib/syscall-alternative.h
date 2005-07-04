
#ifndef __syscall_retval
#define __syscall_retval(v)	do { } while(0)
#endif

#ifndef __syscall_error
#define __syscall_error(e) 	do { errno = (e); } while(0)
#endif

#define	__check(pos, reg)						\
	".ifnc " pos ", " reg "\n\t"					\
	".err\n\t"							\
	".endif\n\t"

#ifndef __stringify0
#define __stringify0(val)	#val
#endif

#ifndef __stringify
#define __stringify(val)	__stringify0(val)
#endif

#define	__comment(name)							\
	"\t/* kernel sys_" #name "[" __stringify(__NR_##name) "] */"


/*	*****************************************
 	ALPHA	ALPHA	ALPHA	ALPHA		*
	alpha kernel interface 			*/
	
#if 	defined(__alpha__)

/*  	The Alpha calling convention doesn't use the stack until 
    	after the first six arguments have been passed in registers.
	
	scnr:	v0($0)
	args:	a0($16), a1($17), a2($18), a3($19), a4($20), a5($21)
	sret:	v0($0)
	serr:	a3($19) (!=0, err=sret)
*/


#define __syscall_return(type, ret, err) do {				\
    	__syscall_retval(ret);						\
    	if (err) {							\
    	    	int __err = (ret);					\
		__syscall_error(__err);					\
		ret = -1;						\
	}								\
	return (type) ret;						\
} while (0)				

#define __syscall_regdef(name, reg)					\
	register long __sc_##name __asm__ (reg)

#define __syscall_regval(name, reg, val)				\
	register long __sc_##name __asm__ (reg) = (long)(val)
    	
#define __syscall_clobbers						\
	"$1", "$2", "$3", "$4", "$5", "$6", "$7", "$8",			\
	"$22", "$23", "$24", "$25", "$27", "$28", "memory" 		\


#define _syscall0(type, name)						\
type name(void)								\
{									\
	long __sc_ret, __sc_err;					\
	{								\
		__syscall_regval(v0, "$0", __NR_##name);		\
		__syscall_regdef(a3, "$19");				\
									\
		__asm__ __volatile__ (					\
		__check("%0%1%2", "$0$19$0")				\
			"callsys" __comment(name)			\
		      : "=r"(__sc_v0), "=r"(__sc_a3)			\
		      : "0"(__sc_v0)					\
		      : "$16", "$17", "$18", "$20", "$21",		\
		      	__syscall_clobbers				\
		);							\
		__sc_ret = __sc_v0;					\
		__sc_err = __sc_a3;					\
	}								\
	__syscall_return(type, __sc_ret, __sc_err);			\
}

#define _syscall1(type, name, type1, arg1)				\
type name(type1 arg1)							\
{									\
	long __sc_ret, __sc_err;					\
	{								\
		__syscall_regval(v0, "$0", __NR_##name);		\
		__syscall_regval(a0, "$16", arg1);			\
		__syscall_regdef(a3, "$19");				\
									\
		__asm__ __volatile__ (					\
		__check("%0%1%2%3", "$0$19$0$16")			\
			"callsys" __comment(name)			\
		      : "=r"(__sc_v0), "=r"(__sc_a3)			\
		      : "0"(__sc_v0), 					\
			"r"(__sc_a0)					\
		      : "$17", "$18", "$20", "$21",			\
		      	__syscall_clobbers				\
		);							\
		__sc_ret = __sc_v0;					\
		__sc_err = __sc_a3;					\
	}								\
	__syscall_return(type, __sc_ret, __sc_err);			\
}

#define _syscall2(type, name, type1, arg1, type2, arg2)			\
type name(type1 arg1, type2 arg2)					\
{									\
	long __sc_ret, __sc_err;					\
	{								\
		__syscall_regval(v0, "$0", __NR_##name);		\
		__syscall_regval(a0, "$16", arg1);			\
		__syscall_regval(a1, "$17", arg2);			\
		__syscall_regdef(a3, "$19");				\
									\
		__asm__ __volatile__ (					\
		__check("%0%1%2%3%4", "$0$19$0$16$17")			\
			"callsys" __comment(name)			\
		      : "=r"(__sc_v0), "=r"(__sc_a3)			\
		      : "0"(__sc_v0), 					\
			"r"(__sc_a0), "r"(__sc_a1)			\
		      : "$18", "$20", "$21",				\
		      	__syscall_clobbers				\
		);							\
		__sc_ret = __sc_v0;					\
		__sc_err = __sc_a3;					\
	}								\
	__syscall_return(type, __sc_ret, __sc_err);			\
}

#define _syscall3(type, name, type1, arg1, type2, arg2, type3, arg3)	\
type name(type1 arg1, type2 arg2, type3 arg3)				\
{									\
	long __sc_ret, __sc_err;					\
	{								\
		__syscall_regval(v0, "$0", __NR_##name);		\
		__syscall_regval(a0, "$16", arg1);			\
		__syscall_regval(a1, "$17", arg2);			\
		__syscall_regval(a2, "$18", arg3);			\
		__syscall_regdef(a3, "$19");				\
									\
		__asm__ __volatile__ (					\
		__check("%0%1%2%3%4%5", "$0$19$0$16$17$18")		\
			"callsys" __comment(name)			\
		      : "=r"(__sc_v0), "=r"(__sc_a3)			\
		      : "0"(__sc_v0), 					\
			"r"(__sc_a0), "r"(__sc_a1), "r"(__sc_a2)	\
		      : "$20", "$21",					\
		      	__syscall_clobbers				\
		);							\
		__sc_ret = __sc_v0;					\
		__sc_err = __sc_a3;					\
	}								\
	__syscall_return(type, __sc_ret, __sc_err);			\
}

#define _syscall4(type, name, type1, arg1, type2, arg2, type3, arg3,	\
			      type4, arg4) 				\
type name (type1 arg1, type2 arg2, type3 arg3, type4 arg4)		\
{									\
	long __sc_ret, __sc_err;					\
	{								\
		__syscall_regval(v0, "$0", __NR_##name);		\
		__syscall_regval(a0, "$16", arg1);			\
		__syscall_regval(a1, "$17", arg2);			\
		__syscall_regval(a2, "$18", arg3);			\
		__syscall_regval(a3, "$19", arg4);			\
									\
		__asm__ __volatile__ (					\
		__check("%0%1%2%3%4%5%6", "$0$19$0$16$17$18$19")	\
			"callsys" __comment(name)			\
		      : "=r"(__sc_v0), "=r"(__sc_a3)			\
		      : "0"(__sc_v0), 					\
			"r"(__sc_a0), "r"(__sc_a1), "r"(__sc_a2),	\
			"1"(__sc_a3)					\
		      : "$20", "$21",					\
		      	__syscall_clobbers				\
		);							\
		__sc_ret = __sc_v0;					\
		__sc_err = __sc_a3;					\
	}								\
	__syscall_return(type, __sc_ret, __sc_err);			\
} 

#define _syscall5(type, name, type1, arg1, type2, arg2, type3, arg3,	\
    			      type4, arg4, type5, arg5)			\
type name (type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5)	\
{									\
	long __sc_ret, __sc_err;					\
	{								\
		__syscall_regval(v0, "$0", __NR_##name);		\
		__syscall_regval(a0, "$16", arg1);			\
		__syscall_regval(a1, "$17", arg2);			\
		__syscall_regval(a2, "$18", arg3);			\
		__syscall_regval(a3, "$19", arg4);			\
		__syscall_regval(a4, "$20", arg5);			\
									\
		__asm__ __volatile__ (					\
		__check("%0%1%2%3%4%5%6%7", "$0$19$0$16$17$18$19$20")	\
			"callsys" __comment(name)			\
		      : "=r"(__sc_v0), "=r"(__sc_a3)			\
		      : "0"(__sc_v0), 					\
			"r"(__sc_a0), "r"(__sc_a1), "r"(__sc_a2),	\
			"1"(__sc_a3), "r"(__sc_a4)			\
		      : "$21",						\
		      	__syscall_clobbers				\
		);							\
		__sc_ret = __sc_v0;					\
		__sc_err = __sc_a3;					\
	}								\
	__syscall_return(type, __sc_ret, __sc_err);			\
}

#define _syscall6(type, name, type1, arg1, type2, arg2, type3, arg3,	\
			      type4, arg4, type5, arg5, type6, arg6)	\
type name (type1 arg1, type2 arg2, type3 arg3,				\
	   type4 arg4, type5 arg5, type6 arg6)				\
{									\
	long __sc_ret, __sc_err;					\
	{								\
		__syscall_regval(v0, "$0", __NR_##name);		\
		__syscall_regval(a0, "$16", arg1);			\
		__syscall_regval(a1, "$17", arg2);			\
		__syscall_regval(a2, "$18", arg3);			\
		__syscall_regval(a3, "$19", arg4);			\
		__syscall_regval(a4, "$20", arg5);			\
		__syscall_regval(a5, "$21", arg6);			\
									\
		__asm__ __volatile__ (					\
		__check("%0%1%2%3%4%5%6%7%8",				\
			"$0$19$0$16$17$18$19$20$21")			\
			"callsys" __comment(name)			\
		      : "=r"(__sc_v0), "=r"(__sc_a3)			\
		      : "0"(__sc_v0), 					\
			"r"(__sc_a0), "r"(__sc_a1), "r"(__sc_a2),	\
			"1"(__sc_a3), "r"(__sc_a4), "r"(__sc_a5)	\
		      : __syscall_clobbers				\
		);							\
		__sc_ret = __sc_v0;					\
		__sc_err = __sc_a3;					\
	}								\
	__syscall_return(type, __sc_ret, __sc_err);			\
}



/*	*****************************************
 	ARM	ARM	ARM	ARM		*
	arm kernel interface 			*/

#elif	defined(__arm__)

/*  	The Arm calling convention uses stack args after four arguments
    	but the Linux kernel gets up to seven arguments in registers.
	
	scnr:	imm
	args:	a1(r0), a2(r1), a3(r2), a4(r3), v1(r4), v2(r5), 
	sret:	a1(r0)
	serr:	(err= sret > (unsigned)-EMAXERRNO)
*/

#ifndef EMAXERRNO
#define EMAXERRNO   125
#endif

#define __syscall_errcon(res)						\
    	((unsigned long)(res) >= (unsigned long)(-EMAXERRNO))

#define __syscall_return(type, res) do {				\
    	__syscall_retval(res);						\
    	if (__syscall_errcon(res)) {					\
    	    	int __err = -(res);					\
		__syscall_error(__err);					\
		res = -1;						\
	}								\
	return (type) res;						\
} while (0)				

#define __syscall_regdef(name, reg)					\
	register int __sc_##name __asm__ (reg)

#define __syscall_regval(name, reg, val)				\
	register int __sc_##name __asm__ (reg) = (int)(val)


#define _syscall0(type, name)						\
type name(void)								\
{									\
	long __sc_res;							\
	{								\
		__syscall_regdef(a1, "r0");				\
									\
 	 	__asm__ __volatile__ (  	      			\
		__check("%0", "r0")					\
 	 		"swi %1" __comment(name)			\
 	 	      : "=r"(__sc_a1)					\
 	 	      : "i"(__NR_##name)	      			\
 	 	      : "memory"					\
	    	);  				      			\
 	 	__sc_res = __sc_a1;		      			\
    	}								\
	__syscall_return(type, __sc_res);				\
}

#define _syscall1(type, name, type1, arg1)				\
type name(type1 arg1)							\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(a1, "r0", arg1);			\
									\
 	 	__asm__ __volatile__ (  	      			\
		__check("%0%2", "r0r0")					\
 	 		"swi %1" __comment(name)			\
 	 	      : "=r"(__sc_a1)					\
 	 	      : "i"(__NR_##name),				\
		      	"0"(__sc_a1)		      			\
 	 	      : "memory"					\
	    	);  				      			\
 	 	__sc_res = __sc_a1;		      			\
    	}								\
	__syscall_return(type, __sc_res);				\
}

#define _syscall2(type, name, type1, arg1, type2, arg2)			\
type name(type1 arg1, type2 arg2)					\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(a1, "r0", arg1);			\
		__syscall_regval(a2, "r1", arg2);			\
									\
 	 	__asm__ __volatile__ (  	      			\
		__check("%0%2%3", "r0r0r1")				\
 	 		"swi %1" __comment(name)			\
 	 	      : "=r"(__sc_a1)					\
 	 	      : "i"(__NR_##name),				\
		      	"0"(__sc_a1), "r"(__sc_a2)     			\
 	 	      : "memory"					\
	    	);  				      			\
 	 	__sc_res = __sc_a1;		      			\
    	}								\
	__syscall_return(type, __sc_res);				\
}

#define _syscall3(type, name, type1, arg1, type2, arg2, type3, arg3)	\
type name(type1 arg1, type2 arg2, type3 arg3)				\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(a1, "r0", arg1);			\
		__syscall_regval(a2, "r1", arg2);			\
		__syscall_regval(a3, "r2", arg3);			\
									\
 	 	__asm__ __volatile__ (  	      			\
		__check("%0%2%3%4", "r0r0r1r2")				\
 	 		"swi %1" __comment(name)			\
 	 	      : "=r"(__sc_a1)					\
 	 	      : "i"(__NR_##name),				\
		      	"0"(__sc_a1), "r"(__sc_a2), "r"(__sc_a3)	\
 	 	      : "memory"					\
	    	);  				      			\
 	 	__sc_res = __sc_a1;		      			\
    	}								\
	__syscall_return(type, __sc_res);				\
}

#define _syscall4(type, name, type1, arg1, type2, arg2, type3, arg3,	\
			      type4, arg4) 				\
type name (type1 arg1, type2 arg2, type3 arg3, type4 arg4)		\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(a1, "r0", arg1);			\
		__syscall_regval(a2, "r1", arg2);			\
		__syscall_regval(a3, "r2", arg3);			\
		__syscall_regval(a4, "r3", arg4);			\
									\
 	 	__asm__ __volatile__ (  	      			\
		__check("%0%2%3%4%5", "r0r0r1r2r3")			\
 	 		"swi %1" __comment(name)			\
 	 	      : "=r"(__sc_a1)					\
 	 	      : "i"(__NR_##name),				\
		      	"0"(__sc_a1), "r"(__sc_a2), "r"(__sc_a3),	\
		      	"r"(__sc_a4)					\
 	 	      : "memory"					\
	    	);  				      			\
 	 	__sc_res = __sc_a1;		      			\
    	}								\
	__syscall_return(type, __sc_res);				\
} 

#define _syscall5(type, name, type1, arg1, type2, arg2, type3, arg3,	\
    			      type4, arg4, type5, arg5)			\
type name (type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5)	\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(a1, "r0", arg1);			\
		__syscall_regval(a2, "r1", arg2);			\
		__syscall_regval(a3, "r2", arg3);			\
		__syscall_regval(a4, "r3", arg4);			\
		__syscall_regval(v1, "r4", arg5);			\
									\
 	 	__asm__ __volatile__ (  	      			\
		__check("%0%2%3%4%5%6", "r0r0r1r2r3r4")			\
 	 		"swi %1" __comment(name)			\
 	 	      : "=r"(__sc_a1)					\
 	 	      : "i"(__NR_##name),				\
		      	"0"(__sc_a1), "r"(__sc_a2), "r"(__sc_a3),	\
		      	"r"(__sc_a4), "r"(__sc_v1)			\
 	 	      : "memory"					\
	    	);  				      			\
 	 	__sc_res = __sc_a1;		      			\
    	}								\
	__syscall_return(type, __sc_res);				\
}

#define _syscall6(type, name, type1, arg1, type2, arg2, type3, arg3,	\
			      type4, arg4, type5, arg5, type6, arg6)	\
type name (type1 arg1, type2 arg2, type3 arg3,				\
	   type4 arg4, type5 arg5, type6 arg6)				\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(a1, "r0", arg1);			\
		__syscall_regval(a2, "r1", arg2);			\
		__syscall_regval(a3, "r2", arg3);			\
		__syscall_regval(a4, "r3", arg4);			\
		__syscall_regval(v1, "r4", arg5);			\
		__syscall_regval(v2, "r5", arg6);			\
									\
 	 	__asm__ __volatile__ (  	      			\
		__check("%0%2%3%4%5%6%7", "r0r0r1r2r3r4r5")		\
 	 		"swi %1" __comment(name)			\
 	 	      : "=r"(__sc_a1)					\
 	 	      : "i"(__NR_##name),				\
		      	"0"(__sc_a1), "r"(__sc_a2), "r"(__sc_a3),	\
		      	"r"(__sc_a4), "r"(__sc_v1), "r"(__sc_v2)	\
 	 	      : "memory"					\
	    	);  				      			\
 	 	__sc_res = __sc_a1;		      			\
    	}								\
	__syscall_return(type, __sc_res);				\
}


/*	*****************************************
 	CRIS	CRIS	CRIS	CRIS		*
	cris v10 kernel interface 		*/

#elif	defined(__cris__)

/*  	The Cris calling convention uses stack args after four arguments
    	but the Linux kernel gets up to six arguments in registers.
	
	scnr:	(r9)
	args:	(r10), (r11), (r12), (r13), (mof), (srp), 
	sret:	(r10)
	serr:	(err= sret > (unsigned)-EMAXERRNO)
*/

#ifndef EMAXERRNO
#define EMAXERRNO   125
#endif

#define __syscall_errcon(res)						\
    	((unsigned long)(res) >= (unsigned long)(-EMAXERRNO))

#define __syscall_return(type, res) do {				\
    	__syscall_retval(res);						\
    	if (__syscall_errcon(res)) {					\
    	    	int __err = -(res);					\
		__syscall_error(__err);					\
		res = -1;						\
	}								\
	return (type) res;						\
} while (0)				

#define __syscall_regdef(name, reg)					\
	register long __sc_##name __asm__ (reg)

#define __syscall_regval(name, reg, val)				\
	register long __sc_##name __asm__ (reg) = (long)(val)

#define _syscall0(type, name)						\
type name(void)								\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(a0, "r9", __NR_##name);		\
		__syscall_regdef(a1, "r10");				\
									\
 	 	__asm__ __volatile__ (  	      			\
 		__check("%0%1", "$r10$r9") 	  			\
 	 		"break 13" __comment(name)			\
 	 	      : "=r"(__sc_a1)					\
 	 	      : "r"(__sc_a0)					\
 	 	      : "memory", "srp", "r13", "r12", "r11"		\
	    	);  				      			\
 	 	__sc_res = __sc_a1;		      			\
    	}								\
	__syscall_return(type, __sc_res);				\
}

#define _syscall1(type, name, type1, arg1)				\
type name(type1 arg1)							\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(a0, "r9", __NR_##name);		\
		__syscall_regval(a1, "r10", arg1);			\
									\
 	 	__asm__ __volatile__ (  	      			\
 		__check("%0%1%2", "$r10$r9$r10")   			\
 	 		"break 13" __comment(name)			\
 	 	      : "=r"(__sc_a1)					\
 	 	      : "r"(__sc_a0),					\
			"0"(__sc_a1)					\
 	 	      : "memory", "srp", "r13", "r12", "r11"		\
	    	);  				      			\
 	 	__sc_res = __sc_a1;		      			\
    	}								\
	__syscall_return(type, __sc_res);				\
}

#define _syscall2(type, name, type1, arg1, type2, arg2)			\
type name(type1 arg1, type2 arg2)					\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(a0, "r9", __NR_##name);		\
		__syscall_regval(a1, "r10", arg1);			\
		__syscall_regval(a2, "r11", arg2);			\
									\
 	 	__asm__ __volatile__ (  	      			\
 		__check("%0%1%2%3", "$r10$r9$r10$r11")   		\
 	 		"break 13" __comment(name)			\
 	 	      : "=r"(__sc_a1)					\
 	 	      : "r"(__sc_a0),					\
			"0"(__sc_a1), "r"(__sc_a2)			\
 	 	      : "memory", "srp", "r13", "r12"			\
	    	);  				      			\
 	 	__sc_res = __sc_a1;		      			\
    	}								\
	__syscall_return(type, __sc_res);				\
}

#define _syscall3(type, name, type1, arg1, type2, arg2, type3, arg3)	\
type name(type1 arg1, type2 arg2, type3 arg3)				\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(a0, "r9", __NR_##name);		\
		__syscall_regval(a1, "r10", arg1);			\
		__syscall_regval(a2, "r11", arg2);			\
		__syscall_regval(a3, "r12", arg3);			\
									\
 	 	__asm__ __volatile__ (  	      			\
 		__check("%0%1%2%3%4", "$r10$r9$r10$r11$r12")	   	\
 	 		"break 13" __comment(name)			\
 	 	      : "=r"(__sc_a1)					\
 	 	      : "r"(__sc_a0),					\
			"0"(__sc_a1), "r"(__sc_a2), "r"(__sc_a3)	\
 	 	      : "memory", "srp", "r13"				\
	    	);  				      			\
 	 	__sc_res = __sc_a1;		      			\
    	}								\
	__syscall_return(type, __sc_res);				\
}

#define _syscall4(type, name, type1, arg1, type2, arg2, type3, arg3,	\
			      type4, arg4) 				\
type name (type1 arg1, type2 arg2, type3 arg3, type4 arg4)		\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(a0, "r9", __NR_##name);		\
		__syscall_regval(a1, "r10", arg1);			\
		__syscall_regval(a2, "r11", arg2);			\
		__syscall_regval(a3, "r12", arg3);			\
		__syscall_regval(a4, "r13", arg4);			\
									\
 	 	__asm__ __volatile__ (  	      			\
 		__check("%0%1%2%3%4%5", "$r10$r9$r10$r11$r12$r13")   	\
 	 		"break 13" __comment(name)			\
 	 	      : "=r"(__sc_a1)					\
 	 	      : "r"(__sc_a0),					\
 	 		"0"(__sc_a1), "r"(__sc_a2), "r"(__sc_a3),	\
		      	"r"(__sc_a4)					\
 	 	      : "memory", "srp"					\
	    	);  				      			\
 	 	__sc_res = __sc_a1;		      			\
    	}								\
	__syscall_return(type, __sc_res);				\
} 

#define _syscall5(type, name, type1, arg1, type2, arg2, type3, arg3,	\
    			      type4, arg4, type5, arg5)			\
type name (type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5)	\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(a0, "r9", __NR_##name);		\
		__syscall_regval(a1, "r10", arg1);			\
		__syscall_regval(a2, "r11", arg2);			\
		__syscall_regval(a3, "r12", arg3);			\
		__syscall_regval(a4, "r13", arg4);			\
									\
 	 	__asm__ __volatile__ (  	      			\
		__check("%0%1%2%3%4%5", "$r10$r9$r10$r11$r12$r13")    	\
			"move %6,$mof\n\t"				\
 	 		"break 13" __comment(name)			\
 	 	      : "=r"(__sc_a1)					\
 	 	      : "r"(__sc_a0),					\
 	 		"0"(__sc_a1), "r"(__sc_a2), "r"(__sc_a3),	\
		      	"r"(__sc_a4), 					\
			"g"((long)arg5)					\
 	 	      : "memory", "srp"					\
	    	);  				      			\
 	 	__sc_res = __sc_a1;		      			\
    	}								\
	__syscall_return(type, __sc_res);				\
}

#define _syscall6(type, name, type1, arg1, type2, arg2, type3, arg3,	\
			      type4, arg4, type5, arg5, type6, arg6)	\
type name (type1 arg1, type2 arg2, type3 arg3,				\
	   type4 arg4, type5 arg5, type6 arg6)				\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(a0, "r9", __NR_##name);		\
		__syscall_regval(a1, "r10", arg1);			\
		__syscall_regval(a2, "r11", arg2);			\
		__syscall_regval(a3, "r12", arg3);			\
		__syscall_regval(a4, "r13", arg4);			\
									\
 	 	__asm__ __volatile__ (  	      			\
		__check("%0%1%2%3%4%5", "$r10$r9$r10$r11$r12$r13")    	\
			"move %6,$mof\n\t"				\
			"move %7,$srp\n\t"				\
 	 		"break 13" __comment(name)			\
 	 	      : "=r"(__sc_a1)					\
 	 	      : "r"(__sc_a0),					\
 	 		"0"(__sc_a1), "r"(__sc_a2), "r"(__sc_a3),	\
		      	"r"(__sc_a4),					\
			"g"((long)arg5), "g"((long)arg6)		\
 	 	      : "memory"					\
	    	);  				      			\
 	 	__sc_res = __sc_a1;		      			\
    	}								\
	__syscall_return(type, __sc_res);				\
}


/*	*****************************************
 	FRV	FRV	FRV	FRV		*
	frv kernel interface 		*/

#elif	defined(__frv__)

#warning syscall arch frv not implemented yet


/*	*****************************************
 	H8300	H8300	H8300	H8300		*
	h8/300 kernel interface 		*/

#elif	defined(__h8300__)

#warning syscall arch h8300 not implemented yet


/*	*****************************************
 	I386	I386	I386	I386		*
	i386 kernel interface 			*/

#elif	defined(__i386__)

/*  	The x86 calling convention uses stack args for all arguments,
    	but the Linux kernel passes the first six arguments in the
	following registers: ebx, ecx, edx, esi, edi, ebp.
	
	scnr:	a0(eax)
	args:	a1(ebx), a2(ecx), a3(edx), a4(esi), a5(edi), a6(ebp) 
	sret:	a0(eax)
	serr:	(err= sret > (unsigned)-EMAXERRNO)
*/

#ifndef EMAXERRNO
#define EMAXERRNO   129
#endif

#define __syscall_errcon(res)						\
	((unsigned long)(res) >= (unsigned long)(-EMAXERRNO))

#define __syscall_return(type, res) do {				\
	__syscall_retval(res);						\
	if (__syscall_errcon(res)) {					\
		int __err = -(res);					\
		__syscall_error(__err);					\
		res = -1;						\
	}								\
	return (type) res;						\
} while (0)				

#define __syscall_regdef(name, reg)					\
	register long __sc_##name __asm__ (reg)

#define __syscall_regval(name, reg, val)				\
	register long __sc_##name __asm__ (reg) = (long)(val)


#define _syscall0(type, name)						\
type name(void)								\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(a0, "eax", __NR_##name);		\
									\
		__asm__ volatile (					\
                __check("%0%1", "%%eax%%eax")	 	  	   	\
			"int $0x80" __comment(name)			\
		      : "=a"(__sc_a0)					\
		      : "0"(__sc_a0)					\
		      : "memory"					\
		);							\
		__sc_res = __sc_a0;					\
	}								\
	__syscall_return(type, __sc_res);				\
}

#define _syscall1(type, name, type1, arg1)				\
type name(type1 arg1)							\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(a0, "eax", __NR_##name);		\
		__syscall_regval(a1, "ebx", arg1);			\
									\
		__asm__ volatile (					\
                __check("%0%1%2", "%%eax%%eax%%ebx") 	  	   	\
			"int $0x80" __comment(name)			\
		      : "=a"(__sc_a0)					\
		      : "0"(__sc_a0),					\
			"r" (__sc_a1)					\
		      : "memory"					\
		);							\
		__sc_res = __sc_a0;					\
	}								\
	__syscall_return(type, __sc_res);				\
}

#define _syscall2(type, name, type1, arg1, type2, arg2)			\
type name(type1 arg1, type2 arg2)					\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(a0, "eax", __NR_##name);		\
		__syscall_regval(a1, "ebx", arg1);			\
		__syscall_regval(a2, "ecx", arg2);			\
									\
		__asm__ volatile (					\
                __check("%0%1%2%3", "%%eax%%eax%%ebx%%ecx") 	     	\
			"int $0x80" __comment(name)			\
		      : "=a"(__sc_a0)					\
		      : "0"(__sc_a0),					\
			"r" (__sc_a1), "r"(__sc_a2)			\
		      : "memory"					\
		);							\
		__sc_res = __sc_a0;					\
	}								\
	__syscall_return(type, __sc_res);				\
}

#define _syscall3(type, name, type1, arg1, type2, arg2, type3, arg3)	\
type name(type1 arg1, type2 arg2, type3 arg3)				\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(a0, "eax", __NR_##name);		\
		__syscall_regval(a1, "ebx", arg1);			\
		__syscall_regval(a2, "ecx", arg2);			\
		__syscall_regval(a3, "edx", arg3);			\
									\
		__asm__ volatile (					\
                __check("%0%1%2%3%4", "%%eax%%eax%%ebx%%ecx%%edx")     	\
			"int $0x80" __comment(name)			\
		      : "=a"(__sc_a0)					\
		      : "0"(__sc_a0),					\
			"r" (__sc_a1), "r"(__sc_a2), "r" (__sc_a3)	\
		      : "memory"					\
		);							\
		__sc_res = __sc_a0;					\
	}								\
	__syscall_return(type, __sc_res);				\
} 


#define _syscall4(type, name, type1, arg1, type2, arg2, type3, arg3,	\
			      type4, arg4)				\
type name (type1 arg1, type2 arg2, type3 arg3, type4 arg4)		\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(a0, "eax", __NR_##name);		\
		__syscall_regval(a1, "ebx", arg1);			\
		__syscall_regval(a2, "ecx", arg2);			\
		__syscall_regval(a3, "edx", arg3);			\
		__syscall_regval(a4, "esi", arg4);			\
									\
		__asm__ volatile (					\
                __check("%0%1%2%3%4%5",					\
			"%%eax%%eax%%ebx%%ecx%%edx%%esi") 	     	\
			"int $0x80" __comment(name)			\
		      : "=a"(__sc_a0)					\
		      : "0"(__sc_a0),					\
			"r" (__sc_a1), "r"(__sc_a2), "r" (__sc_a3),	\
			"r" (__sc_a4)					\
		      : "memory"					\
		);							\
		__sc_res = __sc_a0;					\
	}								\
	__syscall_return(type, __sc_res);				\
} 

#define _syscall5(type, name, type1, arg1, type2, arg2, type3, arg3,	\
			      type4, arg4, type5, arg5)			\
type name (type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5)	\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(a0, "eax", __NR_##name);		\
		__syscall_regval(a1, "ebx", arg1);			\
		__syscall_regval(a2, "ecx", arg2);			\
		__syscall_regval(a3, "edx", arg3);			\
		__syscall_regval(a4, "esi", arg4);			\
		__syscall_regval(a5, "edi", arg5);			\
									\
		__asm__ volatile (					\
                __check("%0%1%2%3%4%5%6",				\
			"%%eax%%eax%%ebx%%ecx%%edx%%esi%%edi") 	     	\
			"int $0x80" __comment(name)			\
		      : "=a"(__sc_a0)					\
		      : "0"(__sc_a0),					\
			"r" (__sc_a1), "r"(__sc_a2), "r" (__sc_a3),	\
			"r" (__sc_a4), "r"(__sc_a5)			\
		      : "memory"					\
		);							\
		__sc_res = __sc_a0;					\
	}								\
	__syscall_return(type, __sc_res);				\
}

#define _syscall6(type, name, type1, arg1, type2, arg2, type3, arg3,	\
			      type4, arg4, type5, arg5, type6, arg6)	\
type name (type1 arg1, type2 arg2, type3 arg3,				\
	   type4 arg4, type5 arg5, type6 arg6)				\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(a0, "eax", __NR_##name);		\
		__syscall_regval(a1, "ebx", arg1);			\
		__syscall_regval(a2, "ecx", arg2);			\
		__syscall_regval(a3, "edx", arg3);			\
		__syscall_regval(a4, "esi", arg4);			\
		__syscall_regval(a5, "edi", arg5);			\
		__syscall_regval(a6, "ebp", arg6);			\
									\
		__asm__ volatile (					\
                __check("%0%1%2%3%4%5%6%7",				\
			"%%eax%%eax%%ebx%%ecx%%edx%%esi%%edi%%ebp")	\
			"int $0x80" __comment(name)			\
		      : "=a"(__sc_a0)					\
		      : "0"(__sc_a0),					\
			"r" (__sc_a1), "r"(__sc_a2), "r" (__sc_a3),	\
			"r" (__sc_a4), "r"(__sc_a5), "r" (__sc_a6)	\
		      : "memory"					\
		);							\
		__sc_res = __sc_a0;					\
	}								\
	__syscall_return(type, __sc_res);				\
}


/*	*****************************************
 	IA64	IA64	IA64	IA64		*
	ia64 kernel interface 			*/

#elif	defined(__ia64__)

#warning syscall arch ia64 not implemented yet


/*	*****************************************
 	M32R	IM32R	M32R	M32R		*
	m32r kernel interface 			*/

#elif	defined(__m32r__)

#warning syscall arch m32r not implemented yet


/*	*****************************************
 	M68K	M68K	M68K	M68K		*
	m68k kernel interface 			*/

#elif	defined(__m68000__)

#warning syscall arch m68k not implemented yet


/*	*****************************************
 	MIPS	MIPS	MIPS	MIPS		*
	mips kernel interface 			*/

#elif defined(__mips__)

#warning syscall arch mips not implemented yet


/*	*****************************************
 	HPPA	HPPA	HPPA	HPPA		*
	hppa kernel interface 			*/

#elif defined(__hppa__)

/*  	The hppa calling convention uses r26-r23 for the first 4
    	arguments, the rest is spilled onto the stack. However the
    	Linux kernel passes the first six arguments in the registers
	r26-r21. 
	
	The system call number MUST ALWAYS be loaded in the delay 
	slot of the ble instruction, or restarting system calls 
	WILL NOT WORK.
	
	scnr:	r20
	args:	r26, r25, r24, r23, r22, r21 
	sret:	r28
	serr:	(err= sret > (unsigned)-EMAXERRNO)
	clob:	r1, r2, r4, r20, r29, r31, memory
*/

#ifndef EMAXERRNO
#define EMAXERRNO   4095
#endif

#define __syscall_errcon(res)						\
	((unsigned long)(res) >= (unsigned long)(-EMAXERRNO))

#define __syscall_return(type, res) do {				\
	__syscall_retval(res);						\
	if (__syscall_errcon(res)) {					\
		int __err = -(res);					\
		__syscall_error(__err);					\
		res = -1;						\
	}								\
	return (type) res;						\
} while (0)				

#define __syscall_clobbers						\
	"%r1", "%r2", "%r4", "%r20", "%r29", "%r31", "memory" 

#define __syscall_regdef(name, reg)					\
	register unsigned long __sc_##name __asm__ (reg)

#define __syscall_regval(name, reg, val)				\
	register unsigned long __sc_##name __asm__ (reg) =  	    	\
	(unsigned long)(val)


#define _syscall0(type, name)						\
type name(void)								\
{									\
	long __sc_res;							\
	{								\
		__syscall_regdef(ret, "r28");				\
									\
 	 	__asm__ __volatile__ (  	      			\
		__check("%0", "%%r28")					\
 	 		"ble  0x100(%%sr2, %%r0)" 			\
				__comment(name) "\n\t"			\
			"ldi %1, %%r20"					\
 	 	      : "=r"(__sc_ret)					\
 	 	      : "i"(__NR_##name)				\
 	 	      : "%r21", "%r22", "%r23", 			\
		      	"%r24", "%r25", "%r26", 			\
		      	__syscall_clobbers				\
	    	);  				      			\
 	 	__sc_res = __sc_ret;		      			\
    	}								\
	__syscall_return(type, __sc_res);				\
}

#define _syscall1(type, name, type1, arg1)				\
type name(type1 arg1)							\
{									\
	long __sc_res;							\
	{								\
		__syscall_regdef(ret, "r28");				\
		__syscall_regval(a1, "r26", arg1);			\
									\
 	 	__asm__ __volatile__ (  	      			\
		__check("%0%2", "%%r28%%r26")				\
 	 		"ble  0x100(%%sr2, %%r0)" 			\
				__comment(name) "\n\t"			\
			"ldi %1, %%r20"					\
 	 	      : "=r"(__sc_ret)					\
 	 	      : "i"(__NR_##name), 				\
		      	"r"(__sc_a1)					\
 	 	      : "%r21", "%r22", "%r23", "%r24", "%r25",		\
		      	__syscall_clobbers				\
	    	);  				      			\
 	 	__sc_res = __sc_ret;		      			\
    	}								\
	__syscall_return(type, __sc_res);				\
}

#define _syscall2(type, name, type1, arg1, type2, arg2)			\
type name(type1 arg1, type2 arg2)					\
{									\
	long __sc_res;							\
	{								\
		__syscall_regdef(ret, "r28");				\
		__syscall_regval(a1, "r26", arg1);			\
		__syscall_regval(a2, "r25", arg2);			\
									\
 	 	__asm__ __volatile__ (  	      			\
		__check("%0%2%3", "%%r28%%r26%%r25")			\
 	 		"ble  0x100(%%sr2, %%r0)" 			\
				__comment(name) "\n\t"			\
			"ldi %1, %%r20"					\
 	 	      : "=r"(__sc_ret)					\
 	 	      : "i"(__NR_##name),				\
		      	"r"(__sc_a1), "r"(__sc_a2)			\
 	 	      : "%r21", "%r22", "%r23", "%r24",			\
		      	__syscall_clobbers				\
	    	);  				      			\
 	 	__sc_res = __sc_ret;		      			\
    	}								\
	__syscall_return(type, __sc_res);				\
}

#define _syscall3(type, name, type1, arg1, type2, arg2, type3, arg3)	\
type name(type1 arg1, type2 arg2, type3 arg3)				\
{									\
	long __sc_res;							\
	{								\
		__syscall_regdef(ret, "r28");				\
		__syscall_regval(a1, "r26", arg1);			\
		__syscall_regval(a2, "r25", arg2);			\
		__syscall_regval(a3, "r24", arg3);			\
									\
 	 	__asm__ __volatile__ (  	      			\
		__check("%0%2%3%4", "%%r28%%r26%%r25%%r24")		\
 	 		"ble  0x100(%%sr2, %%r0)" 			\
				__comment(name) "\n\t"			\
			"ldi %1, %%r20"					\
 	 	      : "=r"(__sc_ret)					\
 	 	      : "i"(__NR_##name),				\
		      	"r"(__sc_a1), "r"(__sc_a2), "r"(__sc_a3)	\
 	 	      : "%r21", "%r22", "%r23",				\
		      	__syscall_clobbers				\
	    	);  				      			\
 	 	__sc_res = __sc_ret;		      			\
    	}								\
	__syscall_return(type, __sc_res);				\
} 

#define _syscall4(type, name, type1, arg1, type2, arg2, type3, arg3,	\
			      type4, arg4)				\
type name (type1 arg1, type2 arg2, type3 arg3, type4 arg4)		\
{									\
	long __sc_res;							\
	{								\
		__syscall_regdef(ret, "r28");				\
		__syscall_regval(a1, "r26", arg1);			\
		__syscall_regval(a2, "r25", arg2);			\
		__syscall_regval(a3, "r24", arg3);			\
		__syscall_regval(a4, "r23", arg4);			\
									\
 	 	__asm__ __volatile__ (  	      			\
		__check("%0%2%3%4%5", "%%r28%%r26%%r25%%r24%%r23")	\
 	 		"ble  0x100(%%sr2, %%r0)" 			\
				__comment(name) "\n\t"			\
			"ldi %1, %%r20"					\
 	 	      : "=r"(__sc_ret)					\
 	 	      : "i"(__NR_##name),				\
		      	"r"(__sc_a1), "r"(__sc_a2), "r"(__sc_a3),	\
		      	"r"(__sc_a4)					\
 	 	      : "%r21", "%r22",					\
		      	__syscall_clobbers				\
	    	);  				      			\
 	 	__sc_res = __sc_ret;		      			\
    	}								\
	__syscall_return(type, __sc_res);				\
} 

#define _syscall5(type, name, type1, arg1, type2, arg2, type3, arg3,	\
			      type4, arg4, type5, arg5)			\
type name (type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5)	\
{									\
	long __sc_res;							\
	{								\
		__syscall_regdef(ret, "r28");				\
		__syscall_regval(a1, "r26", arg1);			\
		__syscall_regval(a2, "r25", arg2);			\
		__syscall_regval(a3, "r24", arg3);			\
		__syscall_regval(a4, "r23", arg4);			\
		__syscall_regval(a5, "r22", arg5);			\
									\
 	 	__asm__ __volatile__ (  	      			\
		__check("%0%2%3%4%5%6", 				\
			"%%r28%%r26%%r25%%r24%%r23%%r22")		\
 	 		"ble  0x100(%%sr2, %%r0)" 			\
				__comment(name) "\n\t"			\
			"ldi %1, %%r20"					\
 	 	      : "=r"(__sc_ret)					\
 	 	      : "i"(__NR_##name),				\
		      	"r"(__sc_a1), "r"(__sc_a2), "r"(__sc_a3),	\
		      	"r"(__sc_a4), "r"(__sc_a5)			\
 	 	      : "%r21",						\
		      	__syscall_clobbers				\
	    	);  				      			\
 	 	__sc_res = __sc_ret;		      			\
    	}								\
	__syscall_return(type, __sc_res);				\
}

#define _syscall6(type, name, type1, arg1, type2, arg2, type3, arg3,	\
			type4, arg4, type5, arg5, type6, arg6)		\
type name (type1 arg1, type2 arg2, type3 arg3,				\
	   type4 arg4, type5 arg5, type6 arg6)				\
{									\
	long __sc_res;							\
	{								\
		__syscall_regdef(ret, "r28");				\
		__syscall_regval(a1, "r26", arg1);			\
		__syscall_regval(a2, "r25", arg2);			\
		__syscall_regval(a3, "r24", arg3);			\
		__syscall_regval(a4, "r23", arg4);			\
		__syscall_regval(a5, "r22", arg5);			\
		__syscall_regval(a6, "r21", arg6);			\
									\
 	 	__asm__ __volatile__ (  	      			\
		__check("%0%2%3%4%5%6%7", 				\
			"%%r28%%r26%%r25%%r24%%r23%%r22%%r21")		\
 	 		"ble  0x100(%%sr2, %%r0)" 			\
				__comment(name) "\n\t"			\
			"ldi %1, %%r20"					\
 	 	      : "=r"(__sc_ret)					\
 	 	      : "i"(__NR_##name),				\
		      	"r"(__sc_a1), "r"(__sc_a2), "r"(__sc_a3),	\
		      	"r"(__sc_a4), "r"(__sc_a5), "r"(__sc_a6)	\
 	 	      : __syscall_clobbers				\
	    	);  				      			\
 	 	__sc_res = __sc_ret;		      			\
    	}								\
	__syscall_return(type, __sc_res);				\
}



/*	*****************************************
 	PPC64	PPC64	PPC64	PPC64		*
	ppc64 kernel interface 			*/

#elif defined(__powerpc64__)

#warning syscall arch ppc64 not implemented yet


/*	*****************************************
 	PPC	PPC	PPC	PPC		*
	ppc kernel interface 			*/

#elif defined(__powerpc__)

/*      The powerpc calling convention uses r3-r10 to pass the first
    	eight arguments, the remainder is spilled onto the stack.
        
        scnr:   r0
        args:   a1(r3), a2(r4), a3(r5), a4(r6), a5(r7), a6(r8)
        sret:   r3
        serr:   (carry)
    	call:	sc
	clob:	cr0, ctr
*/

#define __syscall_errcon(err)	(err & 0x10000000)

#define __syscall_return(type, ret, err) do {				\
	__syscall_retval(ret);						\
	if (__syscall_errcon(err)) {					\
		int __err = (ret);					\
		__syscall_error(__err);					\
		ret = -1;						\
	}								\
	return (type) ret;						\
} while (0)				

#define __syscall_regdef(name, reg)					\
	register long __sc_##name __asm__ (reg)

#define __syscall_regval(name, reg, val)				\
	register long __sc_##name __asm__ (reg) = (long)(val)

#define __syscall_clobbers						\
    	"r9", "r10", "r11", "r12",					\
	"cr0", "ctr", "memory"


#define _syscall0(type, name)						\
type name(void)								\
{									\
	long __sc_ret, __sc_err;				\
	{								\
		__syscall_regval(r0, "r0", __NR_##name);		\
		__syscall_regdef(a1, "r3");				\
									\
		__asm__ __volatile__ (					\
			"sc" __comment(name) "\n\t"			\
			"mfcr %0"   					\
		      : "=r"(__sc_r0), "=r"(__sc_a1)			\
		      : "0"(__sc_r0)					\
		      : "r4", "r5", "r6", "r7", "r8",			\
			__syscall_clobbers				\
		);							\
		__sc_ret = __sc_a1;					\
		__sc_err = __sc_r0;					\
	}								\
	__syscall_return(type, __sc_ret, __sc_err);			\
}

#define _syscall1(type, name, type1, arg1)				\
type name(type1 arg1)							\
{									\
	unsigned long __sc_ret, __sc_err;				\
	{								\
		__syscall_regval(r0, "r0", __NR_##name);		\
		__syscall_regval(a1, "r3", arg1);			\
									\
		__asm__ __volatile__ (					\
			"sc" __comment(name) "\n\t"			\
			"mfcr %0"   					\
		      : "=r"(__sc_r0), "=r"(__sc_a1)			\
		      : "0"(__sc_r0),  					\
		      	"1"(__sc_a1)					\
		      : "r4", "r5", "r6", "r7", "r8",			\
			__syscall_clobbers				\
		);							\
		__sc_ret = __sc_a1;					\
		__sc_err = __sc_r0;					\
	}								\
	__syscall_return(type, __sc_ret, __sc_err);			\
}

#define _syscall2(type, name, type1, arg1, type2, arg2)			\
type name(type1 arg1, type2 arg2)					\
{									\
	unsigned long __sc_ret, __sc_err;				\
	{								\
		__syscall_regval(r0, "r0", __NR_##name);		\
		__syscall_regval(a1, "r3", arg1);			\
		__syscall_regval(a2, "r4", arg2);			\
									\
		__asm__ __volatile__ (					\
			"sc" __comment(name) "\n\t"			\
			"mfcr %0"   					\
		      : "=r"(__sc_r0), "=r"(__sc_a1)			\
		      : "0"(__sc_r0),  					\
		      	"1"(__sc_a1), "r"(__sc_a2)			\
		      : "r5", "r6", "r7", "r8",				\
			__syscall_clobbers				\
		);							\
		__sc_ret = __sc_a1;					\
		__sc_err = __sc_r0;					\
	}								\
	__syscall_return(type, __sc_ret, __sc_err);			\
}

#define _syscall3(type, name, type1, arg1, type2, arg2, type3, arg3)	\
type name(type1 arg1, type2 arg2, type3 arg3)				\
{									\
	unsigned long __sc_ret, __sc_err;				\
	{								\
		__syscall_regval(r0, "r0", __NR_##name);		\
		__syscall_regval(a1, "r3", arg1);			\
		__syscall_regval(a2, "r4", arg2);			\
		__syscall_regval(a3, "r5", arg3);			\
									\
		__asm__ __volatile__ (					\
			"sc" __comment(name) "\n\t"			\
			"mfcr %0"   					\
		      : "=r"(__sc_r0), "=r"(__sc_a1)			\
		      : "0"(__sc_r0),  					\
		      	"1"(__sc_a1), "r"(__sc_a2), "r"(__sc_a3)	\
		      : "r6", "r7", "r8",				\
			__syscall_clobbers				\
		);							\
		__sc_ret = __sc_a1;					\
		__sc_err = __sc_r0;					\
	}								\
	__syscall_return(type, __sc_ret, __sc_err);			\
} 


#define _syscall4(type, name, type1, arg1, type2, arg2, type3, arg3,	\
			      type4, arg4) 				\
type name (type1 arg1, type2 arg2, type3 arg3, type4 arg4)		\
{									\
	unsigned long __sc_ret, __sc_err;				\
	{								\
		__syscall_regval(r0, "r0", __NR_##name);		\
		__syscall_regval(a1, "r3", arg1);			\
		__syscall_regval(a2, "r4", arg2);			\
		__syscall_regval(a3, "r5", arg3);			\
		__syscall_regval(a4, "r6", arg4);			\
									\
		__asm__ __volatile__ (					\
			"sc" __comment(name) "\n\t"			\
			"mfcr %0"   					\
		      : "=r"(__sc_r0), "=r"(__sc_a1)			\
		      : "0"(__sc_r0),  					\
		      	"1"(__sc_a1), "r"(__sc_a2), "r"(__sc_a3),	\
		      	"r"(__sc_a4)					\
		      : "r7", "r8",					\
			__syscall_clobbers				\
		);							\
		__sc_ret = __sc_a1;					\
		__sc_err = __sc_r0;					\
	}								\
	__syscall_return(type, __sc_ret, __sc_err);			\
} 

#define _syscall5(type, name, type1, arg1, type2, arg2, type3, arg3,	\
    			      type4, arg4, type5, arg5)			\
type name (type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5)	\
{									\
	unsigned long __sc_ret, __sc_err;				\
	{								\
		__syscall_regval(r0, "r0", __NR_##name);		\
		__syscall_regval(a1, "r3", arg1);			\
		__syscall_regval(a2, "r4", arg2);			\
		__syscall_regval(a3, "r5", arg3);			\
		__syscall_regval(a4, "r6", arg4);			\
		__syscall_regval(a5, "r7", arg5);			\
									\
		__asm__ __volatile__ (					\
			"sc" __comment(name) "\n\t"			\
			"mfcr %0"   					\
		      : "=r"(__sc_r0), "=r"(__sc_a1)			\
		      : "0"(__sc_r0),  					\
		      	"1"(__sc_a1), "r"(__sc_a2), "r"(__sc_a3),	\
		      	"r"(__sc_a4), "r"(__sc_a5)			\
		      : "r8",						\
			__syscall_clobbers				\
		);							\
		__sc_ret = __sc_a1;					\
		__sc_err = __sc_r0;					\
	}								\
	__syscall_return(type, __sc_ret, __sc_err);			\
}

#define _syscall6(type, name, type1, arg1, type2, arg2, type3, arg3,	\
			      type4, arg4, type5, arg5, type6, arg6)	\
type name (type1 arg1, type2 arg2, type3 arg3,				\
	   type4 arg4, type5 arg5, type6 arg6)				\
{									\
	unsigned long __sc_ret, __sc_err;				\
	{								\
		__syscall_regval(r0, "r0", __NR_##name);		\
		__syscall_regval(a1, "r3", arg1);			\
		__syscall_regval(a2, "r4", arg2);			\
		__syscall_regval(a3, "r5", arg3);			\
		__syscall_regval(a4, "r6", arg4);			\
		__syscall_regval(a5, "r7", arg5);			\
		__syscall_regval(a6, "r8", arg6);			\
									\
		__asm__ __volatile__ (					\
			"sc" __comment(name) "\n\t"			\
			"mfcr %0"   					\
		      : "=r"(__sc_r0), "=r"(__sc_a1)			\
		      : "0"(__sc_r0),  					\
		      	"1"(__sc_a1), "r"(__sc_a2), "r"(__sc_a3),	\
		      	"r"(__sc_a4), "r"(__sc_a5), "r"(__sc_a6)	\
		      : __syscall_clobbers				\
		);							\
		__sc_ret = __sc_a1;					\
		__sc_err = __sc_r0;					\
	}								\
	__syscall_return(type, __sc_ret, __sc_err);			\
}


/*	*****************************************
 	S390X	S390X	S390X	S390X		*
	s390x kernel interface 			*/

#elif defined(__s390x__)

/*      The s390x calling convention passes the first five arguments
    	in r2-r6, the remainder is spilled onto the stack. However 
	the Linux kernel passes the first six arguments in r2-r7.
	
        scnr:   imm, r1 
        args:   a1(r2), a2(r3), a3(r4), a4(r5), a5(r6), a6(r7)
        sret:   r2
        serr:   (err= sret > (unsigned)-EMAXERRNO)
    	call:	svc
	clob:	memory
*/

#ifndef EMAXERRNO
#define EMAXERRNO   4095
#endif

#define __syscall_errcon(res)                                           \
        ((unsigned long)(res) >= (unsigned long)(-EMAXERRNO))

#define __syscall_return(type, res) do {                                \
        __syscall_retval(res);                                          \
        if (__syscall_errcon(res)) {                                    \
                int __err = -(res);                                     \
                __syscall_error(__err);                                 \
                res = -1;                                               \
        }                                                               \
        return (type) res;                                              \
} while (0)                             

#define __syscall_regdef(name, reg)					\
	register unsigned long __sc_##name __asm__ (reg)

#define __syscall_regval(name, reg, val)				\
	register unsigned long __sc_##name __asm__ (reg) = 		\
		(unsigned long)(val)

#define __syscall_clobbers	"memory"


#define _syscall0(type, name)						\
type name(void)								\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(nr, "r1", __NR_##name);		\
		__syscall_regdef(a1, "r2");				\
									\
		__asm__ volatile (					\
 		__check("%0%1", "%%r2%%r1")				\
			"svc  0" __comment(name)			\
		      : "=r"(__sc_a1)					\
		      : "r"(__sc_nr)					\
		      : __syscall_clobbers				\
		);							\
		__sc_res = __sc_a1;					\
	}								\
	__syscall_return(type, __sc_res);				\
}

#define _syscall1(type, name, type1, arg1)				\
type name(type1 arg1)							\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(nr, "r1", __NR_##name);		\
		__syscall_regval(a1, "r2", arg1);			\
									\
		__asm__ volatile (					\
 		__check("%0%1%2", "%%r2%%r1%%r2")			\
			"svc  0" __comment(name)			\
		      : "=r"(__sc_a1)					\
		      : "r"(__sc_nr), 					\
			"0"(__sc_a1)					\
		      : __syscall_clobbers				\
		);							\
		__sc_res = __sc_a1;					\
	}								\
	__syscall_return(type, __sc_res);				\
}

#define _syscall2(type, name, type1, arg1, type2, arg2)			\
type name(type1 arg1, type2 arg2)					\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(nr, "r1", __NR_##name);		\
		__syscall_regval(a1, "r2", arg1);			\
		__syscall_regval(a2, "r3", arg2);			\
									\
		__asm__ volatile (					\
 		__check("%0%1%2%3", "%%r2%%r1%%r2%%r3")			\
			"svc  0" __comment(name)			\
		      : "=r"(__sc_a1)					\
		      : "r"(__sc_nr), 					\
			"0"(__sc_a1), "r"(__sc_a2)			\
		      : __syscall_clobbers				\
		);							\
		__sc_res = __sc_a1;					\
	}								\
	__syscall_return(type, __sc_res);				\
}

#define _syscall3(type, name, type1, arg1, type2, arg2, type3, arg3)	\
type name(type1 arg1, type2 arg2, type3 arg3)				\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(nr, "r1", __NR_##name);		\
		__syscall_regval(a1, "r2", arg1);			\
		__syscall_regval(a2, "r3", arg2);			\
		__syscall_regval(a3, "r4", arg3);			\
									\
		__asm__ volatile (					\
 		__check("%0%1%2%3%4", "%%r2%%r1%%r2%%r3%%r4")		\
			"svc  0" __comment(name)			\
		      : "=r"(__sc_a1)					\
		      : "r"(__sc_nr), 					\
			"0"(__sc_a1), "r"(__sc_a2), "r"(__sc_a3)	\
		      : __syscall_clobbers				\
		);							\
		__sc_res = __sc_a1;					\
	}								\
	__syscall_return(type, __sc_res);				\
} 

#define _syscall4(type, name, type1, arg1, type2, arg2, type3, arg3,	\
			      type4, arg4)				\
type name (type1 arg1, type2 arg2, type3 arg3, type4 arg4)		\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(nr, "r1", __NR_##name);		\
		__syscall_regval(a1, "r2", arg1);			\
		__syscall_regval(a2, "r3", arg2);			\
		__syscall_regval(a3, "r4", arg3);			\
		__syscall_regval(a4, "r5", arg4);			\
									\
		__asm__ volatile (					\
 		__check("%0%1%2%3%4%5", "%%r2%%r1%%r2%%r3%%r4%%r5")	\
			"svc  0" __comment(name)			\
		      : "=r"(__sc_a1)					\
		      : "r"(__sc_nr), 					\
			"0"(__sc_a1), "r"(__sc_a2), "r"(__sc_a3),	\
			"r"(__sc_a4)					\
		      : __syscall_clobbers				\
		);							\
		__sc_res = __sc_a1;					\
	}								\
	__syscall_return(type, __sc_res);				\
} 

#define _syscall5(type, name, type1, arg1, type2, arg2, type3, arg3,	\
			      type4, arg4, type5, arg5)			\
type name (type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5)	\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(nr, "r1", __NR_##name);		\
		__syscall_regval(a1, "r2", arg1);			\
		__syscall_regval(a2, "r3", arg2);			\
		__syscall_regval(a3, "r4", arg3);			\
		__syscall_regval(a4, "r5", arg4);			\
		__syscall_regval(a5, "r6", arg5);			\
									\
		__asm__ volatile (					\
 		__check("%0%1%2%3%4%5%6",				\
			"%%r2%%r1%%r2%%r3%%r4%%r5%%r6")			\
			"svc  0" __comment(name)			\
		      : "=r"(__sc_a1)					\
		      : "r"(__sc_nr), 					\
			"0"(__sc_a1), "r"(__sc_a2), "r"(__sc_a3),	\
			"r"(__sc_a4), "r"(__sc_a5)			\
		      : __syscall_clobbers				\
		);							\
		__sc_res = __sc_a1;					\
	}								\
	__syscall_return(type, __sc_res);				\
}

#define _syscall6(type, name, type1, arg1, type2, arg2, type3, arg3,	\
			type4, arg4, type5, arg5, type6, arg6)		\
type name (type1 arg1, type2 arg2, type3 arg3,				\
	   type4 arg4, type5 arg5, type6 arg6)				\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(nr, "r1", __NR_##name);		\
		__syscall_regval(a1, "r2", arg1);			\
		__syscall_regval(a2, "r3", arg2);			\
		__syscall_regval(a3, "r4", arg3);			\
		__syscall_regval(a4, "r5", arg4);			\
		__syscall_regval(a5, "r6", arg5);			\
		__syscall_regval(a6, "r7", arg6);			\
									\
		__asm__ volatile (					\
 		__check("%0%1%2%3%4%5%6%7",				\
			"%%r2%%r1%%r2%%r3%%r4%%r5%%r6%%r7")		\
			"svc  0" __comment(name)			\
		      : "=r"(__sc_a1)					\
		      : "r"(__sc_nr), 					\
			"0"(__sc_a1), "r"(__sc_a2), "r"(__sc_a3),	\
			"r"(__sc_a4), "r"(__sc_a5), "r"(__sc_a6)	\
		      : __syscall_clobbers				\
		);							\
		__sc_res = __sc_a1;					\
	}								\
	__syscall_return(type, __sc_res);				\
}



/*	*****************************************
 	S390	S390	S390	S390		*
	s390 kernel interface 			*/

#elif defined(__s390__)

#warning syscall arch s390 not implemented yet


/*	*****************************************
 	SH	SH	SH	SH		*
	sh kernel interface 			*/

#elif defined(__sh__) && !defined(__SH5__)

#warning syscall arch sh not implemented yet


/*	*****************************************
 	SH64	SH64	SH64	SH64		*
	sh64 kernel interface 			*/

#elif defined(__sh__) && defined(__SH5__)

#warning syscall arch sh64 not implemented yet


/*	*****************************************
 	SPARC64	SPARC64	SPARC64	SPARC64		*
	sparc64 kernel interface 		*/

#elif defined(__sparc__) && defined(__arch64__)

/*      The sparc64 calling convention uses o0-o5 to pass the first six
    	arguments (mapped via register windows).
        
        scnr:   g1
        args:   o0, o1, o2, o3, o4, o5 
        sret:   o0
        serr:   (carry)
    	call:	t 0x10
	clob:	g1-g6, g7?, o7?, f0-f31, cc
*/

#ifndef EMAXERRNO
#define EMAXERRNO   515
#endif

#define __syscall_errcon(res)                                           \
        ((unsigned long)(res) >= (unsigned long)(-EMAXERRNO))

#define __syscall_return(type, res) do {                                \
        __syscall_retval(res);                                          \
        if (__syscall_errcon(res)) {                                    \
                int __err = -(res);                                     \
                __syscall_error(__err);                                 \
                res = -1;                                               \
        }                                                               \
        return (type) res;                                              \
} while (0)                             

#define __syscall_clobbers						\
	"g2", "g3", "g4", "g5", "g6", 					\
	"f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", 		\
	"f9", "f10", "f11", "f12", "f13", "f14", "f15", "f16", 		\
	"f17", "f18", "f19", "f20", "f21", "f22", "f23", "f24", 	\
	"f25", "f26", "f27", "f28", "f29", "f30", "f31", "f32",		\
	"f34", "f36", "f38", "f40", "f42", "f44", "f46", "f48",		\
	"f50", "f52", "f54", "f56", "f58", "f60", "f62",    	    	\
	"cc", "memory" 

#define __syscall_regdef(name, reg)					\
	register long __sc_##name __asm__ (reg)

#define __syscall_regval(name, reg, val)				\
	register long __sc_##name __asm__ (reg) = (long)(val)

#define _syscall0(type, name)						\
type name(void)								\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(g1, "g1", __NR_##name);		\
		__syscall_regdef(o0, "o0");				\
									\
		__asm__ volatile (					\
			"ta 0x6d" __comment(name) "\n\t"		\
			"bcs,a,pt %%xcc,1f\n\t"   			\
			"sub %%g0,%%o0,%%o0\n"   			\
			"1:" 	  					\
		      : "=r"(__sc_o0)					\
		      : "r"(__sc_g1) 					\
		      : __syscall_clobbers				\
		);							\
		__sc_res = __sc_o0;					\
	}								\
	__syscall_return(type, __sc_res);				\
}

#define _syscall1(type, name, type1, arg1)				\
type name(type1 arg1)							\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(g1, "g1", __NR_##name);		\
		__syscall_regval(o0, "o0", arg1);			\
									\
		__asm__ volatile (					\
			"ta 0x6d" __comment(name) "\n\t"		\
			"bcs,a,pt %%xcc,1f\n\t"   			\
			"sub %%g0,%%o0,%%o0\n"   			\
			"1:" 	  					\
		      : "=r"(__sc_o0)					\
		      : "r"(__sc_g1), 					\
			"0"(__sc_o0)					\
		      : __syscall_clobbers				\
		);							\
		__sc_res = __sc_o0;					\
	}								\
	__syscall_return(type, __sc_res);				\
}

#define _syscall2(type, name, type1, arg1, type2, arg2)			\
type name(type1 arg1, type2 arg2)					\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(g1, "g1", __NR_##name);		\
		__syscall_regval(o0, "o0", arg1);			\
		__syscall_regval(o1, "o1", arg2);			\
									\
		__asm__ volatile (					\
			"ta 0x6d" __comment(name) "\n\t"		\
			"bcs,a,pt %%xcc,1f\n\t"   			\
			"sub %%g0,%%o0,%%o0\n"   			\
			"1:" 	  					\
		      : "=r"(__sc_o0)					\
		      : "r"(__sc_g1), 					\
			"0"(__sc_o0), "r"(__sc_o1)			\
		      : __syscall_clobbers				\
		);							\
		__sc_res = __sc_o0;					\
	}								\
	__syscall_return(type, __sc_res);				\
}

#define _syscall3(type, name, type1, arg1, type2, arg2, type3, arg3)	\
type name(type1 arg1, type2 arg2, type3 arg3)				\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(g1, "g1", __NR_##name);		\
		__syscall_regval(o0, "o0", arg1);			\
		__syscall_regval(o1, "o1", arg2);			\
		__syscall_regval(o2, "o2", arg3);			\
									\
		__asm__ volatile (					\
			"ta 0x6d" __comment(name) "\n\t"		\
			"bcs,a,pt %%xcc,1f\n\t"   			\
			"sub %%g0,%%o0,%%o0\n"   			\
			"1:" 	  					\
		      : "=r"(__sc_o0)					\
		      : "r"(__sc_g1), 					\
			"0"(__sc_o0), "r"(__sc_o1), "r"(__sc_o2)	\
		      : __syscall_clobbers				\
		);							\
		__sc_res = __sc_o0;					\
	}								\
	__syscall_return(type, __sc_res);				\
} 

#define _syscall4(type, name, type1, arg1, type2, arg2, type3, arg3,	\
			      type4, arg4)				\
type name (type1 arg1, type2 arg2, type3 arg3, type4 arg4)		\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(g1, "g1", __NR_##name);		\
		__syscall_regval(o0, "o0", arg1);			\
		__syscall_regval(o1, "o1", arg2);			\
		__syscall_regval(o2, "o2", arg3);			\
		__syscall_regval(o3, "o3", arg4);			\
									\
		__asm__ volatile (					\
			"ta 0x6d" __comment(name) "\n\t"		\
			"bcs,a,pt %%xcc,1f\n\t"   			\
			"sub %%g0,%%o0,%%o0\n"   			\
			"1:" 	  					\
		      : "=r"(__sc_o0)					\
		      : "r"(__sc_g1), 					\
			"0"(__sc_o0), "r"(__sc_o1), "r"(__sc_o2),	\
			"r"(__sc_o3)					\
		      : __syscall_clobbers				\
		);							\
		__sc_res = __sc_o0;					\
	}								\
	__syscall_return(type, __sc_res);				\
} 

#define _syscall5(type, name, type1, arg1, type2, arg2, type3, arg3,	\
			      type4, arg4, type5, arg5)			\
type name (type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5)	\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(g1, "g1", __NR_##name);		\
		__syscall_regval(o0, "o0", arg1);			\
		__syscall_regval(o1, "o1", arg2);			\
		__syscall_regval(o2, "o2", arg3);			\
		__syscall_regval(o3, "o3", arg4);			\
		__syscall_regval(o4, "o4", arg5);			\
									\
		__asm__ volatile (					\
			"ta 0x6d" __comment(name) "\n\t"		\
			"bcs,a,pt %%xcc,1f\n\t"   			\
			"sub %%g0,%%o0,%%o0\n"   			\
			"1:" 	  					\
		      : "=r"(__sc_o0)					\
		      : "r"(__sc_g1), 					\
			"0"(__sc_o0), "r"(__sc_o1), "r"(__sc_o2),	\
			"r"(__sc_o3), "r"(__sc_o4)			\
		      : __syscall_clobbers				\
		);							\
		__sc_res = __sc_o0;					\
	}								\
	__syscall_return(type, __sc_res);				\
}

#define _syscall6(type, name, type1, arg1, type2, arg2, type3, arg3,	\
			type4, arg4, type5, arg5, type6, arg6)		\
type name (type1 arg1, type2 arg2, type3 arg3,				\
	   type4 arg4, type5 arg5, type6 arg6)				\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(g1, "g1", __NR_##name);		\
		__syscall_regval(o0, "o0", arg1);			\
		__syscall_regval(o1, "o1", arg2);			\
		__syscall_regval(o2, "o2", arg3);			\
		__syscall_regval(o3, "o3", arg4);			\
		__syscall_regval(o4, "o4", arg5);			\
		__syscall_regval(o5, "o5", arg6);			\
									\
		__asm__ volatile (					\
			"ta 0x6d" __comment(name) "\n\t"		\
			"bcs,a,pt %%xcc,1f\n\t"   			\
			"sub %%g0,%%o0,%%o0\n"   			\
			"1:" 	  					\
		      : "=r"(__sc_o0)					\
		      : "r"(__sc_g1), 					\
			"0"(__sc_o0), "r"(__sc_o1), "r"(__sc_o2),	\
			"r"(__sc_o3), "r"(__sc_o4), "r"(__sc_o5)	\
		      : __syscall_clobbers				\
		);							\
		__sc_res = __sc_o0;					\
	}								\
	__syscall_return(type, __sc_res);				\
}


/*	*****************************************
 	SPARC	SPARC	SPARC	SPARC		*
	sparc kernel interface 			*/

#elif defined(__sparc__)

/*      The sparc calling convention uses o0-o5 to pass the first six
    	arguments (mapped via register windows).
        
        scnr:   g1
        args:   o0, o1, o2, o3, o4, o5 
        sret:   o0
        serr:   (carry)
    	call:	t 0x10
	clob:	g1-g6, g7?, o7?, f0-f31, cc
*/

#ifndef EMAXERRNO
#define EMAXERRNO   515
#endif

#define __syscall_errcon(res)                                           \
        ((unsigned long)(res) >= (unsigned long)(-EMAXERRNO))

#define __syscall_return(type, res) do {                                \
        __syscall_retval(res);                                          \
        if (__syscall_errcon(res)) {                                    \
                int __err = -(res);                                     \
                __syscall_error(__err);                                 \
                res = -1;                                               \
        }                                                               \
        return (type) res;                                              \
} while (0)                             

#define __syscall_clobbers						\
	"g2", "g3", "g4", "g5", "g6", 					\
	"f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", 		\
	"f9", "f10", "f11", "f12", "f13", "f14", "f15", "f16", 		\
	"f17", "f18", "f19", "f20", "f21", "f22", "f23", "f24", 	\
	"f25", "f26", "f27", "f28", "f29", "f30", "f31", 		\
	"cc", "memory" 

#define __syscall_regdef(name, reg)					\
	register long __sc_##name __asm__ (reg)

#define __syscall_regval(name, reg, val)				\
	register long __sc_##name __asm__ (reg) = (long)(val)

#define _syscall0(type, name)						\
type name(void)								\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(g1, "g1", __NR_##name);		\
		__syscall_regdef(o0, "o0");				\
									\
		__asm__ volatile (					\
			"t 0x10" __comment(name) "\n\t"			\
			"bcs,a 1f\n\t"					\
			"sub %%g0,%%o0,%%o0\n"   			\
			"1:" 	  					\
		      : "=r"(__sc_o0)					\
		      : "r"(__sc_g1) 					\
		      : __syscall_clobbers				\
		);							\
		__sc_res = __sc_o0;					\
	}								\
	__syscall_return(type, __sc_res);				\
}

#define _syscall1(type, name, type1, arg1)				\
type name(type1 arg1)							\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(g1, "g1", __NR_##name);		\
		__syscall_regval(o0, "o0", arg1);			\
									\
		__asm__ volatile (					\
			"t 0x10" __comment(name) "\n\t"			\
			"bcs,a 1f\n\t"					\
			"sub %%g0,%%o0,%%o0\n"   			\
			"1:" 	  					\
		      : "=r"(__sc_o0)					\
		      : "r"(__sc_g1), 					\
			"0"(__sc_o0)					\
		      : __syscall_clobbers				\
		);							\
		__sc_res = __sc_o0;					\
	}								\
	__syscall_return(type, __sc_res);				\
}

#define _syscall2(type, name, type1, arg1, type2, arg2)			\
type name(type1 arg1, type2 arg2)					\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(g1, "g1", __NR_##name);		\
		__syscall_regval(o0, "o0", arg1);			\
		__syscall_regval(o1, "o1", arg2);			\
									\
		__asm__ volatile (					\
			"t 0x10" __comment(name) "\n\t"			\
			"bcs,a 1f\n\t"					\
			"sub %%g0,%%o0,%%o0\n"   			\
			"1:" 	  					\
		      : "=r"(__sc_o0)					\
		      : "r"(__sc_g1), 					\
			"0"(__sc_o0), "r"(__sc_o1)			\
		      : __syscall_clobbers				\
		);							\
		__sc_res = __sc_o0;					\
	}								\
	__syscall_return(type, __sc_res);				\
}

#define _syscall3(type, name, type1, arg1, type2, arg2, type3, arg3)	\
type name(type1 arg1, type2 arg2, type3 arg3)				\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(g1, "g1", __NR_##name);		\
		__syscall_regval(o0, "o0", arg1);			\
		__syscall_regval(o1, "o1", arg2);			\
		__syscall_regval(o2, "o2", arg3);			\
									\
		__asm__ volatile (					\
			"t 0x10" __comment(name) "\n\t"			\
			"bcs,a 1f\n\t"					\
			"sub %%g0,%%o0,%%o0\n"   			\
			"1:" 	  					\
		      : "=r"(__sc_o0)					\
		      : "r"(__sc_g1), 					\
			"0"(__sc_o0), "r"(__sc_o1), "r"(__sc_o2)	\
		      : __syscall_clobbers				\
		);							\
		__sc_res = __sc_o0;					\
	}								\
	__syscall_return(type, __sc_res);				\
} 

#define _syscall4(type, name, type1, arg1, type2, arg2, type3, arg3,	\
			      type4, arg4)				\
type name (type1 arg1, type2 arg2, type3 arg3, type4 arg4)		\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(g1, "g1", __NR_##name);		\
		__syscall_regval(o0, "o0", arg1);			\
		__syscall_regval(o1, "o1", arg2);			\
		__syscall_regval(o2, "o2", arg3);			\
		__syscall_regval(o3, "o3", arg4);			\
									\
		__asm__ volatile (					\
			"t 0x10" __comment(name) "\n\t"			\
			"bcs,a 1f\n\t"					\
			"sub %%g0,%%o0,%%o0\n"   			\
			"1:" 	  					\
		      : "=r"(__sc_o0)					\
		      : "r"(__sc_g1), 					\
			"0"(__sc_o0), "r"(__sc_o1), "r"(__sc_o2),	\
			"r"(__sc_o3)					\
		      : __syscall_clobbers				\
		);							\
		__sc_res = __sc_o0;					\
	}								\
	__syscall_return(type, __sc_res);				\
} 

#define _syscall5(type, name, type1, arg1, type2, arg2, type3, arg3,	\
			      type4, arg4, type5, arg5)			\
type name (type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5)	\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(g1, "g1", __NR_##name);		\
		__syscall_regval(o0, "o0", arg1);			\
		__syscall_regval(o1, "o1", arg2);			\
		__syscall_regval(o2, "o2", arg3);			\
		__syscall_regval(o3, "o3", arg4);			\
		__syscall_regval(o4, "o4", arg5);			\
									\
		__asm__ volatile (					\
			"t 0x10" __comment(name) "\n\t"			\
			"bcs,a 1f\n\t"					\
			"sub %%g0,%%o0,%%o0\n"   			\
			"1:" 	  					\
		      : "=r"(__sc_o0)					\
		      : "r"(__sc_g1), 					\
			"0"(__sc_o0), "r"(__sc_o1), "r"(__sc_o2),	\
			"r"(__sc_o3), "r"(__sc_o4)			\
		      : __syscall_clobbers				\
		);							\
		__sc_res = __sc_o0;					\
	}								\
	__syscall_return(type, __sc_res);				\
}

#define _syscall6(type, name, type1, arg1, type2, arg2, type3, arg3,	\
			type4, arg4, type5, arg5, type6, arg6)		\
type name (type1 arg1, type2 arg2, type3 arg3,				\
	   type4 arg4, type5 arg5, type6 arg6)				\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(g1, "g1", __NR_##name);		\
		__syscall_regval(o0, "o0", arg1);			\
		__syscall_regval(o1, "o1", arg2);			\
		__syscall_regval(o2, "o2", arg3);			\
		__syscall_regval(o3, "o3", arg4);			\
		__syscall_regval(o4, "o4", arg5);			\
		__syscall_regval(o5, "o5", arg6);			\
									\
		__asm__ volatile (					\
 		__check("%0%1%2%3%4%5", "$o0$g1$o0$o1$o2$o3$o4$o5$o6")	\
			"t 0x10" __comment(name) "\n\t"			\
			"bcs,a 1f\n\t"					\
			"sub %%g0,%%o0,%%o0\n"   			\
			"1:" 	  					\
		      : "=r"(__sc_o0)					\
		      : "r"(__sc_g1), 					\
			"0"(__sc_o0), "r"(__sc_o1), "r"(__sc_o2),	\
			"r"(__sc_o3), "r"(__sc_o4), "r"(__sc_o5)	\
		      : __syscall_clobbers				\
		);							\
		__sc_res = __sc_o0;					\
	}								\
	__syscall_return(type, __sc_res);				\
}


/*	*****************************************
 	V850	V850	V850	V850		*
	v850 kernel interface 			*/

#elif defined(__v850__)

#warning syscall arch v850 not implemented yet


/*	*****************************************
 	X86_64	X86_64	X86_64	X86_64		*
	x86_64 kernel interface 		*/

#elif defined(__x86_64__)

/*      The x86_64 calling convention uses rdi, rsi, rdx, rcx, r8, r9
        but the Linux kernel interface uses rdi, rsi, rdx, r10, r8, r9.
        
        scnr:   a0(rax)
        args:   a1(rdi), a2(rsi), a3(rdx), a4(r10), a5(r8), a6(r9) 
        sret:   a0(rax)
        serr:   (err= sret > (unsigned)-EMAXERRNO)
    	call:	syscall
	clob:	rcx, r11
*/

#ifndef EMAXERRNO
#define EMAXERRNO   4095
#endif

#define __syscall_errcon(res)                                           \
        ((unsigned long)(res) >= (unsigned long)(-EMAXERRNO))

#define __syscall_return(type, res) do {                                \
        __syscall_retval(res);                                          \
        if (__syscall_errcon(res)) {                                    \
                int __err = -(res);                                     \
                __syscall_error(__err);                                 \
                res = -1;                                               \
        }                                                               \
        return (type) res;                                              \
} while (0)                             

#define __syscall_clobbers						\
    	"cc", "r11", "rcx", "memory" 

#define __syscall_regdef(name, reg)					\
	register long __sc_##name __asm__ (reg)

#define __syscall_regval(name, reg, val)				\
	register long __sc_##name __asm__ (reg) = (long)(val)


#define _syscall0(type, name)						\
type name(void)								\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(a0, "rax", __NR_##name);		\
									\
		__asm__ volatile (					\
		__check("%0%1", "%%rax%%rax")				\
			"syscall" __comment(name)			\
		      : "=a"(__sc_a0)					\
		      : "0"(__sc_a0)					\
		      : __syscall_clobbers				\
		);							\
		__sc_res = __sc_a0;					\
	}								\
	__syscall_return(type, __sc_res);				\
}

#define _syscall1(type, name, type1, arg1)				\
type name(type1 arg1)							\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(a0, "rax", __NR_##name);		\
		__syscall_regval(a1, "rdi", arg1);			\
									\
		__asm__ volatile (					\
		__check("%0%1%2", "%%rax%%rax%%rdi")			\
			"syscall" __comment(name)			\
		      : "=a"(__sc_a0)					\
		      : "0"(__sc_a0),					\
			"r" (__sc_a1)					\
		      : __syscall_clobbers				\
		);							\
		__sc_res = __sc_a0;					\
	}								\
	__syscall_return(type, __sc_res);				\
}

#define _syscall2(type, name, type1, arg1, type2, arg2)			\
type name(type1 arg1, type2 arg2)					\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(a0, "rax", __NR_##name);		\
		__syscall_regval(a1, "rdi", arg1);			\
		__syscall_regval(a2, "rsi", arg2);			\
									\
		__asm__ volatile (					\
		__check("%0%1%2%3", "%%rax%%rax%%rdi%%rsi")		\
			"syscall" __comment(name)			\
		      : "=a"(__sc_a0)					\
		      : "0"(__sc_a0),					\
			"r" (__sc_a1), "r"(__sc_a2)			\
		      : __syscall_clobbers				\
		);							\
		__sc_res = __sc_a0;					\
	}								\
	__syscall_return(type, __sc_res);				\
}

#define _syscall3(type, name, type1, arg1, type2, arg2, type3, arg3)	\
type name(type1 arg1, type2 arg2, type3 arg3)				\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(a0, "rax", __NR_##name);		\
		__syscall_regval(a1, "rdi", arg1);			\
		__syscall_regval(a2, "rsi", arg2);			\
		__syscall_regval(a3, "rdx", arg3);			\
									\
		__asm__ volatile (					\
		__check("%0%1%2%3%4", "%%rax%%rax%%rdi%%rsi%%rdx")	\
			"syscall" __comment(name)			\
		      : "=a"(__sc_a0)					\
		      : "0"(__sc_a0),					\
			"r" (__sc_a1), "r"(__sc_a2), "r" (__sc_a3)	\
		      : __syscall_clobbers				\
		);							\
		__sc_res = __sc_a0;					\
	}								\
	__syscall_return(type, __sc_res);				\
} 


#define _syscall4(type, name, type1, arg1, type2, arg2, type3, arg3,	\
			      type4, arg4) 				\
type name (type1 arg1, type2 arg2, type3 arg3, type4 arg4)		\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(a0, "rax", __NR_##name);		\
		__syscall_regval(a1, "rdi", arg1);			\
		__syscall_regval(a2, "rsi", arg2);			\
		__syscall_regval(a3, "rdx", arg3);			\
		__syscall_regval(a4, "r10", arg4);			\
									\
		__asm__ volatile (					\
		__check("%0%1%2%3%4%5",					\
			"%%rax%%rax%%rdi%%rsi%%rdx%%r10")		\
			"syscall" __comment(name)			\
		      : "=a"(__sc_a0)					\
		      : "0"(__sc_a0),					\
			"r" (__sc_a1), "r"(__sc_a2), "r" (__sc_a3),	\
			"r" (__sc_a4)					\
		      : __syscall_clobbers				\
		);							\
		__sc_res = __sc_a0;					\
	}								\
	__syscall_return(type, __sc_res);				\
} 

#define _syscall5(type, name, type1, arg1, type2, arg2, type3, arg3,	\
    			      type4, arg4, type5, arg5)			\
type name (type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5)	\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(a0, "rax", __NR_##name);		\
		__syscall_regval(a1, "rdi", arg1);			\
		__syscall_regval(a2, "rsi", arg2);			\
		__syscall_regval(a3, "rdx", arg3);			\
		__syscall_regval(a4, "r10", arg4);			\
		__syscall_regval(a5, "r8", arg5);			\
									\
		__asm__ volatile (					\
		__check("%0%1%2%3%4%5%6",				\
			"%%rax%%rax%%rdi%%rsi%%rdx%%r10%%r8")		\
			"syscall" __comment(name)			\
		      : "=a"(__sc_a0)					\
		      : "0"(__sc_a0),					\
			"r" (__sc_a1), "r"(__sc_a2), "r" (__sc_a3),	\
			"r" (__sc_a4), "r"(__sc_a5)			\
		      : __syscall_clobbers				\
		);							\
		__sc_res = __sc_a0;					\
	}								\
	__syscall_return(type, __sc_res);				\
}

#define _syscall6(type, name, type1, arg1, type2, arg2, type3, arg3,	\
			type4, arg4, type5, arg5, type6, arg6)		\
type name (type1 arg1, type2 arg2, type3 arg3,				\
           type4 arg4, type5 arg5, type6 arg6)				\
{									\
	long __sc_res;							\
	{								\
		__syscall_regval(a0, "rax", __NR_##name);		\
		__syscall_regval(a1, "rdi", arg1);			\
		__syscall_regval(a2, "rsi", arg2);			\
		__syscall_regval(a3, "rdx", arg3);			\
		__syscall_regval(a4, "r10", arg4);			\
		__syscall_regval(a5, "r8", arg5);			\
		__syscall_regval(a6, "r9", arg6);			\
									\
		__asm__ volatile (					\
		__check("%0%1%2%3%4%5%6%7",				\
			"%%rax%%rax%%rdi%%rsi%%rdx%%r10%%r8%%r9")	\
			"syscall" __comment(name)			\
		      : "=a"(__sc_a0)					\
		      : "0"(__sc_a0),					\
			"r" (__sc_a1), "r"(__sc_a2), "r" (__sc_a3),	\
			"r" (__sc_a4), "r"(__sc_a5), "r" (__sc_a6)	\
		      : __syscall_clobbers				\
		);							\
		__sc_res = __sc_a0;					\
	}								\
	__syscall_return(type, __sc_res);				\
}


#endif

