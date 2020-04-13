#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_si(char *args){
  //printf("args=%s\n",args);	
  char *arg = strtok(NULL, " ");
  int num;	//si[N]值
  if(arg==NULL){	
	num=1;
	}else{
	num = atoi(arg);
	}
  cpu_exec(num);
  return 0;
}	

static int cmd_q(char *args) {
  return -1;
}

static int cmd_info(char *args){
  char *arg = strtok(NULL, " ");  

  if (strcmp(arg,"r") == 0){
	//打印32位寄存器
	for(int i=0; i<8; i++){
	  printf("%s:\t0x%-.8x\t%d\n", regsl[i], cpu.gpr[i]._32, cpu.gpr[i]._32);
	}	
	//打印16位寄存器
    for(int i=0; i<8; i++){
      printf("%s:\t0x%-.8x\t%d\n", regsw[i], cpu.gpr[i]._16, cpu.gpr[i]._16);
    }
	//打印8位寄存器
	for(int i=0; i<8; i++){
	  int j;
	  if(i<=3){	//16位寄存器的第一个8位寄存器
		j=0;
	  }else{	//对应的第二个8位寄存器
		j=1;
	  }
      printf("%s:\t0x%-8.4x\t%d\n",regsb[i], cpu.gpr[i%4]._8[j], cpu.gpr[i%4]._8[j]);
	}
  }

  if(strcmp(arg,"w") == 0){
    list_watchpoint();
  }

  return 0;
}

static int cmd_x(char *args){
  char *step = strtok(NULL, " ");
  //读取次数
  vaddr_t stepNum = atoi(step);	
  
  //起始位置
  char *addr = strtok(NULL, " ");
  uint32_t addrStart;
  sscanf(addr,"%x",&addrStart);	
  
  //printf("stepNum=%d\taddrStart=%x\n",stepNum,addrStart);
  printf("%-8s\t%-8s ... %8s\n","Address","Dword block","Byte sequence"); 
  //循环使用 vaddr_read 函数来读取内存
  for (int i=stepNum; i>0; i--){
    uint32_t dBlock;
    dBlock = vaddr_read(addrStart,4);
    printf("0x%-8x\t",addrStart);
	printf("0x%-.8x  ... ",dBlock);
    
	//内循环，打印每字节
	uint32_t addrReStart = addrStart;
	for(int j=1; j<=4; j++){
	  uint32_t byteSeq;
	  byteSeq = vaddr_read(addrReStart,1);
	  printf("%.2x ",byteSeq);
	  addrReStart += 1;
	}
	printf("\n");	
	addrStart += 4;
  }
  return 0;
}

static int cmd_p(char *args){
//  init_regex();
  printf("%s\n",args);
  bool success;
  uint32_t value = expr(args,&success);
  if(success){
    printf("%d\n", value);
  }else{
    printf("no value\n");
  }
  return 0;
}

static int cmd_w(char *args){
  set_watchpoint(args);
  return 0;
}

static int cmd_d(char *args){
  int NO;
  sscanf(args,"%d",&NO); 
  delete_watchpoint(NO);
  return 0;
}


static int cmd_help(char *args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "[N]Stop the program after N steps, default value is 1", cmd_si },
  { "info", "[r]Print register status\t[w]Show all watchpoints", cmd_info },
  { "x", "Scan memory", cmd_x},
  { "p", "Evaluate expression", cmd_p},
  { "w", "Set watchpoints", cmd_w},
  { "d", "delete watchpoints", cmd_d} 
  /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
