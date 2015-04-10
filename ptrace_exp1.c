/* This code is inspired by the code written in "http://www.linuxjournal.com/article/6100" (INTEL x86_32bit system).
 * It is rectified to support the INTEL x86_64bit system. All changes are commented and justified in diff_32_64.txt file.
 * The basic Ptrace operation are mentioned and explained in the following link:
 * 									"https://mikecvet.wordpress.com/2010/08/14/ptrace-tutorial/"
 * Please see the manual page of ptrace(2) for further details.
 * /*/
 
#include <stdio.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
/* Please see the readme file for further details about reg.h.
 * For further details concerning the diffrence between the x86_64bit and x86_32bit files,
 * please see the diff_32_64.txt file.*/
#include <sys/reg.h>
 
int main()
{   
	pid_t child;
    long orig_rax;
    child = fork();
    if(child == 0) 
    {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        /* Change the execl arguments by the process you want to initiate.*/
        execl("./helloworld_dynamic", "helloworld_dynamic", NULL);/* A sample helloworld compiled dynamically*/
    }
    else 
    {
		/* If you are tracing multiple tasks simultaneously use the waitpid() function */
        wait(NULL);
        /* See the diff_32_64.txt file.*/
        orig_rax = ptrace(PTRACE_PEEKUSER, child, 8 * ORIG_RAX,NULL);
        printf("The child made a system call with the sys_call_number: %ld\n", orig_rax);
        ptrace(PTRACE_CONT, child, NULL, NULL);
    }
    return 0;
}
