/* This code is inspired by the code written in "http://www.linuxjournal.com/article/6100" (INTEL x86_32bit system).
 * It is rectified to support the INTEL x86_64bit system. All changes are commented and justified in diff_32_64.txt file.
 * The basic Ptrace operation are mentioned and explained in the following link:
 * 									"https://mikecvet.wordpress.com/2010/08/14/ptrace-tutorial/"
 * Please see the manual page of ptrace(2) for further details.
 */
 
#include <stdio.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/reg.h>
#include <sys/syscall.h>   /* It contains the syscall numbers associated macros.
							* For further details concerning the diffrence between the x86_64bit and x86_32bit files,
							* please see the diff_32_64.txt file.
							*/

int main()
{   
	pid_t child;
    long orig_rax, rax;
    long params[3];
    int status;
    int insyscall = 0;
    
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
				  if(orig_rax == SYS_write) /* You can use SYS_write or __NR_write. 
											 * For further details, please see the diff_32_64.txt file.
											 */
				  {
						if(insyscall == 0) 
						{
								printf("\vSyscall entry\n");
								insyscall = 1;
								/*Getting the parameters from the associated registers.*/
								params[0] = ptrace(PTRACE_PEEKUSER, child, 8 * RDI, NULL);
								params[1] = ptrace(PTRACE_PEEKUSER, child, 8 * RSI, NULL);
								params[2] = ptrace(PTRACE_PEEKUSER, child, 8 * RDX, NULL);
								printf("\nThe system call Write is called with the following parameters:\n arg1=%ld\n arg2=%ld\n arg3=%ld\n\n",
									   params[0], params[1], params[2]);
						}
						else 
						{
								printf("Syscall exit\n");
								/*Getting the returned value from the associated register.*/
								rax = ptrace(PTRACE_PEEKUSER, child, 8 * RAX, NULL);
								printf("Write returned with %ld\n", rax);
								insyscall = 0;
						}
					
				  }
					
				  ptrace(PTRACE_SYSCALL, child, NULL, NULL);
			}
    }
    return 0;
}
