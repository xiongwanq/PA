#include "nemu.h"
#include "device/mmio.h"
#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
  int mmio_id = is_mmio(addr);
  if(mmio_id != -1){
    return mmio_read(addr, len, mmio_id);
  }
  else{
    return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
  }
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  int mmio_id = is_mmio(addr);
  if(mmio_id != -1){
    return mmio_write(addr, len, data, mmio_id);
  }
  else{
    memcpy(guest_to_host(addr), &data, len);
  }
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  if(cpu.cr0.paging) {
    if (((addr & 0xfff) + len) > 0x1000) {
		int firSize,secSize;
        firSize = 0x1000 - (addr & 0xfff);
        secSize = len - firSize; 

        uint32_t firAddr = page_translate(addr, false);
        uint32_t firMem = paddr_read(firAddr, firSize);

        uint32_t secAddr = page_translate(addr + firSize, false);
        uint32_t secMem = paddr_read(secAddr, secSize);
        
        return firMem + (secMem << (firSize << 3));
    }
    else {
        paddr_t paddr = page_translate(addr, false);
        return paddr_read(paddr, len);
    }
  }
  else
    return paddr_read(addr, len);
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  if(cpu.cr0.paging) {
    if (((addr & 0xfff) + len) > 0x1000) {
		int firSize,secSize;
        firSize = 0x1000 - (addr & 0xfff);
        secSize = len - firSize; 

        uint32_t firAddr = page_translate(addr, true);
        paddr_write(firAddr, firSize, data);

        uint32_t highData = data >> (firSize << 3);
        uint32_t secAddr = page_translate(addr + firSize, true);
        paddr_write(secAddr, secSize, highData);
    }
    else {
        paddr_t paddr = page_translate(addr, true);
        paddr_write(paddr, len, data);
    }
  }
  else
    return paddr_write(addr, len, data);
}

paddr_t page_translate(vaddr_t vaddr, bool writing){
  PDE pde;
  PTE pte;

  uint32_t DIR = vaddr >> 22;
  uint32_t PAGE = (vaddr >> 12) & 0x3ff;
  uint32_t OFFSET = vaddr & 0xfff;
//  Log("DIR=0x%x,PAGE=0x%x,OFFSET=0x%x\n",DIR,PAGE,OFFSET);

  uint32_t pdaddr = (cpu.cr3.page_directory_base << 12) + (DIR << 2);
//  Log("pdaddr=0x%x\n",pdaddr);
  pde.val = paddr_read(pdaddr, 4);
//  Log("pde.val=0x%x\n",pde.val);
  assert(pde.present);

  uint32_t ptaddr = (pde.val & 0xfffff000) + (PAGE << 2);
//  Log("ptaddr=0x%x\n",ptaddr);
  pte.val = paddr_read(ptaddr, 4);
//  Log("pte.val=0x%x\n",pte.val);
  assert(pte.present);

  uint32_t paddr = (pte.val & 0xfffff000) + OFFSET;
//  Log("paddr=0x%x\n",paddr);

  if(pde.accessed == 0){
    pde.accessed = 1;
    paddr_write(pdaddr, 4, pde.val);
  }
  if(pte.accessed == 0 || ((pte.dirty == 0) && writing)){
    pte.accessed = 1;
    pte.dirty = 1;
  }
  paddr_write(ptaddr, 4, pte.val);
  return paddr;
}
