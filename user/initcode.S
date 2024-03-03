# Initial process that execs /init.
# This code runs in user space.

#include "syscall.h"

# exec(init, argv)
.globl start
start:
        la a0, init
        la a1, argv
        li a7, SYS_exec
        ecall

# for(;;) exit();
exit:
        li a7, SYS_exit
        ecall
        jal exit

# char init[] = "/init\0";
init:
  .string "/init\0"

# char *argv[] = { init, 0 };
.p2align 2
argv:
  .long init
  .long 0


# 这里的代码是如何运行的？
# 在userinit函数里面，将 trapframe中保存了epc的值为0，这样在返回用户空间时，pc寄存器就会恢复为epc保存的值，程序就会从地址 0x0 开始执行
# 也就是开始执行initcode中的代码
#
# 在 userinit 后面，进程状态机设置为 RUNNABLE, 那么这里的代码就会在 main函数调用 scheduler函数时，开始遍历到，并开始执行
# 这样，第一个用户程序 initcode 就开始运行了
# ref: https://juejin.cn/post/7308621051525120037
#      https://jianzzz.github.io/2017/08/20/%E7%AC%94%E8%AE%B007-xv6-%E5%90%AF%E5%8A%A8%E5%88%B0%E6%89%A7%E8%A1%8C%E7%AC%AC%E4%B8%80%E4%B8%AA%E8%BF%9B%E7%A8%8B/


# ecall：  CPUs provide a special instruction that switches the CUP from user mode to supervisor mode and enters the kernel at  an entry point specified by the kernel
#   RISC-V provide the ecall instruction for this purpose.
