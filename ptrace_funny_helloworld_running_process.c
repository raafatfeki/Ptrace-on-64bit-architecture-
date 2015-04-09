#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <sys/reg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
const int long_size = sizeof(long);

void reverse(char *str)
{   int i, j;
    char temp;
    for(i = 0, j = strlen(str) - 2;
        i <= j; ++i, --j) {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
    }
}

void getdata(pid_t child, long addr,
             char *str, int len)
{   char *laddr;
    int i, j;
    union u {
            long val;
            char chars[long_size];
    }data;
    i = 0;
    j = len / long_size;
    laddr = str;
    while(i < j) {
        data.val = ptrace(PTRACE_PEEKDATA,
                          child, addr + i * 8,
                          NULL);
        memcpy(laddr, data.chars, long_size);
        ++i;
        laddr += long_size;
    }
    j = len % long_size;
    if(j != 0) {
        data.val = ptrace(PTRACE_PEEKDATA,
                          child, addr + i * 8,
                          NULL);
        memcpy(laddr, data.chars, j);
    }
    str[len] = '\0';
    printf("%s",str);
}   
	
void putdata(pid_t child, long addr,
             char *str, int len)
{   char *laddr;
    int i, j;
    union u {
            long val;
            char chars[long_size];
    }data;
    i = 0;
    j = len / long_size;
    laddr = str;
    while(i < j) {
        memcpy(data.chars, laddr, long_size);
        ptrace(PTRACE_POKEDATA, child,
               addr + i * 8, data.val);
        ++i;
        laddr += long_size;
    }
    j = len % long_size;
    if(j != 0) {
        memcpy(data.chars, laddr, j);
        ptrace(PTRACE_POKEDATA, child,
               addr + i * 8, data.val);
    }
}

int main(int argc, char *argv[])
{   
	pid_t traced_process;
   long orig_eax, eax;
    long params[3];
    int status;
    int toggle = 0;
    struct user_regs_struct regs;
    long res;
    char* str;
    if(argc != 2) {
        printf("Usage: %s <pid to be traced>\n",argv[1]);
        exit(1);
    }
    traced_process = atoi(argv[1]);
    printf("%d\n",traced_process);
    res =ptrace(PTRACE_ATTACH,traced_process, NULL, NULL);
    if (res!= 0)
    {printf("%ld\n",res);}
	while(1) {
         wait(&status);
         if(WIFEXITED(status))
             break;
         orig_eax = ptrace(PTRACE_PEEKUSER,
                           traced_process, 8 * ORIG_RAX,
                           NULL);
         if(orig_eax == SYS_write) {
            if(toggle == 0) {
               toggle = 1;
               ptrace(PTRACE_GETREGS, traced_process, NULL, &regs);
               printf("The child made a system call with syscall_number: %llu \n", regs.orig_rax);
                 printf("Write called with the arguments"
                        "%llu, %llu, %llu\n",
                        regs.rbx, regs.rcx,
                        regs.rdx);
               
               
               
               params[0] = (long)regs.rdi;
               params[1] = (long)regs.rsi;
               params[2] = (long)regs.rdx;
               params[3] = (long)regs.r10;
               params[4] = (long)regs.r8;
               params[5] = (long)regs.r9;

               printf("The child made a system call with syscall_number: %llu \n", regs.orig_rax);
                 printf("Write called with the arguments"
                        "%lu, %lu, %lu,%lu,%lu, %lu\n",
                        params[0], params[1],
                        params[2],params[3],params[4],params[5]);
               str = (char *)calloc((params[2]+1)
                                ,sizeof(char));
				
               getdata(traced_process, params[1], str,
                       params[2]);
               reverse(str);
               putdata(traced_process, params[1], str,
                       params[2]);
            }
            else {
               toggle = 0;
            }
         }
      ptrace(PTRACE_SYSCALL, traced_process, NULL, NULL);
      }
    return 0;
}
