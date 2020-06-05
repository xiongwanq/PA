#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) {
  rtl_push(&id_dest -> val);
  print_asm_template1(push);
}

make_EHelper(pop) {
  rtlreg_t t;
  rtl_pop(&t);
  operand_write(id_dest, &t);

  print_asm_template1(pop);
}

make_EHelper(pusha) {
  rtlreg_t temp = cpu.esp;
  rtl_push(&cpu.eax);
  rtl_push(&cpu.ecx);
  rtl_push(&cpu.edx);
  rtl_push(&cpu.ebx);
  rtl_push(&temp);
  rtl_push(&cpu.ebp);
  rtl_push(&cpu.esi);
  rtl_push(&cpu.edi);

  print_asm("pusha");
}

make_EHelper(popa) {
  rtl_push(&cpu.edi);
  rtl_push(&cpu.esi);
  rtl_push(&cpu.ebp);
  rtl_push(NULL);
  rtl_push(&cpu.ebx);
  rtl_push(&cpu.edx);
  rtl_push(&cpu.ecx);
  rtl_push(&cpu.eax);

  print_asm("popa");
}

make_EHelper(leave) {

  rtl_mv(&cpu.esp, &cpu.ebp);
  rtl_pop(&cpu.ebp);

  print_asm("leave");
}

make_EHelper(cltd) {
  rtlreg_t t;
  rtlreg_t t_flag;
  rtlreg_t t_val;
  if (decoding.is_operand_size_16) {
	rtl_lr(&t, R_AX, 2);
	rtl_msb(&t_flag, &t, 2);
	if(t_flag == 0x1){
	  t_val = 0xffff;
	  rtl_sr(R_DX, 2, &t_val);
	}
	else{
	  t_val = 0;
	  rtl_sr(R_DX, 2, &t_val);
	}
  }
  else {
	rtl_lr(&t, R_EAX, 4);
    rtl_msb(&t_flag, &t, 4);

    if(t_flag == 0x1){
	  t_val = 0xffffffff;
	  rtl_sr(R_EDX, 4, &t_val);
	}
	else{
	  t_val = 0;
	  rtl_sr(R_EDX, 4, &t_val);
	}
  }

  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  rtlreg_t t;
  if (decoding.is_operand_size_16) {
	rtl_lr(&t, R_AX, 1);
	rtl_sext(&t, &t, 1);
	rtl_sr(R_AX, 2, &t);
  }
  else {
	rtl_lr(&t, R_EAX, 2);
	rtl_sext(&t, &t, 2);
	rtl_sr(R_EAX, 4, &t);
  }

  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_sext(&t2, &id_src->val, id_src->width);
  operand_write(id_dest, &t2);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  rtl_li(&t2, id_src->addr);
  operand_write(id_dest, &t2);
  print_asm_template2(lea);
}
