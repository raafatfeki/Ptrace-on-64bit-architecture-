/* This code is inspired by the code written in "http://www.linuxjournal.com/article/6100" (INTEL x86_32bit system).
 * It is rectified to support the INTEL x86_64bit system. All changes are commented and justified in diff_32_64.txt file.
 * The basic Ptrace operation are mentioned and explained in the following link:
 * 									"https://mikecvet.wordpress.com/2010/08/14/ptrace-tutorial/"
 * Please see the manual page of ptrace(2) for further details.
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/syscall.h>
/* Please see the readme file for further details about user.h et reg.h.
 * For further details concerning the diffrence between the x86_64bit and x86_32bit files,
 * please see the diff_32_64.txt file.*/
#include <sys/user.h>
#include <sys/reg.h>



int main()
{   
	pid_t child;
    long orig_rax, rax;/* orig_rax contains the syscall_number and rax contains the returned value from the syscall. */
    long params[3];
    int status;
    int insyscall = 0;
    struct user_regs_struct regs;/* This structure is defined under sys/user.h */
    
    child = fork();
    if(child == 0) 
    {
		ptrace(PTRACE_TRACEME, 0, NULL, NULL);
		/* Change the execl arguments by the process you want to initiate. */
		execl("/bin/ls", "ls", NULL);
    }
    else 
    {
			while(1) 
			{
				  wait(&status);
				  if(WIFEXITED(status)) break; /* This macro returns a nonzero value 
												* if the child process terminated normally with exit or _exit. 
												*/
				  orig_rax = ptrace(PTRACE_PEEKUSER, child, 8 * ORIG_RAX, NULL);
				  if(orig_rax == SYS_write) 
				  {
						if(insyscall == 0) 
						{
							printf("\vSyscall entry\n");
							insyscall = 1;
							/* Getting the content of all registers in the regs structure. */
							ptrace(PTRACE_GETREGS, child, NULL, &regs);
							printf("\nThe system call Write is called with the following parameters:\n arg1=%llu\n arg2=%llu\n arg3=%llu\n\n",
									   regs.rdi, regs.rsi, regs.rdx);
						}
						else 
						{ 
							printf("Syscall exit\n");
							/* Refresh the content of regs after the system call.  */
							ptrace(PTRACE_GETREGS, child, NULL, &regs);
							printf("Write returned with %llu\n", regs.rax);
							insyscall = 0;
						}
						
				  }
				  
				  ptrace(PTRACE_SYSCALL, child, NULL, NULL);
			}
    }
    return 0;
}
