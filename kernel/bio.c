// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

struct {
  struct spinlock lock; // only for buf
  struct buf buf[NBUF];
  struct hashtable htable[HASHPRIME];
} bcache;

void
binit (void)
{
  struct buf *b;
  struct hashtable *ht;

  initlock (&bcache.lock, "bcache");
  
  for (ht = bcache.htable; ht < bcache.htable + HASHPRIME; ++ht)
    {
      initlock(&ht->lock, "bcache_hashtable"); 
      ht->type = 0;
    }

  // Create linked list of buffers
  for (b = bcache.buf; b < bcache.buf+NBUF; b++)
    initsleeplock(&b->lock, "bcache_buffer");
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget (uint dev, uint blockno)
{
  struct hashtable *ht, *htable;
  struct buf *b;
  uint hashno, bufno;

  acquire (&bcache.lock);
  htable = bcache.htable;
  hashno = blockno % HASHPRIME;
  bufno  = -1;

  // Is the block already cached?
  while (1)
    {
      ht = &htable[hashno];

      acquire (&ht->lock);
      // Not in used
      if (ht->type == 0)  
        {
          ht->type = 1;
          goto endcachedbget;
        }
      else if (ht->dev == dev && ht->blockno == blockno)
        {
          bufno = ht->bufno;
          goto endcachedbget;
        }

      hashno = (hashno + 1) % HASHPRIME;
      // Only if HASHPRIME > NBUF
      if (hashno == blockno)
        panic ("bget: check if cached");  
    }

endcachedbget:
  release (&ht->lock);

  if (bufno != -1)
    { 
      b = &bcache.buf[bufno];
      acquiresleep (&b->lock);
      b->refcnt++;
      release (&bcache.lock);
      return b;
    }

  // Not cached.
  for (bufno = 0; bufno < NBUF; bufno++)
    {
      b = bcache.buf + bufno;
      if(b->refcnt == 0) 
        {
          b->dev = dev;
          b->blockno = blockno;
          b->valid = 0;
          b->refcnt = 1;
          b->hashno = hashno;
          release(&bcache.lock);
          
          acquire (&ht->lock);
          ht->dev = dev;
          ht->blockno = blockno;
          ht->bufno = bufno;
          release (&ht->lock);
  
          acquiresleep(&b->lock);
          return b;
        }
    }      
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  acquire(&bcache.lock);
  b->refcnt--;
  release(&bcache.lock);
}

void
bpin(struct buf *b) {
  acquire(&bcache.lock);
  b->refcnt++;
  release(&bcache.lock);
}

void
bunpin(struct buf *b) {
  acquire(&bcache.lock);
  b->refcnt--;
  release(&bcache.lock);
}


