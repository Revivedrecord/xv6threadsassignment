int
sys_clone(void)
{
  void (*fcn)(void *, void *);
  void *arg1, *arg2, *stack;
  
  if(argptr(0, (void*)&fcn, sizeof(fcn)) < 0 ||
     argptr(1, (void*)&arg1, sizeof(arg1)) < 0 ||
     argptr(2, (void*)&arg2, sizeof(arg2)) < 0 ||
     argptr(3, (void*)&stack, sizeof(stack)) < 0)
    return -1;
    
  return clone(fcn, arg1, arg2, stack);
}

int
sys_join(void)
{
  void **stack;
  
  if(argptr(0, (void*)&stack, sizeof(stack)) < 0)
    return -1;
  
  return join(stack);
}
