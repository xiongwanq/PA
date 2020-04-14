#include "nemu.h"
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ = 1, TK_UEQ = -1, TK_HEX = 16, TK_DEC = 10, TK_REG = 9, TK_POINT = 257, TK_NEG = -2
  
  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {"0x[0-9a-fA-F]+", TK_HEX},					  // hexadecimal
  {"[0-9]+", TK_DEC},							  // decimal
  {"\\$(eax|ecx|edx|ebx|esp|ebp|esi|edi|eip)", TK_REG},   // register
  {"\\(", '('},			// '('
  {"\\)", ')'},			// ')'
  {"\\+", '+'},			// add 
  {"\\-", '-'},			// subtraction
  {"\\*", '*'},			// multiply
  {"/", '/'},			// division
  {"==", TK_EQ},        // equal
  {"!=", TK_UEQ},		// unequal
  {"&&", '&'},			// and
  {"\\|\\|", '|'},		// or
  {"!", '!'},			// not
  {" +", TK_NOTYPE},    // spaces
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
		  case TK_HEX:
		  case TK_DEC:
		  case TK_REG:
		  case '(':
          case ')': 
		  case '-':
		  case '*':
		  case '/':
		  case '+':
		  case TK_UEQ:
		  case TK_EQ:
		  case '&':
		  case '|':
		  case '!':
			tokens[nr_token].type = rules[i].token_type;
			memset(tokens[nr_token].str, 0, sizeof(tokens[nr_token].str));
			strncpy(tokens[nr_token].str, substr_start, substr_len);
			printf("token[%d]:\ttype:%d,",nr_token,tokens[nr_token].type);
			printf("str:%s\n",tokens[nr_token].str);
			nr_token ++;
			break;
		  default: break;
        }
		break;
      }
    }
    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool check_parentheses(int p,int q){
  if(tokens[p].type != '(' && tokens[q].type != ')'){
	return false;
  }

  int num_left = 0;

  for(int i = p + 1;i < q ;i++){
	  if(tokens[i].type == '('){
	    num_left ++;
      }else if(tokens[i].type == ')'){
		if(num_left != 0){
			num_left --;
	    }else{
			return false;
	    }
	  }
  }

  if(num_left == 0){
	return true;
  }else{
	return false;
  } 
}

uint32_t level(int token_type){
  if(token_type == TK_NEG||token_type == TK_POINT||token_type == '!'){
    return 1;
  }
  else if(token_type == '*'||token_type == '/'){
    return 2;
  }
  else if(token_type == '+'||token_type == '-'){
    return 3;
  }
  else if(token_type == TK_EQ||token_type == TK_UEQ){
    return 4;
  }
  else if(token_type == '&'){
    return 5;
  }
  else if(token_type == '|'){
    return 6;
  }
  else{  
    return 0;   //number's level
  }
}

uint32_t find_dominated_op(int p,int q){
  int num_left = 0;
  uint32_t dominate = p;
  uint32_t dominate_level = level(tokens[p].type);
  uint32_t index_level;

  for(int i=p;i<=q;i++){
    if(tokens[i].type == '('){
      num_left ++;
    }
    if(tokens[i].type == ')'){
      num_left --;
    }
    if(num_left == 0){
      index_level = level(tokens[i].type);
	  printf("str=%s,index_level=%d,dominate_level=%d\n",tokens[i].str,index_level,dominate_level);
      if(index_level >= dominate_level){
         dominate = i;
         dominate_level = index_level;
      }
    }
//	printf("p=%d,i=%d,q=%d\n",p,i,q);
//	printf("i=%d,dominate=%d,dominate_level=%d\n",i,dominate,dominate_level);
  }
  printf("dominate:%d",dominate);
  return dominate;
}

uint32_t eval(int p, int q) {
  if (p > q) {
    /* Bad expression */
	printf("Bad expression");
	return -1;
	}
  else if (p == q) {
    /* Single token.
    * For now this token should be a number.
    * Return the value of the number.
    */
	printf("single token!\n");
	printf("p=%d,q=%d\n",p,q);
	printf("type:%s\n",tokens[p].str);
	uint32_t num;
	if(tokens[p].type == TK_HEX){
		printf("hex!");
		sscanf(tokens[p].str,"%x",&num);
		printf("num=%d\n",num);
		return num;
	}
    else if(tokens[p].type == TK_DEC){
		printf("dec!");
		sscanf(tokens[p].str,"%d",&num);
		printf("num=%x\n",num);
		return num;
	}
    else if(tokens[p].type == TK_REG){
		printf("reg!\n");
		for(int i=0;i<4;i++){
			tokens[p].str[i] = tokens[p].str[i+1];
		}
		printf("reg_name=%s\n",tokens[p].str);
		if(strcmp(tokens[p].str, "eip") == 0){
			printf("cpu.eip=%d\n",cpu.eip);
			return cpu.eip;
		}
		for(int i = 0;i < 8;i ++ ){
//			char reg[4]="$";
//			strcat(reg,regsl[i]);
			if(strcmp(tokens[p].str,regsl[i]) == 0){
				printf("reg_l(%d)=%d\n",i,reg_l(i));
				return reg_l(i);
			}
		}
		return -1;
	}
    else {printf("234!");return -1;}
  }
  else if (check_parentheses(p, q) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
    * If that is the case, just throw away the parentheses.
    */
    return eval(p + 1, q - 1);
  }
  else {
    /* We should do more things here. */
	uint32_t op = find_dominated_op(p, q);
	printf(",op_str=%s,op_id=%d\n",tokens[op].str,op);
	if(tokens[op].type == TK_NEG|| tokens[op].type == TK_POINT|| tokens[op].type == '!'){
	  uint32_t val = eval(p+1, q);
	  switch (tokens[op].type){
	    case TK_NEG: return -val;
	    case TK_POINT: printf("TK_POINT:");return vaddr_read(val, 4);
	    case '!': return !val;
		default: assert(0);
	  }
    }
	else{
	  uint32_t val1 = eval(p, op - 1);
	  uint32_t val2 = eval(op + 1, q);
	  printf("val1=%d,val2=%d\n",val1,val2);
	  switch (tokens[op].type){
		case '+': return val1 + val2;
		case '-': return val1 - val2;
		case '*': return val1 * val2;
		case '/': 
			if(val2 == 0){
				printf("cannot devide 0!");
				assert(0);
			}
			else
				return val1 / val2;
		case TK_EQ: return val1 == val2;
		case TK_UEQ: return val1 != val2;
		case '&': return val1 && val2;
		case '|': return val1 || val2;
		default: assert(0);
      }
    }
  }
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  //TODO();
  *success = true;
  for(int i = 0;i < nr_token;i ++){
	if(tokens[i].type == '*' && (i == 0 || (tokens[i - 1].type != TK_HEX && tokens[i - 1].type != TK_DEC && tokens[i - 1].type != TK_REG && tokens[i - 1].type !=')')) ) {
        tokens[i].type = TK_POINT;
	}
	if(tokens[i].type == '-' && (i == 0 || (tokens[i - 1].type != TK_HEX && tokens[i - 1].type != TK_DEC && tokens[i - 1].type != TK_REG && tokens[i - 1].type !=')')) ) {
		tokens[i].type = TK_NEG;
	}
  }
  uint32_t p = 0,q = nr_token - 1;
  printf("p=%d,q=%d\n",p,q);
  return eval(p,q);
}

