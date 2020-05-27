#include "cpu/exec.h"

make_EHelper(test) {
  rtlreg_t t;
  rtl_and(&t, &id_dest->val, &id_src->val);
  
  rtl_update_ZFSF(&t, id_dest->width);
  t = 0;
  rtl_set_OF(&t);
  rtl_set_CF(&t);

  print_asm_template2(test);
}

make_EHelper(and) {
  rtlreg_t t;
  rtl_and(&t, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t);

  rtl_update_ZFSF(&t, id_dest->width);  
  t = 0;
  rtl_set_OF(&t);
  rtl_set_CF(&t);

  print_asm_template2(and);
}

make_EHelper(xor) {
  rtlreg_t t;
  rtl_xor(&t, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t);

  rtl_update_ZFSF(&t, id_dest->width);  
  t = 0;
  rtl_set_OF(&t);
  rtl_set_CF(&t);

  print_asm_template2(xor);
}

make_EHelper(or) {
  rtlreg_t t;
  rtl_or(&t, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t);

  rtl_update_ZFSF(&t, id_dest->width);  
  t = 0;
  rtl_set_OF(&t);
  rtl_set_CF(&t);

  print_asm_template2(and);

  print_asm_template2(or);
}

make_EHelper(sar) {
  // unnecessary to update CF and OF in NEMU
  rtlreg_t t;
  rtl_sar(&t, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t);

  rtl_update_ZFSF(&t, id_dest->width);

  print_asm_template2(sar);
}

make_EHelper(shl) {
  // unnecessary to update CF and OF in NEMU
  rtlreg_t t;
  rtl_shl(&t, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t);

  rtl_update_ZFSF(&t, id_dest->width);

  print_asm_template2(shl);
}

make_EHelper(shr) {
  // unnecessary to update CF and OF in NEMU
  rtlreg_t t;
  rtl_shr(&t, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t);

  rtl_update_ZFSF(&t, id_dest->width);


  print_asm_template2(shr);
}

make_EHelper(setcc) {
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(subcode), id_dest->str);
}

make_EHelper(not) {
  rtlreg_t t;
  rtl_mv(&t, &id_dest->val);
  rtl_not(&t);
  operand_write(id_dest, &t);

  print_asm_template1(not);
}

make_EHelper(rol) {
  rtlreg_t t = id_src->val;
  rtlreg_t tmpcf = 0;
  while (t != 0)
  {
    rtl_shri(&tmpcf, &id_dest->val, id_dest->width * 8 - 1);
    rtl_shli(&t0, &id_dest->val, 1);
    rtl_add(&t0, &t0, &tmpcf);
    t--;
  }
  operand_write(id_dest, &t0);

  rtl_set_CF(&tmpcf);
   
  print_asm_template2(rol);
}
