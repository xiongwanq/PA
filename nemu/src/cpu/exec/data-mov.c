//#include "cpu/exec.h"
//
//make_EHelper(mov) {
//  operand_write(id_dest, &id_src->val);
//  print_asm_template2(mov);
//}
//
//make_EHelper(push) {
//  rtl_push(&id_dest -> val);
//  print_asm_template1(push);
//}
//
//make_EHelper(pop) {
//  rtlreg_t t;
//  rtl_pop(&t);
//  operand_write(id_dest, &t);
//
//  print_asm_template1(pop);
//}
//
//make_EHelper(pusha) {
//  rtlreg_t temp = cpu.esp;
//  rtl_push(&cpu.eax);
//  rtl_push(&cpu.ecx);
//  rtl_push(&cpu.edx);
//  rtl_push(&cpu.ebx);
//  rtl_push(&temp);
//  rtl_push(&cpu.ebp);
//  rtl_push(&cpu.esi);
//  rtl_push(&cpu.edi);
//
//  print_asm("pusha");
//}
//
//make_EHelper(popa) {
//  rtl_pop(&cpu.edi);
//  rtl_pop(&cpu.esi);
//  rtl_pop(&cpu.ebp);
//  rtl_pop(&t1);
//  rtl_pop(&cpu.ebx);
//  rtl_pop(&cpu.edx);
//  rtl_pop(&cpu.ecx);
//  rtl_pop(&cpu.eax);
//
//  print_asm("popa");
//}
//
//make_EHelper(leave) {
//
//  rtl_mv(&cpu.esp, &cpu.ebp);
//  rtl_pop(&cpu.ebp);
//
//  print_asm("leave");
//}
//
//make_EHelper(cltd) {
//  rtlreg_t t;
//  rtlreg_t t_flag;
//  rtlreg_t t_val;
//  if (decoding.is_operand_size_16) {
//	rtl_lr(&t, R_AX, 2);
//	rtl_msb(&t_flag, &t, 2);
//	if(t_flag == 0x1){
//	  t_val = 0xffff;
//	  rtl_sr(R_DX, 2, &t_val);
//	}
//	else{
//	  t_val = 0;
//	  rtl_sr(R_DX, 2, &t_val);
//	}
//  }
//  else {
//	rtl_lr(&t, R_EAX, 4);
//    rtl_msb(&t_flag, &t, 4);
//
//    if(t_flag == 0x1){
//	  t_val = 0xffffffff;
//	  rtl_sr(R_EDX, 4, &t_val);
//	}
//	else{
//	  t_val = 0;
//	  rtl_sr(R_EDX, 4, &t_val);
//	}
//  }
//
//  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
//}
//
//make_EHelper(cwtl) {
//  rtlreg_t t;
//  if (decoding.is_operand_size_16) {
//	rtl_lr(&t, R_AX, 1);
//	rtl_sext(&t, &t, 1);
//	rtl_sr(R_AX, 2, &t);
//  }
//  else {
//	rtl_lr(&t, R_EAX, 2);
//	rtl_sext(&t, &t, 2);
//	rtl_sr(R_EAX, 4, &t);
//  }
//
//  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
//}
//
//make_EHelper(movsx) {
//  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
//  rtl_sext(&t2, &id_src->val, id_src->width);
//  operand_write(id_dest, &t2);
//  print_asm_template2(movsx);
//}
//
//make_EHelper(movzx) {
//  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
//  operand_write(id_dest, &id_src->val);
//  print_asm_template2(movzx);
//}
//
//make_EHelper(lea) {
//  rtl_li(&t2, id_src->addr);
//  operand_write(id_dest, &t2);
//  print_asm_template2(lea);
//}
#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) {
  rtl_push(&id_dest->val);
  print_asm_template1(push);
}

make_EHelper(pop) {
  rtl_pop(&t0);//寄存器出栈
  //更新操作数
  operand_write(id_dest,&t0);
  print_asm_template1(pop);
}

make_EHelper(pusha) {
  t1 = cpu.esp;//temp
  rtl_push(&cpu.eax);
  rtl_push(&cpu.ecx);
  rtl_push(&cpu.edx);
  rtl_push(&cpu.ebx);
  rtl_push(&t1);
  rtl_push(&cpu.ebp);
  rtl_push(&cpu.esi);
  rtl_push(&cpu.edi);

  print_asm("pusha");
}

make_EHelper(popa) {
  rtl_pop(&cpu.edi);
  rtl_pop(&cpu.esi);
  rtl_pop(&cpu.ebp);
  rtl_pop(&t1);
  rtl_pop(&cpu.ebx);
  rtl_pop(&cpu.edx);
  rtl_pop(&cpu.ecx);
  rtl_pop(&cpu.eax);

  print_asm("popa");
}

make_EHelper(leave) {
  rtl_mv(&cpu.esp,&cpu.ebp);
  rtl_pop(&cpu.ebp);
  print_asm("leave");
}

make_EHelper(cltd) {
 /* if (decoding.is_operand_size_16) {//16位为ax
    rtl_lr_w(&t0,R_AX);//读ax
    if((int32_t)(int16_t)(uint16_t)t0<0){//ax存32位无符号数，将其拓展为32位有符号数
      reg_w(R_DX) = 0Xffff;//dx = 0xffff
    }
    else{
      reg_w(R_DX) = 0;//dx = 0
    }
  }
  else {//32位为eax
    rtl_lr_l(&t0,R_EAX);//读eax
    if((int32_t)t0 < 0){//拓展为有符号数
      reg_l(R_EDX) = 0xffffffff;//edx = 0xffffffff
    }
    else{
      reg_l(R_EDX) = 0;//edx = 0
    }
  }*/
  if (decoding.is_operand_size_16) {
	rtl_lr_w(&t0, R_AX);
	rtl_sext(&t0, &t0, 2);
	rtl_sari(&t0, &t0, 16);
	rtl_sr_w(R_DX, &t0);
  }
  else{
	rtl_lr_l(&t0, R_EAX);
	rtl_sari(&t0, &t0, 31);
	rtl_sari(&t0, &t0, 1);
	rtl_sr_l(R_EDX, &t0);
  }
  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  if (decoding.is_operand_size_16) {//16位
    rtl_lr_b(&t0,R_AL);
    rtl_sext(&t0,&t0,1);//al的8-16位赋给ax
    //reg_w(R_AX) = t0;
    rtl_sr_w(R_AX,&t0);
  }
  else {//32位
    rtl_lr_w(&t0,R_AX);
    rtl_sext(&t0,&t0,2);//ax的16到32位赋给eax
    //reg_l(R_EAX) = t0;
    rtl_sr_l(R_EAX,&t0);
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

