/* This code aims at changing the behaviour of a running process by changing the syscall itself.
 * It supports the INTEL x86_64bit system. 
 * Please see the 3 ptrace exemple and the ptrace_reverse before trying this code.
 * There are some difference between INTEL x86_32bit system and INTEL x86_64bit system. 
 * Please see the diff_32_64.txt file if there are some ambiguity concerning the system architecture.
 * The basic Ptrace operation are mentioned and explained in the following link:
 * 									"https://mikecvet.wordpress.com/2010/08/14/ptrace-tutorial/"
 * Please see the manual page of ptrace(2) for further details.
 */
 
 
 /* We will use "process_to_intercept.c" as a testing process.
  * The file "process_to_intercept.c" is cloned from "http://www.linuxjournal.com/article/6100" (Tutorial on INTEL x86_32bit system).
  * You should compile and execute it in background and give its PID as an argument to this program after running it. 
  */
  
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <sys/reg.h>
#include <errno.h>

const int long_size = sizeof(long);

/* Reverse the input string. */
void reverse(char *str); 
/* Extract "len" Bytes of data pointed by "addr" and returned it in "str". */
void getdata(pid_t child, long addr, char *str, int len);
/* Put "len" Bytes of the data contained in "str" to the address "addr". */
void putdata(pid_t child, long addr, char *str, int len);

int main(int argc, char *argv[])
{   
	pid_t traced_process;
    long orig_rax, rax;
    long params[3];
    int status;
    int insyscall = 0;
    struct user_regs_struct regs;
    char* str;
    long res;
    int counter = 0;
    if(argc != 2) 
    {
        printf("You should give the PID of the process to intercep.\n");
        exit(1);
    }
    
    traced_process = atoi(argv[1]);
    printf("The PID of the traced process is: %d\n",traced_process);
    res =ptrace(PTRACE_ATTACH,traced_process, NULL, NULL);
    if (res!= 0)
		if (errno==1)
			{
				printf("This operation is not permetted.\n"
					"Please see the readme file(the PTRACE_ATTACH section) to solve this problem.\n");
				exit(1);
			}
	
	while(1) 
	{
         wait(&status);
         if(WIFEXITED(status)) break;
         orig_rax = ptrace(PTRACE_PEEKUSER, traced_process, 8 * ORIG_RAX, NULL);
        
         if(orig_rax == SYS_write) 
			 {
					if(insyscall == 0) 
					{
						++counter;
						printf("\vSyscall entry %d\n", counter);
						printf("\n =============================== ");

						insyscall = 1;
						/* Getting the content of all registers in the regs structure. */
						ptrace(PTRACE_GETREGS, traced_process, NULL, &regs);
						  
						params[0] = (long)regs.rdi;
						params[1] = (long)regs.rsi;
						params[2] = (long)regs.rdx;

						/*printf("\nThe system call Write is called with the following parameters:\n arg1=%lu\n arg2=%lu\n arg3=%lu\n\n",
						 *	   params[0], params[1], params[2]);
						 */												
						str = (char *)calloc((params[2]+1), sizeof(char));
							
						getdata(traced_process, params[1], str, params[2]);
						printf("\n%s =============================== ",str);
						reverse(str);
						putdata(traced_process, params[1], str, params[2]);
					}
					else 
					{
					   insyscall = 0;
					}
					
			}
				  
				  ptrace(PTRACE_SYSCALL, traced_process, NULL, NULL);
		}
		 
	return 0;

    }

void reverse(char *str)
{   
	int i, j;
    char temp;
    
    for(i = 0, j = strlen(str) - 2; i <= j; ++i, --j) 
    {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
    }
}

void getdata(pid_t child, long addr, char *str, int len)
{   
	char *laddr;
    int i, j;
    union u {
            long val;
            char chars[long_size];
    }data;
    
    i = 0;
    j = len / long_size;
    laddr = str;
    while(i < j) 
    {
        data.val = ptrace(PTRACE_PEEKDATA,child, addr + i * 8, NULL);
        memcpy(laddr, data.chars, long_size);
        ++i;
        laddr += long_size;
    }
    
    j = len % long_size;
    if(j != 0) 
    {
        data.val = ptrace(PTRACE_PEEKDATA, child, addr + i * 8, NULL);
        memcpy(laddr, data.chars, j);
    }
    str[len] = '\0';
}   

	
void putdata(pid_t child, long addr, char *str, int len)
{   
	char *laddr;
    int i, j;
    union u {
            long val;
            char chars[long_size];
    }data;
    i = 0;
    j = len / long_size;
    laddr = str;
    
    while(i < j)
    {
        memcpy(data.chars, laddr, long_size);
        ptrace(PTRACE_POKEDATA, child, addr + i * 8, data.val);
        ++i;
        laddr += long_size;
    }
    j = len % long_size;
    if(j != 0) 
    {
        memcpy(data.chars, laddr, j);
        ptrace(PTRACE_POKEDATA, child,addr + i * 8, data.val);
    }
}
