#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"

char*
strcpy(char *s, const char *t)
{
  char *os;

  os = s;
  while((*s++ = *t++) != 0)
    ;
  return os;
}

int
strcmp(const char *p, const char *q)
{
  while(*p && *p == *q)
    p++, q++;
  return (uchar)*p - (uchar)*q;
}

uint
strlen(const char *s)
{
  int n;

  for(n = 0; s[n]; n++)
    ;
  return n;
}

void*
memset(void *dst, int c, uint n)
{
  stosb(dst, c, n);
  return dst;
}

char*
strchr(const char *s, char c)
{
  for(; *s; s++)
    if(*s == c)
      return (char*)s;
  return 0;
}

char*
gets(char *buf, int max)
{
  int i, cc;
  char c;

  for(i=0; i+1 < max; ){
    cc = read(0, &c, 1);
    if(cc < 1)
      break;
    buf[i++] = c;
    if(c == '\n' || c == '\r')
      break;
  }
  buf[i] = '\0';
  return buf;
}

int
stat(const char *n, struct stat *st)
{
  int fd;
  int r;

  fd = open(n, O_RDONLY);
  if(fd < 0)
    return -1;
  r = fstat(fd, st);
  close(fd);
  return r;
}

int
atoi(const char *s)
{
  int n;

  n = 0;
  while('0' <= *s && *s <= '9')
    n = n*10 + *s++ - '0';
  return n;
}

void*
memmove(void *vdst, const void *vsrc, int n)
{
  char *dst;
  const char *src;

  dst = vdst;
  src = vsrc;
  while(n-- > 0)
    *dst++ = *src++;
  return vdst;
}

uint
fetchadd(uint *addr, uint val)
{
  uint res;
  
  asm volatile("lock; xaddl %0, %1" :
               "+r" (val), "+m" (*addr) :
               : "memory");
  res = val;
  return res;
}

void
lock_init(lock_t *lk)
{
  lk->ticket = 0;
  lk->turn = 0;
}

void
lock_acquire(lock_t *lk)
{
  uint myturn = fetchadd(&lk->ticket, 1);
  
  while(lk->turn != myturn)
    ; // Spin
}

void
lock_release(lock_t *lk)
{
  fetchadd(&lk->turn, 1);
}

int
thread_create(void (*fn)(void *, void *), void *a1, void *a2)
{
  void *stk = malloc(2 * PGSIZE);
  if(!stk)
    return -1;
  
  void *aligned_stk = (void*)(((uint)stk + PGSIZE - 1) & ~(PGSIZE - 1));
  
  int tid = clone(fn, a1, a2, aligned_stk);
  if(tid < 0) {
    free(stk);
    return -1;
  }
  
  if(tid == 0)
    return 0;
  
  return tid;
}

int
thread_join(int tid)
{
  void *stk;
  
  int pid = join(&stk);
  if(pid < 0)
    return -1;
  
  if(stk)
    free(stk);
  
  return pid;
}
