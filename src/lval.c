#include "lval.h"

char* ltype_name(int ltype){
  switch(ltype){
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
  v->func = func;
  return v;
}


// delte, add, other operations to lval
void lval_del(lval* v){
  switch(v->type){
    case LVAL_NUM: break;
    case LVAL_FUNC: break;

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
    case LVAL_FUNC: x->func = v->func; break;

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
    case LVAL_NUM:
      printf("%li", v->num); break;
    case LVAL_ERR:
      printf("Error: %s", v->err); break;
    case LVAL_SYM:
      printf("%s", v->symbol); break;
    case LVAL_SEXPR: lval_expr_print(v, '(', ')'); break;
    case LVAL_QEXPR: lval_expr_print(v, '{', '}'); break;
    case LVAL_FUNC: printf("<function>"); break;
  }
}

void lval_println(lval* v){
  lval_print(v); putchar('\n');
}
