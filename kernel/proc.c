struct proc {
  uint sz;                    
  pde_t* pgdir;               
  char *kstack;               
  enum procstate state;        
  int pid;                     
  struct proc *parent;         
  struct trapframe *tf;        
  struct context *context;     
  void *chan;                  
  int killed;                  
  struct file *ofile[NOFILE];  
  struct inode *cwd;           
  char name[16];              
  void *ustack;               
};
