#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp(){
  WP *free_p = free_;
  WP *head_p = head;
  if(free_p){
    free_ = free_->next;
	if(head_p){
		while(head_p->next){
		  head_p = head_p->next;
	    }
		head_p->next = free_p;
	}
	else{
		free_p->next = head;
		head = free_p;
	}
	return free_p;

//	if(head == NULL){
//	    ptr->next = head;
//	    head = ptr;
//	}
//	else{
//        ptr->next = head->next;
//		head->next = ptr;
//	}
//	printf("return ptr->NO:%d\n",ptr->NO);
//    return ptr;
  }else{
	assert(0);
  }
}

void free_wp(WP *wp){
  if(wp == head){
	head = head->next;
	wp->next = free_;
	free_ = wp;
  }else{
    WP *pre_wp = head;
    while(pre_wp->next != wp){
    	pre_wp = pre_wp->next;
    }
    pre_wp->next = wp->next;
    wp->next = free_;
    free_ = wp;
  }
}

int set_watchpoint(char *e){
  WP *wp = new_wp();
  strcpy(wp->expr,e);
  bool success;
  wp->old_val = expr(e,&success);
  if(success){
    printf("wp->NO=%d,wp->expr=%s,old_val=%#x\n",wp->NO,wp->expr,wp->old_val);
    return wp->NO;
  }else{
	printf("no value\n");
    return 0;
  }
}

bool delete_watchpoint(int NO){
  WP *wp = head;
  while(wp->NO != NO){
	wp = wp->next;
  }
  if(wp){
	free_wp(wp);
    return true;
  }else{
	printf("no such watchpoint");
	return false;
  }
}

void list_watchpoint(){
  WP *wp = head;
  printf("%-4s%-20s%-20s\n","NO","EXPR","Old Value");
  while(wp){
	printf("%-4d%-20s0x%-20x\n",wp->NO,wp->expr,wp->old_val);
	wp = wp->next;
  }
}

WP* scan_watchpoint(){
  
  return NULL;  
}
