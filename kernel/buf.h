struct buf {
  int valid;   // has data been read from disk?
  int disk;    // does disk "own" buf?
  uint dev;
  uint blockno;
  uint hashno;
  struct sleeplock lock;
  uint refcnt;
  uchar data[BSIZE];
};

#define HASHPRIME 137

struct hashtable {
  uint type;   // 0: not existed. 1: in used  
  uint dev;
  uint blockno;   
  uint bufno;
  struct spinlock lock;
};

