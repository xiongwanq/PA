#include "nemu.h"
#include "monitor/monitor.h"
#include <unistd.h>
#include <sys/prctl.h>
#include <signal.h>

#include "protocol.h"
#include <stdlib.h>

bool gdb_connect_qemu(void);
bool gdb_memcpy_to_qemu(uint32_t, void *, int);
bool gdb_getregs(union gdb_regs *);
bool gdb_setregs(union gdb_regs *);
bool gdb_si(void);
void gdb_exit(void);

static bool is_skip_qemu;
static bool is_skip_nemu;

void diff_test_skip_qemu() { is_skip_qemu = true; }
void diff_test_skip_nemu() { is_skip_nemu = true; }

#define regcpy_from_nemu(regs) \
  do { \
    regs.eax = cpu.eax; \
    regs.ecx = cpu.ecx; \
    regs.edx = cpu.edx; \
    regs.ebx = cpu.ebx; \
    regs.esp = cpu.esp; \
    regs.ebp = cpu.ebp; \
    regs.esi = cpu.esi; \
    regs.edi = cpu.edi; \
    regs.eip = cpu.eip; \
  } while (0)

#define diff_state(regName) \
  static bool concat(test_,regName) (union gdb_regs r){ \
    if(r.regName != cpu.regName) {  \
	  printf("r.eax: %-10xcpu.eax: %-10x\n",r.eax,cpu.eax);  \
      printf("r.ecx: %-10xcpu.ecx: %-10x\n",r.ecx,cpu.ecx);  \
      printf("r.edx: %-10xcpu.edx: %-10x\n",r.edx,cpu.edx);  \
      printf("r.ebx: %-10xcpu.ebx: %-10x\n",r.ebx,cpu.ebx);  \
      printf("r.esp: %-10xcpu.esp: %-10x\n",r.esp,cpu.esp);  \
      printf("r.ebp: %-10xcpu.ebp: %-10x\n",r.ebp,cpu.ebp);  \
      printf("r.esi: %-10xcpu.esi: %-10x\n",r.esi,cpu.esi);  \
      printf("r.edi: %-10xcpu.edi: %-10x\n",r.edi,cpu.edi);  \
      printf("r.eip: %-10xcpu.eip: %-10x\n",r.eip,cpu.eip);  \
      return true; \
    } \
    else   return false; \
  } 
 

diff_state(eax);
diff_state(ebx);
diff_state(ecx);
diff_state(edx);
diff_state(esp);
diff_state(ebp);
diff_state(esi);
diff_state(edi);

static uint8_t mbr[] = {
  // start16:
  0xfa,                           // cli
  0x31, 0xc0,                     // xorw   %ax,%ax
  0x8e, 0xd8,                     // movw   %ax,%ds
  0x8e, 0xc0,                     // movw   %ax,%es
  0x8e, 0xd0,                     // movw   %ax,%ss
  0x0f, 0x01, 0x16, 0x44, 0x7c,   // lgdt   gdtdesc
  0x0f, 0x20, 0xc0,               // movl   %cr0,%eax
  0x66, 0x83, 0xc8, 0x01,         // orl    $CR0_PE,%eax
  0x0f, 0x22, 0xc0,               // movl   %eax,%cr0
  0xea, 0x1d, 0x7c, 0x08, 0x00,   // ljmp   $GDT_ENTRY(1),$start32

  // start32:
  0x66, 0xb8, 0x10, 0x00,         // movw   $0x10,%ax
  0x8e, 0xd8,                     // movw   %ax, %ds
  0x8e, 0xc0,                     // movw   %ax, %es
  0x8e, 0xd0,                     // movw   %ax, %ss
  0xeb, 0xfe,                     // jmp    7c27
  0x8d, 0x76, 0x00,               // lea    0x0(%esi),%esi

  // GDT
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0xff, 0x00, 0x00, 0x00, 0x9a, 0xcf, 0x00,
  0xff, 0xff, 0x00, 0x00, 0x00, 0x92, 0xcf, 0x00,

  // GDT descriptor
  0x17, 0x00, 0x2c, 0x7c, 0x00, 0x00
};

void init_difftest(void) {
  int ppid_before_fork = getpid();
  int pid = fork();
  if (pid == -1) {
    perror("fork");
    panic("fork error");
  }
  else if (pid == 0) {
    // child

    // install a parent death signal in the chlid
    int r = prctl(PR_SET_PDEATHSIG, SIGTERM);
    if (r == -1) {
      perror("prctl error");
      panic("prctl");
    }

    if (getppid() != ppid_before_fork) {
      panic("parent has died!");
    }

    close(STDIN_FILENO);
    execlp("qemu-system-i386", "qemu-system-i386", "-S", "-s", "-nographic", NULL);
    perror("exec");
    panic("exec error");
  }
  else {
    // father

    gdb_connect_qemu();
    Log("Connect to QEMU successfully");

    atexit(gdb_exit);

    // put the MBR code to QEMU to enable protected mode
    bool ok = gdb_memcpy_to_qemu(0x7c00, mbr, sizeof(mbr));
    assert(ok == 1);

    union gdb_regs r;
    gdb_getregs(&r);

    // set cs:eip to 0000:7c00
    r.eip = 0x7c00;
    r.cs = 0x0000;
    ok = gdb_setregs(&r);
    assert(ok == 1);

    // execute enough instructions to enter protected mode
    int i;
    for (i = 0; i < 20; i ++) {
      gdb_si();
    }
  }
}

void init_qemu_reg() {
  union gdb_regs r;
  gdb_getregs(&r);
  regcpy_from_nemu(r);
  bool ok = gdb_setregs(&r);
  assert(ok == 1);
}

void difftest_step(uint32_t eip) {
  union gdb_regs r;
  bool diff = false;

  if (is_skip_nemu) {
    is_skip_nemu = false;
    return;
  }

  if (is_skip_qemu) {
    // to skip the checking of an instruction, just copy the reg state to qemu
    gdb_getregs(&r);
    regcpy_from_nemu(r);
    gdb_setregs(&r);
    is_skip_qemu = false;
    return;
  }

  gdb_si();
  gdb_getregs(&r);

  // TODO: Check the registers state with QEMU.
  // Set `diff` as `true` if they are not the same.

  diff=test_eax(r) || test_ebx(r) || test_ecx(r) || test_edx(r) ||
       test_ebp(r) || test_edi(r) || test_esi(r) || test_esp(r) ;
//  if(r.eflags != cpu.eflags.value){
//	diff = true;
//	printf("r.eflags=0x%x,cpu.eflags.value=0x%x\n",r.eflags,cpu.eflags.value);
//  }
//  if(r.eflags.CF != cpu.eflags.CF || r.eflags.ZF != cpu.eflags.ZF || r.eflags.SF != cpu.eflags.SF ||
//     r.eflags.IF != cpu.eflags.IF || r.eflags.OF != cpu.eflags.OF){
//	printf("r.eflags.CF=0x%x,cpu.eflags.CF=0x%x\n",r.eflags.CF,cpu.eflags.CF);
//	printf("r.eflags.ZF=0x%x,cpu.eflags.ZF=0x%x\n",r.eflags.ZF,cpu.eflags.ZF);
//	printf("r.eflags.SF=0x%x,cpu.eflags.SF=0x%x\n",r.eflags.SF,cpu.eflags.SF);
//	printf("r.eflags.IF=0x%x,cpu.eflags.IF=0x%x\n",r.eflags.IF,cpu.eflags.IF);
//	printf("r.eflags.OF=0x%x,cpu.eflags.OF=0x%x\n",r.eflags.OF,cpu.eflags.OF);
//  diff = true;
//  }

 
  if (diff) {
    nemu_state = NEMU_END;
  }
}
