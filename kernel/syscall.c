extern int sys_clone(void);
extern int sys_join(void);

static int (*syscalls[])(void) = {
  [SYS_fork]    sys_fork,
  [SYS_exit]    sys_exit,
  [SYS_wait]    sys_wait,
  [SYS_pipe]    sys_pipe,
  // ...other entries
  [SYS_clone]   sys_clone,
  [SYS_join]    sys_join,
};
