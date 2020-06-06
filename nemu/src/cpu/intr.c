#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */
  vaddr_t gate_addr = cpu.idtr.base + NO * 8;

  if(NO * 8 > cpu.idtr.limit){
    assert(0);
  }

  rtl_push(&cpu.eflags.value);
  rtl_push(&cpu.cs);
  rtl_push(&ret_addr);
  
  uint32_t low, high, offset;
  low = vaddr_read(gate_addr, 4) & 0x0000ffff;
  high = vaddr_read(gate_addr + 2, 4) & 0xffff0000;
  offset = low | high;
  decoding.is_jmp = true;
  decoding.jmp_eip = offset;

}

void dev_raise_intr() {
}
