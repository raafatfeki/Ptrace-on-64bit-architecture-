/* This code aims at changing the behaviour of a child process by changing the syscall itself.
 * It supports the INTEL x86_64bit system. 
 * Please see the 3 ptrace exemple and the ptrace_reverse before trying this code.
 * There are some difference between INTEL x86_32bit system and INTEL x86_64bit system. 
 * Please see the diff_32_64.txt file if there are some ambiguity concerning the system architecture.
 * The basic Ptrace operation are mentioned and explained in the following link:
 * 									"https://mikecvet.wordpress.com/2010/08/14/ptrace-tutorial/"
 * Please see the manual page of ptrace(2) for further details.
 */
 
#include <stdio.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/reg.h>

int main()
{
   pid_t child;
   pid_t pid_value;
   pid_t child_pid;
   pid_t parent_pid;
   
   child = fork();
   if(child == 0) 
   {
		child_pid = getpid();
		printf("The child PID is: %d\n", (int)child_pid);
		ptrace(PTRACE_TRACEME, 0, NULL, NULL);
		/* Change the execl arguments by the process you want to initiate.
		 * Use the hello_world example for testing.
		 */
		execl("./helloworld_dynamic", "helloworld_dynamic", NULL);
   }
   else 
   {
		long orig_rax;
		long params[5];
		int status;
		int insyscall = 0;
		struct user_regs_struct regs;
		
		parent_pid = getpid();
		
		printf("The Parent PID is: %d\n", (int)parent_pid);
		while(1) 
		{
			 wait(&status);
			 if(WIFEXITED(status)) break;
			 
			 orig_rax = ptrace(PTRACE_PEEKUSER, child, 8 * ORIG_RAX, NULL);
			 
			 if(orig_rax == 39) /* The hello_world example do not use the getpid system call. 
								 * If there are a system call to the getpid, then we can judge that the write system call has changed. 
								 */
			 {
					printf("The process use the getpid system call:\n");
					printf("============================================\n");
					ptrace(PTRACE_GETREGS, child, NULL, &regs);
					printf("The syscall 'getpid' returns the value %llu\n",regs.rax);
					printf("============================================\n");

			 }
			 
			 if(orig_rax == SYS_write) 
			 {
				if(insyscall == 0) 
				{
				   insyscall = 1;
				   ptrace(PTRACE_GETREGS, child, NULL, &regs);
				  
				   params[0] = (long)regs.rdi;
				   params[1] = (long)regs.rsi;
				   params[2] = (long)regs.rdx;
					
				   printf("\nThe system call Write is called with the following parameters:\n arg1=%ld\n arg2=%ld\n arg3=%ld\n\n",
						  params[0], params[1], params[2]);
				   
				   printf("Changing the registers values and setting the rax register to the 'getpid' syscall_number 39.\n");
				   regs.orig_rax = 39;
				   regs.rdi = 0;
				   regs.rsi = 0;
				   regs.rdx = 0;
				   regs.r10 = 0;
				   ptrace (PTRACE_SETREGS, child, NULL, &regs);
				   
				   params[0] = (long)regs.rdi;
				   params[1] = (long)regs.rsi;
				   params[2] = (long)regs.rdx;
					
				   printf("\nThe system call Write is replaced by the getpid system call.\n"
				          "The new parameters are:\n arg1=%ld\n arg2=%ld\n arg3=%ld\n\n",
						  params[0], params[1], params[2]);
            }
            else 
            {
               insyscall = 0;
            }
        }
		ptrace(PTRACE_SYSCALL, child, NULL, NULL);
      }
   }
   return 0;
}
