// Mutual exclusion spin locks.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "proc.h"
#include "defs.h"

void
initlock(struct spinlock *lk, char *name)
{
  lk->name = name;
  lk->locked = 0;
  lk->cpu = 0;
}

// Acquire the lock.
// Loops (spins) until the lock is acquired.
void
acquire(struct spinlock *lk)
{
  push_off(); // disable interrupts to avoid deadlock.
  if(holding(lk))
    panic("acquire");

  // On RISC-V, sync_lock_test_and_set turns into an atomic swap:
  //   a5 = 1
  //   s1 = &lk->locked
  //   amoswap.w.aq a5, a5, (s1)
  while(__sync_lock_test_and_set(&lk->locked, 1) != 0)
    ;

  // Tell the C compiler and the processor to not move loads or stores
  // past this point, to ensure that the critical section's memory
  // references happen strictly after the lock is acquired.
  // On RISC-V, this emits a fence instruction.
  __sync_synchronize();

  // Record info about lock acquisition for holding() and debugging.
  lk->cpu = mycpu();
}

// Release the lock.
void
release(struct spinlock *lk)
{
  if(!holding(lk))
    panic("release");

  lk->cpu = 0;

  // Tell the C compiler and the CPU to not move loads or stores
  // past this point, to ensure that all the stores in the critical
  // section are visible to other CPUs before the lock is released,
  // and that loads in the critical section occur strictly before
  // the lock is released.
  // On RISC-V, this emits a fence instruction.
  __sync_synchronize();

  // Release the lock, equivalent to lk->locked = 0.
  // This code doesn't use a C assignment, since the C standard
  // implies that an assignment might be implemented with
  // multiple store instructions.
  // On RISC-V, sync_lock_release turns into an atomic swap:
  //   s1 = &lk->locked
  //   amoswap.w zero, zero, (s1)
  __sync_lock_release(&lk->locked);

  pop_off();
}

// Check whether this cpu is holding the lock.
// Interrupts must be off.
int
holding(struct spinlock *lk)
{
  int r;
  r = (lk->locked && lk->cpu == mycpu());
  return r;
}

// push_off/pop_off are like intr_off()/intr_on() except that they are matched:
// it takes two pop_off()s to undo two push_off()s.  Also, if interrupts
// are initially off, then push_off, pop_off leaves them off.

void
push_off(void)
{
  int old = intr_get();

  intr_off();
  if(mycpu()->noff == 0)
    mycpu()->intena = old;
  mycpu()->noff += 1;
}

void
pop_off(void)
{
  struct cpu *c = mycpu();
  if(intr_get())
    panic("pop_off - interruptible");
  if(c->noff < 1)
    panic("pop_off");
  c->noff -= 1;
  if(c->noff == 0 && c->intena)
    intr_on();
}


/**
 * 
__sync_val_compare_and_swap  这个函数是哪里提供的？
__sync_val_compare_and_swap 是一个原子操作函数，由 GNU编译器集合（GCC） 提供1。这个函数会比较 __compVal 的值和 __p 指向的变量的值。
如果它们相等，__exchVal 的值就会被存储在 __p 指向的地址中；否则，不执行任何操作2。当这个函数被调用时，会创建一个完整的内存屏障2。
这个函数的原型是 T __sync_val_compare_and_swap ( T__exchVal , ...)2。其中，__p 是指向要与 __compVal 比较的变量的指针，
__compVal 是要与 __p 指向的变量的值进行比较的值，__exchVal 是要存储在指向的地址的值2。这个函数返回 __p 指向的变量的初始值2。
这个函数在 IBM2 和 GCC1 的文档中都有详细的描述。你可以参考这些文档来了解更多关于这个函数的信息。希望这个答案对你有所帮助！


是不是只要是GCC编译的程序，都可以用到这个函数？
是的，__sync_val_compare_and_swap 是 GNU编译器集合（GCC） 提供的内置函数。只要你的程序是用 GCC 编译的，你就可以在你的程序中使用这个函数。
但是，你需要注意的是，这个函数是 GCC 的特性，不是 C 或 C++ 标准的一部分。因此，如果你的代码需要在非 GCC 环境中编译，你可能需要找到一个替代的
实现或者使用其他的原子操作库，比如 C++11 提供的 <atomic> 库。希望这个答案对你有所帮助！（已编辑）


////////////////

__sync_synchronize  这个函数是哪里提供的？
__sync_synchronize 是由 GNU Compiler Collection (GCC) 提供的内置函数1。这些函数是为了与 Intel Itanium Processor-specific Application Binary Interface
描述的函数兼容而设计的1。这些函数在 GCC 中的实现是基于 __atomic 内置函数1。新的代码应该使用 __atomic 内置函数，而不是 __sync 内置函数1。
这就是 __sync_synchronize 函数的来源。

*/
