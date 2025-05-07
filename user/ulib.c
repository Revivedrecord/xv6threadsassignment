#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

//
// wrapper so that it's OK if main() does not call exit().
//
void
start()
{
  extern int main();
  main();
  exit(0);
}

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
  char *cdst = (char *) dst;
  int i;
  for(i = 0; i < n; i++){
    cdst[i] = c;
  }
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
  if (src > dst) {
    while(n-- > 0)
      *dst++ = *src++;
  } else {
    dst += n;
    src += n;
    while(n-- > 0)
      *--dst = *--src;
  }
  return vdst;
}

int
memcmp(const void *s1, const void *s2, uint n)
{
  const char *p1 = s1, *p2 = s2;
  while (n-- > 0) {
    if (*p1 != *p2) {
      return *p1 - *p2;
    }
    p1++;
    p2++;
  }
  return 0;
}

void *
memcpy(void *dst, const void *src, uint n)
{
  return memmove(dst, src, n);
}
// Atomic add for RISC-V
uint
fetchadd(uint* addr, uint val)
{
  uint temp;
  __asm__ volatile(
    "amoadd.w %[temp], %[val], (%[addr])"
    : [temp] "=r" (temp)
    : [val] "r" (val), [addr] "r" (addr)
    : "memory"
  );
  return temp;
}

// Initialize lock
void
lock_init(lock_t *lock)
{
  lock->ticket = 0;
  lock->turn = 0;
}

// Acquire lock
void
lock_acquire(lock_t *lock)
{
  uint myturn = fetchadd(&lock->ticket, 1);
  
  while(lock->turn != myturn)
    ; // Spin
}

// Release lock
void
lock_release(lock_t *lock)
{
  fetchadd(&lock->turn, 1);
}

// Create a new thread
int
thread_create(void (*start_routine)(void *, void *), void *arg1, void *arg2)
{
  // Allocate stack (2 pages to ensure alignment)
  void *stack = malloc(2*PGSIZE);
  if(!stack)
    return -1;
  
  // Align stack to page boundary
  void *stack_aligned = (void*)(((uint64)stack + PGSIZE - 1) & ~(PGSIZE - 1));
  
  // Create thread
  int tid = clone(start_routine, arg1, arg2, stack_aligned);
  if(tid < 0) {
    free(stack);
    return -1;
  }
  
  // Return appropriate value
  if(tid == 0) // Child
    return 0;
  
  return tid; // Parent
}

// Join with a thread
int
thread_join(int tid)
{
  void *stack;
  
  int pid = join(&stack);
  if(pid < 0)
    return -1;
  
  // Free the thread's stack
  if(stack)
    free(stack);
  
  return pid;
}
