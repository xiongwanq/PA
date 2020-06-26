//#include "cpu/exec.h"
//#include "memory/mmu.h"
//
//void raise_intr(uint8_t NO, vaddr_t ret_addr) {
//  /* TODO: Trigger an interrupt/exception with ``NO''.
//   * That is, use ``NO'' to index the IDT.
//   */
//  vaddr_t gate_addr = cpu.idtr.base + NO * 8;
//
//  if(NO * 8 > cpu.idtr.limit){
//    assert(0);
//  }
//
//  rtl_push(&cpu.eflags.value);
//  rtl_push(&cpu.cs);
//  rtl_push(&ret_addr);
//  
//  uint32_t low, high, offset;
//  low = vaddr_read(gate_addr, 4) & 0x0000ffff;
//  high = vaddr_read(gate_addr + 4, 4) & 0xffff0000;
//  offset = low | high;
//  decoding.is_jmp = true;
//  decoding.jmp_eip = offset;
//
//}
//
//void dev_raise_intr() {
//}
#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */

  rtl_push(&cpu.eflags.value);//eflags的值压入栈
  rtl_push(&cpu.cs);//cs值压入栈
  rtl_push(&ret_addr);//保存当前地址
  if(NO > cpu.idtr.limit)//NO大于idtr寄存器的上限则终止程序
    assert(0);

  vaddr_t goal_addr = cpu.idtr.base + 8*NO;//NO索引到门描述符的offset域
  uint32_t bottom,top;
  bottom = vaddr_read(goal_addr,4)&0x0000ffff;//取低四位
  top = vaddr_read(goal_addr+4,4)&0xffff0000;//取高四位
  decoding.jmp_eip = bottom|top;//依次取低四位高四位之后，按位进行或运算，组成目标地址
  decoding.is_jmp = 1;
}

void dev_raise_intr() {
}
