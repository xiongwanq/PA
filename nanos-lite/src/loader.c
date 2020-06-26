#include "common.h"
#include "fs.h"
#include "memory.h"

#define DEFAULT_ENTRY ((void *)0x8048000)

extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern size_t get_ramdisk_size();
uintptr_t loader(_Protect *as, const char *filename) {
 /* size_t ramdisk_size = get_ramdisk_size();
  ramdisk_read(DEFAULT_ENTRY,0,ramdisk_size);*/
  int fd = fs_open(filename,0,0);//打开文件
  size_t f_size = fs_filesz(fd);
  //fs_read(fd,DEFAULT_ENTRY,f_size);//获取文件大小
  void *start = DEFAULT_ENTRY;//赋起始位置为初值
  void *destination;
  int pages = f_size / PGSIZE + 1;//获取页数
  for(int i = 0;i < pages;i++){
    destination = new_page();//获取空闲页
    //Log("Map va to pa :0x%08x to 0x%08x",start,destination);
    _map(as,start,destination);
    fs_read(fd,destination,PGSIZE);
    start+=PGSIZE;//更新 
  }
  fs_close(fd);
  return (uintptr_t)DEFAULT_ENTRY;
}

//uintptr_t loader(_Protect *as, const char *filename) {
//  int file = fs_open(filename, 0, 0);
//  size_t fileSize = fs_filesz(file); 
////  fs_read(file, DEFAULT_ENTRY, fs_filesz(file));
//  void *va = DEFAULT_ENTRY;
//  void *pa;
//  int pageNum = fileSize / PGSIZE + 1;
//
//  for(int i = 0; i < pageNum; i ++){
//    pa = new_page();//空闲物理页
////    Log("Map va to pa: 0x%08x to 0x%08x", va, pa);
//    _map(as, va, pa);
//    fs_read(file, pa, PGSIZE);
//    va += PGSIZE;
//  }
//
//  fs_close(file);
//
//  return (uintptr_t)DEFAULT_ENTRY;
//}
