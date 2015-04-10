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
#include <sys/user.h>
#include <sys/syscall.h>
#include <sys/reg.h>

const int long_size = sizeof(long);

/* Reverse the input string. */
void reverse(char *str); 
/* Extract "len" Bytes of data pointed by "addr" and returned it in "str". */
void getdata(pid_t child, long addr, char *str, int len);
/* Put "len" Bytes of the data contained in "str" to the address "addr". */
void putdata(pid_t child, long addr, char *str, int len);

int main()
{
   pid_t child;
   child = fork();
   if(child == 0) 
   {
      ptrace(PTRACE_TRACEME, 0, NULL, NULL);
      execl("/bin/ls", "ls", NULL);
   }
   else 
   {
      long orig_rax;
      long params[3];
      int status;
      char *str, *laddr;
      int insyscall = 0;
      struct user_regs_struct regs;
      int compteur = 0;
      
      while(1) 
      {
			 wait(&status);
			 if(WIFEXITED(status)) break;      /* This macro returns a nonzero value 
												* if the child process terminated normally with exit or _exit. 
												*/
			 orig_rax = ptrace(PTRACE_PEEKUSER, child, 8 * ORIG_RAX, NULL);
			 if(orig_rax == SYS_write) 
			 {
					if(insyscall == 0) 
					{
						++compteur;
						printf("\vSyscall entry %d\n", compteur);
						printf("\n =============================== ");

						insyscall = 1;
						/* Getting the content of all registers in the regs structure. */
						ptrace(PTRACE_GETREGS, child, NULL, &regs);
						  
						params[0] = (long)regs.rdi;
						params[1] = (long)regs.rsi;
						params[2] = (long)regs.rdx;

						/*printf("\nThe system call Write is called with the following parameters:\n arg1=%lu\n arg2=%lu\n arg3=%lu\n\n",
						 *	   params[0], params[1], params[2]);
						 */												
						str = (char *)calloc((params[2]+1), sizeof(char));
							
						getdata(child, params[1], str, params[2]);
						printf("\n%s =============================== ",str);
						reverse(str);
						putdata(child, params[1], str, params[2]);
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

