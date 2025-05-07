#include "types.h"
#include "stat.h"
#include "user.h"

#define NTHREADS    4
#define NINCREMENTS 10000
#define NLOCKS      2

volatile int counter = 0;
volatile int thread_counts[NTHREADS] = {0};
lock_t countlock;
lock_t locks[NLOCKS];

struct targ {
  int id;
  int incs;
  lock_t *lk;
};

void
worker(void *arg1, void *arg2)
{
  struct targ *args = (struct targ*)arg1;
  int pat = (int)arg2;
  int id = args->id;
  int n = args->incs;
  lock_t *lk = args->lk;
  
  printf(1, "Thread %d: Starting (pattern %d)\n", id, pat);
  
  switch(pat) {
    case 0:
      for(int i = 0; i < n; i++) {
        lock_acquire(lk);
        counter++;
        thread_counts[id]++;
        lock_release(lk);
      }
      break;
      
    case 1:
      for(int i = 0; i < n; i++) {
        lock_acquire(lk);
        counter++;
        lock_release(lk);
        
        thread_counts[id]++;
      }
      break;
      
    case 2:
      for(int i = 0; i < n; i++) {
        lock_t *cur = &locks[i % NLOCKS];
        lock_acquire(cur);
        counter++;
        thread_counts[id]++;
        lock_release(cur);
      }
      break;
      
    case 3:
      for(int i = 0; i < n; i++) {
        void *mem = malloc(8);
        lock_acquire(lk);
        counter++;
        thread_counts[id]++;
        lock_release(lk);
        free(mem);
      }
      break;
  }
  
  printf(1, "Thread %d: Done (%d incs)\n", id, thread_counts[id]);
  exit();
}

int
main(int argc, char *argv[])
{
  int tids[NTHREADS];
  struct targ args[NTHREADS];
  
  printf(1, "Test: %d threads, %d increments each\n",
         NTHREADS, NINCREMENTS);
  
  lock_init(&countlock);
  for(int i = 0; i < NLOCKS; i++)
    lock_init(&locks[i]);
  
  for(int i = 0; i < NTHREADS; i++) {
    args[i].id = i;
    args[i].incs = NINCREMENTS;
    args[i].lk = &countlock;
    
    int pat = i % 4;
    int tid = thread_create(worker, &args[i], (void*)pat);
    
    if(tid < 0) {
      printf(1, "thread_create failed (%d)\n", i);
      exit();
    }
    
    tids[i] = tid;
    printf(1, "Created thread %d (tid=%d, pattern=%d)\n", i, tid, pat);
  }
  
  for(int i = 0; i < NTHREADS; i++) {
    if(thread_join(tids[i]) < 0)
      printf(1, "thread_join failed (%d)\n", i);
    else
      printf(1, "Joined thread %d\n", i);
  }
  
  printf(1, "All threads joined\n");
  printf(1, "Final counter: %d\n", counter);
  
  int expected = NTHREADS * NINCREMENTS;
  printf(1, "Expected: %d\n", expected);
  
  if(counter == expected)
    printf(1, "TEST PASSED: Counter OK\n");
  else
    printf(1, "TEST FAILED: Counter mismatch\n");
  
  int total = 0;
  for(int i = 0; i < NTHREADS; i++) {
    printf(1, "Thread %d count: %d\n", i, thread_counts[i]);
    total += thread_counts[i];
  }
  
  printf(1, "Per-thread total: %d\n", total);
  if(total == expected)
    printf(1, "TEST PASSED: Thread counts OK\n");
  else
    printf(1, "TEST FAILED: Thread counts mismatch\n");
  
  exit();
}
