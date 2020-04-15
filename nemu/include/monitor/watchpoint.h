#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"


typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char expr[100];
  int new_val;
  int old_val;
  /* TODO: Add more members if necessary */

} WP;

void list_watchpoint();
WP* scan_watchpoint();
int set_watchpoint(char *e);
bool delete_watchpoint(int NO);


#endif
