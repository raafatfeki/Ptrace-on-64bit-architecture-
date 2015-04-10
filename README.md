# Ptrace-on-64bit-architecture-

I- Introdution:

This repository contains 6 different programs. Each one of is independent from the others.
The 3 examples aims at initiating the programmers to the use of "ptrace".
	
	EXP1 -- It extract the RAX register content which represents the syscall_number called.
	EXP2 -- It extract the RAX, RDI, RSI,RDX regiters contents which represent respectively the syscall_number, parameters 1,2 et 3 used 			by this syscall.
	EXP3 -- It do the job as EXP2 but with a different way which can be useful to extract all the register contents.

The "ptrace_reverse.c" aims at intercpting the arguments given to any write system call and reversing the string to be written.
The "ptrace_write_to_getpid.c" aims at intercepting the write system call and change it with a getpid system call.
The "ptrace_reverse_running_process.c" aims at doing the same job as "ptrace_reverse.c". But, it is used with running process.

II- C Standard Utilities Library Used:

	1)sys/reg.h:
	    
	    Index into an array of 8 byte longs returned from ptrace for location of the users' stored general purpose registers.
		 Exp:
		      # define RAX    10
		      # define RCX    11
		      # define RDX    12

	2)sys/syscall.h:
	    
	    It contains the syscall numbers associated macros.


	3)sys/user.h:

	    It defines the necessary structures for the extracting of information.
	    	 Exp:
		      It defines the user_regs_struct which contains all the registers values. 

III- Ptrace operations:

You can find all necessary information about Ptrace operations in the following link:
		     "https://mikecvet.wordpress.com/2010/08/14/ptrace-tutorial/"


IV- Errors:

While using the "PTRACE_ATTACH" operation, you may have the following error: "This operation is not permetted".
In fact, PTRACE_ATTACH aims at attaching a given running process to the program as a child.
Therefore, you need to set the value in the following path to 0.

	For permanent effect: /etc/sysctl.d/10-ptrace.conf
	For temporary effect: /proc/sys/kernel/yama/ptrace_scope







