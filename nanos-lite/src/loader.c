#include "common.h"
#include "fs.h"
#define DEFAULT_ENTRY ((void *)0x4000000)

extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern size_t get_ramdisk_size();

uintptr_t loader(_Protect *as, const char *filename) {
  int file = fs_open(filename, 0, 0);
  fs_read(file, DEFAULT_ENTRY, fs_filesz(file));
  fs_close(file);

  return (uintptr_t)DEFAULT_ENTRY;
}
