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
  int mmio_id;
  mmio_id = is_mmio(addr);//传入物理地址
  if(mmio_id!=-1)//映射到I/O空间
    return mmio_read(addr,len,mmio_id);
  else
    return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  int mmio_id;
  mmio_id = is_mmio(addr);//传入物理地址
  if(mmio_id!=-1)//映射到I/O空间
    mmio_write(addr,len,data,mmio_id);
  else
    memcpy(guest_to_host(addr), &data, len);
}

paddr_t page_translate(vaddr_t vaddr){
  PDE pde;
  PTE pte;
  uint32_t DIR = (vaddr >> 22);//虚拟地址高十位隐含页目录字段
  //cr3得到页目录表基址  将低二十位移到高二十位 构成页目录地址的高二十位
  //DIR左移两位构成页目录项地址的低十二位 即乘以四 便于4字节读
  //最后从内存中读出页目录项
  //Log("vaddr:%#x",vaddr);
  //Log("CR3:%#x",cpu.cr3.page_directory_base);
  //Log("DIR:%#x",DIR);
  uint32_t pde_addr = (cpu.cr3.page_directory_base << 12)+(DIR << 2);
  pde.val = paddr_read(pde_addr,4);
  //Log("PDE_addr:%#x   PDE_val:%#x",pde_addr,pde.val);
  //检验有效位
  assert(pde.present);

  //取出vaddr的10~20位，乘4，与pde.val的高二十位结合，得到pte.val
  uint32_t PAGE = ((vaddr >>12)&0x3ff);
  uint32_t pte_addr = (pde.val & 0xfffff000)+(PAGE << 2);
  pte.val = paddr_read(pte_addr,4);
  //Log("PTE_addr:%#x   PTE_val:%#x",pte_addr,pte.val);
  //检验有效位
  assert(pte.present);

  //pte.val高二十位与vaddr低十二位结合得到物理地址
  uint32_t physical_addr = (pte.val & 0xfffff000) + (vaddr & 0xfff);
  //Log("physical addr:%#x",physical_addr);

  //pde的accessed位设为1，将pde.val写回pde_addr
  pde.accessed = 1;
  paddr_write(pde_addr,4,pde.val);
  //accessed位为0 或 pte的dirty位为0且writing为1
  //则将accessed和dirty位置为1
  if(pte.accessed == 0 || pte.dirty == 0){
    pte.accessed = 1;
    pte.dirty = 1;
  }

  //将pte.val写回pte_addr
  //返回物理地址
  paddr_write(pte_addr,4,pte.val);
  return physical_addr;
}


uint32_t vaddr_read(vaddr_t addr, int len) {
  if(cpu.cr0.paging){
    if((((addr << 20) >> 20) + len) > 0x1000){
      int size_1st,size_2nd;
      size_1st = 0x1000 - (addr & 0xfff);//第一个页面数据大小
      size_2nd = len -size_1st;//第二个页面数据大小
    
      //获取页面1地址及数据
      uint32_t page1_addr = page_translate(addr);
      uint32_t page1_mem = paddr_read(page1_addr,size_1st);
      //获取页面2地址及数据
      uint32_t page2_addr = page_translate(addr + size_1st);
      uint32_t page2_mem = paddr_read(page2_addr,size_2nd);

      return page1_mem + (page2_mem << (size_1st << 3));
    }
    else{
      paddr_t paddr = page_translate(addr);
      return paddr_read(paddr,len);
    }
  }
  else
    return paddr_read(addr, len);
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
if(cpu.cr0.paging){
    if((((addr << 20) >> 20) + len) > 0x1000){
      int size_1st,size_2nd;
      size_1st = 0x1000 - (addr & 0xfff);//第一个页面数据大小
      size_2nd = len -size_1st;//第二个页面数据大小

      //将地位的数据写入第一个页面
      uint32_t page1_addr = page_translate(addr);
      paddr_write(page1_addr,size_1st,data);

      //将高位的数据写入第二个页面
      uint32_t h_data = data >> (size_1st << 3);//data右移页面1大小的八倍
      uint32_t page2_addr = page_translate(addr + size_1st);
      paddr_write(page2_addr,size_2nd,h_data);
    }
    else{
      paddr_t paddr = page_translate(addr);
      paddr_write(paddr,len,data);
    }
  }
  else  paddr_write(addr, len, data);
}


