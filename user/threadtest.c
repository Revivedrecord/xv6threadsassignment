#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define NTHREADS    4
#define NINCREMENTS 10000
#define NLOCKS      2

volatile int counter = 0;
volatile int thread_counts[NTHREADS] = {0};
lock_t counter_lock;
lock_t locks[NLOCKS];

struct thread_arg {
  int id;
  int incs;
  lock_t *lk;
};

void
worker(void *arg1, void *arg2)
{
  struct thread_arg *args = (struct thread_arg*)arg1;
  int pattern = (int)(uint64)arg2;
  int id = args->id;
  int n = args->incs;
  lock_t *lk = args->lk;
  
  printf("Thread %d: Starting (pattern %d)\n", id, pattern);
  
  switch(pattern) {
    case 0: // Coarse-grained locking
      for(int i = 0; i < n; i++) {
        lock_acquire(lk);
        counter++;
        thread_counts[id]++;
        lock_release(lk);
      }
      break;
      
    case 1: // Fine-grained locking
      for(int i = 0; i < n; i++) {
        lock_acquire(lk);
        counter++;
        lock_release(lk);
        
        thread_counts[id]++;
      }
      break;
      
    case 2: // Multiple lock acquisition
      for(int i = 0; i < n; i++) {
        lock_t *cur = &locks[i % NLOCKS];
        lock_acquire(cur);
        counter++;
        thread_counts[id]++;
        lock_release(cur);
      }
      break;
      
    case 3: // Memory allocation stress test
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
  
  printf("Thread %d: Done (%d incs)\n", id, thread_counts[id]);
  exit(0);
}

int
main(int argc, char *argv[])
{
  int tids[NTHREADS];
  struct thread_arg args[NTHREADS];
  
  printf("Test: %d threads, %d increments each\n",
         NTHREADS, NINCREMENTS);
  
  lock_init(&counter_lock);
  for(int i = 0; i < NLOCKS; i++)
    lock_init(&locks[i]);
  
  for(int i = 0; i < NTHREADS; i++) {
    args[i].id = i;
    args[i].incs = NINCREMENTS;
    args[i].lk = &counter_lock;
    
    int pattern = i % 4;
    int tid = thread_create(worker, &args[i], (void*)(uint64)pattern);
    
    if(tid < 0) {
      printf("thread_create failed (%d)\n", i);
      exit(1);
    }
    
    tids[i] = tid;
    printf("Created thread %d (tid=%d, pattern=%d)\n", i, tid, pattern);
  }
  
  for(int i = 0; i < NTHREADS; i++) {
    if(thread_join(tids[i]) < 0)
      printf("thread_join failed (%d)\n", i);
    else
      printf("Joined thread %d\n", i);
  }
  
  printf("All threads joined\n");
  printf("Final counter: %d\n", counter);
  
  int expected = NTHREADS * NINCREMENTS;
  printf("Expected: %d\n", expected);
  
  if(counter == expected)
    printf("TEST PASSED: Counter OK\n");
  else
    printf("TEST FAILED: Counter mismatch\n");
  
  int total = 0;
  for(int i = 0; i < NTHREADS; i++) {
    printf("Thread %d count: %d\n", i, thread_counts[i]);
    total += thread_counts[i];
  }
  
  printf("Per-thread total: %d\n", total);
  if(total == expected)
    printf("TEST PASSED: Thread counts OK\n");
  else
    printf("TEST FAILED: Thread counts mismatch\n");
  
  exit(0);
}
