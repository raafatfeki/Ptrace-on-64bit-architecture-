#include <stdio.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* See the readme file for further details about user.h et reg.h*/

#include <sys/user.h> 
#include <sys/reg.h>

/* This code is inspired by the code written in "http://www.linuxjournal.com/article/6100" (INTEL x86_32bit system).
 * It is rectified to support the INTEL x86_64bit system. All changes are commented and justified in diff_32_64.txt file.
 * The basic Ptrace operation are mentioned and explained in the following link: https://mikecvet.wordpress.com/2010/08/14/ptrace-tutorial/
 * Please see the manual page of ptrace(2) for further details.
 * /*/
 
int main()
{   
	pid_t child;
    long orig_rax;
    child = fork();
    if(child == 0) 
    {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        /* Change the execl arguments by the process you want to initiate.*/
        execl("./helloworld_dynamic", "helloworld_dynamic", NULL);
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
