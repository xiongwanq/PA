#include "common.h"
#include "syscall.h"
#include "fs.h"

int mm_brk(uint32_t new_brk);

static inline uintptr_t sys_open(uintptr_t pathname, uintptr_t flags, uintptr_t mode) {
//  SYSCALL_ARG1(r) = fs_open((char *)pathname, flags, mode);
  return 1;
}

static inline uintptr_t sys_write(uintptr_t fd, uintptr_t buf, uintptr_t len) {
//  Log();
  return fs_write(fd, (void *)buf, len);
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

static inline uintptr_t sys_brk(_RegSet *r) {
//  Log();
  SYSCALL_ARG1(r) = 0;
  return 1;
}

static inline uintptr_t sys_none(_RegSet *r) {
  SYSCALL_ARG1(r) = 1;
  return 1;
}
  
static inline uintptr_t sys_exit(_RegSet *r) {
  _halt(SYSCALL_ARG2(r));
  return 1;
}

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  uintptr_t eax;
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG4(r);
  switch (a[0]) {
    case SYS_none:
	  sys_none(r);
	  break;
    case SYS_exit:
	  sys_exit(r);
	  break;
//	case SYS_open:
//	  sys_open(r);
//	  break;
	case SYS_write:
	  Log("use write\n");
	  eax = sys_write(SYSCALL_ARG2(r), SYSCALL_ARG3(r), SYSCALL_ARG4(r));
	  break;
	case SYS_brk:
	  Log("use brk\n");
//	  eax = sys_brk();
	  sys_brk(r);
	  break;
//	case SYS_read:
//	  sys_read(r):
//	  break;
//	case SYS_lseek:
//	  sys_lseek(r);
//	  break;
//	case SYS_close:
//	  sys_close(r);
//	  break;

    default: panic("Unhandled syscall ID = %d", a[0]);
  }
  SYSCALL_ARG1(r) = eax;
 
  return NULL;
}
