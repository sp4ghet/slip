#include "lval.h"

char* ltype_name(int ltype){
  switch(ltype){
    case LVAL_BOOL: return "boolean";
    case LVAL_ERR: return "error";
    case LVAL_NUM: return "number";
    case LVAL_SYM: return "symbol";
    case LVAL_FUNC: return "function";
    case LVAL_SEXPR: return "s-expression";
    case LVAL_QEXPR: return "q-expression";
    default: return "unknown type";
  }
}

// lval constructors
// -----------------
lval* lval_num(long x){
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_NUM;
  v->num = x;
  return v;
}

lval* lval_err(char* fmt, ...){
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_ERR;

  va_list va;
  va_start(va, fmt);

  v->err = malloc(512);
  vsnprintf(v->err, 511, fmt, va);
  v->err = realloc(v->err, strlen(v->err)+1);

  va_end(va);
  return v;
}

lval* lval_sym(char* s){
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_SYM;
  v->symbol = malloc(strlen(s)+1);
  strcpy(v->symbol, s);
  return v;
}

lval* lval_sexpr(void){
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_SEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}

lval* lval_qexpr(void){
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_QEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}

lval* lval_func(lbuiltin func){
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_FUNC;
  v->builtin = func;
  return v;
}

lval* lval_bool(int b){
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_BOOL;
  v->truth = b ? 1 : 0;

  return v;
}


lval* lval_lambda(lval* formals, lval* body){
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_FUNC;

  v->builtin = NULL;
  v->func_scope = lenv_new();
  v->formals = formals;
  v->body = body;

  return v;
}

// delte, add, other operations to lval
void lval_del(lval* v){
  switch(v->type){
    case LVAL_BOOL:
    case LVAL_NUM: break;
    case LVAL_FUNC:
      if(v->builtin == NULL){
        lenv_del(v->func_scope);
        lval_del(v->formals);
        lval_del(v->body);
      }
      break;

    case LVAL_ERR: free(v->err); break;
    case LVAL_SYM: free(v->symbol); break;

    case LVAL_QEXPR:
    case LVAL_SEXPR:
      // free each cell from the cells array
      for(int i = 0; i < v->count; i++){
        lval_del(v->cell[i]);
      }
      // free the cells array
      free(v->cell);
    break;
  }
  // free the whole lvalue
  free(v);
}

lval* lval_add(lval* v, lval* next){
  v->count++;
  v->cell = realloc(v->cell, sizeof(lval*) * v->count);
  v->cell[v->count-1] = next;
  return v;
}


lval* lval_copy(lval* v){
  lval* x = malloc(sizeof(lval));
  x->type = v->type;

  switch(v->type){
    case LVAL_NUM: x->num = v->num; break;
    case LVAL_BOOL: x->truth = v->truth; break;
    case LVAL_FUNC:
      if(v->builtin != NULL){
        x->builtin = v->builtin;
      }else{
        x->builtin = NULL;
        x->func_scope = lenv_copy(v->func_scope);
        x->formals = lval_copy(v->formals);
        x->body = lval_copy(v->body);
      }
    break;

    case LVAL_ERR:
      x->err = malloc(strlen(v->err) + 1);
      strcpy(x->err, v->err);
      break;
    case LVAL_SYM:
      x->symbol = malloc(strlen(v->symbol) + 1);
      strcpy(x->symbol, v->symbol);
      break;
    case LVAL_SEXPR:
    case LVAL_QEXPR:
      x->count = v->count;
      x->cell = malloc(sizeof(lval*) * x->count);
      for(int i = 0; i < x->count; i++){
        x->cell[i] = lval_copy(v->cell[i]);
      }
      break;
  }

  return x;
}

lval* lval_pop(lval* v, int i){
  lval* x = v->cell[i];

  memmove(&v->cell[i], &v->cell[i+1], sizeof(lval*) * (v->count - i - 1));

  v->count--;
  v->cell = realloc(v->cell, sizeof(lval*) * v->count);
  return x;
}

lval* lval_take(lval* v, int i){
  lval* x = lval_pop(v, i);
  lval_del(v);
  return x;
}

lval* lval_join(lval* x, lval* y){

  while(y->count > 0){
    x = lval_add(x, lval_pop(y, 0));
  }

  lval_del(y);
  return x;
}

// print
void lval_expr_print(lval* v, char open, char close){
  putchar(open);
  for (int i = 0; i < v->count; i++){

    lval_print(v->cell[i]);

    if (i != (v->count-1)){
      putchar(' ');
    }
  }
  putchar(close);
}

void lval_print(lval* v){
  switch(v->type){
    case LVAL_BOOL:
      printf(v->truth ? "True" : "False"); break;
    case LVAL_NUM:
      printf("%li", v->num); break;
    case LVAL_ERR:
      printf("Error: %s", v->err); break;
    case LVAL_SYM:
      printf("%s", v->symbol); break;
    case LVAL_SEXPR: lval_expr_print(v, '(', ')'); break;
    case LVAL_QEXPR: lval_expr_print(v, '{', '}'); break;
    case LVAL_FUNC:
      if(v->builtin != NULL){
        printf("<function>");
      }else{
        printf("(\\ "); lval_print(v->formals);
        putchar(' '); lval_print(v->body); putchar(')');
      }
    break;
  }
}

void lval_println(lval* v){
  lval_print(v); putchar('\n');
}

lval* lval_eq(lval* a, lval* b){
  int is_equal;

  if(a->type != b->type){
    is_equal =  0;
  }

  switch(a->type){
    case LVAL_NUM:
      is_equal = (a->num == b->num);
      break;
    case LVAL_BOOL:
      is_equal = (a->truth == b->truth);
      break;
    case LVAL_SYM:
      is_equal = (strstr(a->symbol, b->symbol));
      break;
    case LVAL_FUNC:
      is_equal = lval_eq(a->formals, b->formals)->truth &&
        lval_eq(a->body, b->body)->truth;
      break;
    case LVAL_SEXPR:
    case LVAL_QEXPR:
      if(a->count != b->count){ is_equal = 0; break; }

      for(int i = 0; i < a->count; i++){
        is_equal = is_equal && lval_eq(a->cell[i], b->cell[i])->truth;
        if(!is_equal){break;}
      }
    break;
    default:
      return lval_err("Uncomparable type");
      break;
  }

  return lval_bool(is_equal);
}
