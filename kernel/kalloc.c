// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  // freelist lock
  struct spinlock lock[NCPU];
  struct run *freelist[NCPU];
} kmem;

void
kinit()
{
  for (uint i = 0; i < NCPU; ++i)
    initlock(&kmem.lock[i], "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;
  int hart;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;
  push_off();
  hart = cpuid();

  acquire(&kmem.lock[hart]);
  r->next = kmem.freelist[hart];
  kmem.freelist[hart] = r;
  release(&kmem.lock[hart]);
  pop_off();
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;
  int myhart, hart;
  
  r = 0;
  push_off();
  myhart = cpuid();
  hart = myhart;

  while(r == 0)
    { 
      acquire(&kmem.lock[hart]);
      r = kmem.freelist[hart];
      if(r)
        kmem.freelist[hart] = r->next;
      release(&kmem.lock[hart]);
      
      hart = (hart + 1) % NCPU;
      if(hart == myhart)
        break;
    }
  pop_off();

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
