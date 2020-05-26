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
  TODO();

  print_asm("pusha");
}

make_EHelper(popa) {
  TODO();

  print_asm("popa");
}

make_EHelper(leave) {

  rtl_mv(&cpu.esp, &cpu.ebp);
  rtl_pop(&cpu.ebp);

  print_asm("leave");
}

make_EHelper(cltd) {
  rtlreg_t t;
  rtlreg_t t_val;
  if (decoding.is_operand_size_16) {
	rtl_lr_w(&t, R_AX);
//  printf("R_AX=0x%x\n",t);
	if(t < 0){
	  t_val = 0xffff;
	  rtl_sr_w(R_DX, &t_val);
	}
	else{
	  t_val = 0;
	  rtl_sr_w(R_DX, &t_val);
	}
  }
  else {
	rtl_lr_l(&t, R_EAX);
//    printf("R_EAX=0x%x\n",t);
    if(t < 0){
	  t_val = 0xffffffff;
	  rtl_sr_l(R_EDX, &t_val);
	}
	else{
//	  printf("----before-----\n");
//	  printf("R_EDX=0x%x\n",reg_l(R_EDX));
	  t_val = 0;
	  rtl_sr_l(R_EDX, &t_val);
//	  printf("----after-----\n");
//	  printf("t_val=0x%x,R_EDX=0x%x\n",t_val,reg_l(R_EDX));
	}
  }

  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  if (decoding.is_operand_size_16) {
    TODO();
  }
  else {
    TODO();
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
