int clone(void(*fcn)(void *, void *), void *arg1, void *arg2, void *stack);
int join(void **stack);

int thread_create(void (*start_routine)(void *, void *), void *arg1, void *arg2);
int thread_join(int tid);

typedef struct {
  volatile uint ticket;
  volatile uint turn;
} lock_t;

void lock_init(lock_t *lk);
void lock_acquire(lock_t *lk);
void lock_release(lock_t *lk);
