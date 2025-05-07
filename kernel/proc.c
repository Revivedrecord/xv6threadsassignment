struct spinlock growlock;

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
  initlock(&growlock, "addrspace_lock");
}

int
clone(void(*fcn)(void *, void *), void *arg1, void *arg2, void *stack)
{
  int pid;
  struct proc *np;
  
  if((uint)stack % PGSIZE != 0)
    return -1;
  
  if((np = allocproc()) == 0)
    return -1;
  
  np->pgdir = proc->pgdir;
  np->sz = proc->sz;
  np->parent = proc;
  *np->tf = *proc->tf;
  
  np->tf->eax = 0;
  
  uint sp = (uint)stack + PGSIZE;
  
  sp -= 4;
  *(uint*)sp = 0xffffffff;
  
  sp -= 4;
  *(uint*)sp = (uint)arg2;
  
  sp -= 4;
  *(uint*)sp = (uint)arg1;
  
  np->tf->esp = sp;
  np->tf->eip = (uint)fcn;
  np->ustack = stack;
  
  for(int i = 0; i < NOFILE; i++)
    if(proc->ofile[i])
      np->ofile[i] = filedup(proc->ofile[i]);
  np->cwd = idup(proc->cwd);
  
  safestrcpy(np->name, proc->name, sizeof(proc->name));
  np->state = RUNNABLE;
  
  pid = np->pid;
  return pid;
}

int
join(void **stack)
{
  struct proc *p;
  int havekids, pid;
  
  acquire(&ptable.lock);
  for(;;) {
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
      if(p->parent != proc || p->pgdir != proc->pgdir)
        continue;
      
      havekids = 1;
      if(p->state == ZOMBIE) {
        pid = p->pid;
        *stack = p->ustack;
        
        kfree(p->kstack);
        p->kstack = 0;
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->state = UNUSED;
        
        release(&ptable.lock);
        return pid;
      }
    }
    
    if(!havekids || proc->killed) {
      release(&ptable.lock);
      return -1;
    }
    
    sleep(proc, &ptable.lock);
  }
}
