#include "common.h"
#include "syscall.h"
#include "fs.h"

int mm_brk(uint32_t new_brk);

static inline uintptr_t sys_open(uintptr_t pathname, uintptr_t flags, uintptr_t mode) {
//  SYSCALL_ARG1(r) = fs_open((char *)pathname, flags, mode);
  return 1;
}

static inline uintptr_t sys_write(uintptr_t fd, uintptr_t buf, uintptr_t len) {
//  SYSCALL_ARG1(r) = fs_write(fd, (void *)buf, len);
  return 1;
}

static inline uintptr_t sys_read(uintptr_t fd, uintptr_t buf, uintptr_t len) {
//  SYSCALL_ARG1(r) = fs_read(fd, (void *)buf, len);
  return 1;
}

static inline uintptr_t sys_lseek(uintptr_t fd, uintptr_t offset, uintptr_t whence) {
  return fs_lseek(fd, offset, whence);
}

static inline uintptr_t sys_close(uintptr_t fd) {
//  SYSCALL_ARG1(r) = fs_close(fd);
  return 1;
}

static inline uintptr_t sys_brk(uintptr_t new_brk) {
//  SYSCALL_ARG1(r) = 0;
  return 1;
}

static inline uintptr_t sys_none(_RegSet *r) {
  SYSCALL_ARG1(r) = 1;
  return 1;
}
  
_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);

  switch (a[0]) {
    case SYS_none:
	  sys_none(r);
	  break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
