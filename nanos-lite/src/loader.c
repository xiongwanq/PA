#include "common.h"
#include "fs.h"
#include "memory.h"

#define DEFAULT_ENTRY ((void *)0x8048000)

extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern size_t get_ramdisk_size();

uintptr_t loader(_Protect *as, const char *filename) {
  int file = fs_open(filename, 0, 0);
  size_t fileSize = fs_filesz(file); 
//  fs_read(file, DEFAULT_ENTRY, fs_filesz(file));
  void *va = DEFAULT_ENTRY;
  void *pa;
  int pageNum = fileSize / PGSIZE + 1;

  for(int i = 0; i < pageNum; i ++){
    pa = new_page();//空闲物理页
//    Log("Map va to pa: 0x%08x to 0x%08x", va, pa);
    _map(as, va, pa);
    fs_read(file, pa, PGSIZE);
    va += PGSIZE;
  }

  fs_close(file);

  return (uintptr_t)DEFAULT_ENTRY;
}
