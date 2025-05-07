# XV6 Kernel Threads Implementation

This repository contains modified xv6 files that implement kernel threads with shared address space capabilities. The implementation includes:

- `clone()` and `join()` system calls
- Thread library with `thread_create()` and `thread_join()`
- Ticket locks with atomic operations
- A test program to verify functionality

## Files Modified

- `kernel/syscall.h` - Added system call numbers
- `kernel/syscall.c` - Updated syscall table
- `kernel/proc.h` - Added stack field to proc structure
- `kernel/proc.c` - Added clone() and join() implementations, modified wait(), exit(), and growproc()
- `kernel/sysproc.c` - Added syscall handlers
- `user/user.h` - Added declarations for thread functions and lock operations
- `user/ulib.c` - Implemented thread library functions
- `user/threadtest.c` - Added test program for threads
- `Makefile` - Added threadtest to build targets

## Installation

To apply these changes to a fresh xv6 installation:

1. Clone the standard xv6 repository:  https://github.com/mit-pdos/xv6-public
2. Replace the original files with the modified versions from this repository

