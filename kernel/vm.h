// page mode
#define KPAGEMODE 0 // 4KiB
#define MPAGEMODE 1 // 2MiB

#define PG_GETLEVEL(pgmode)\
((pgmode) == KPAGEMODE ? 0 :\
((pgmode) == MPAGEMODE ? 1 :\
-1))

#define PG_GETMODE(start, end)\
((start) + PG_GETSIZE (MPAGEMODE) <= (end) ? MPAGEMODE : KPAGEMODE)

#define PG_GETSIZE(pgmode)\
((pgmode) == KPAGEMODE ? PGSIZE :\
((pgmode) == MPAGEMODE ? SUPERPGSIZE :\
-1))

#define PG_ROUNDUP(pgmode, sz)\
((pgmode) == KPAGEMODE ? PGROUNDUP(sz) :\
((pgmode) == MPAGEMODE ? SUPERPGROUNDUP(sz) :\
-1))

#define PG_GETALLOC(pgmode)\
((pgmode) == KPAGEMODE ? &kalloc :\
((pgmode) == MPAGEMODE ? &superalloc :\
0))

#define PG_GETFREE(pgmode)\
((pgmode) == KPAGEMODE ? &kfree :\
((pgmode) == MPAGEMODE ? &superfree :\
0))

