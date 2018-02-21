#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>
#include <editline/history.h>

#include "lib/mpc.h"

// create new language structure: lval
// -----------------------------------

// declares a new struct, lval
typedef struct lval {
  int type;
  long num;
  char* err;
  char* symbol;
  int count;
  struct lval** cell;
} lval;


void lval_print(lval* v);
lval* lval_eval_sexpr(lval* v);
lval* builtin_op(lval* a, char* op);

// types in our system
enum {LVAL_ERR, LVAL_NUM, LVAL_SYM, LVAL_SEXPR};

// lval constructors
// -----------------
lval* lval_num(long x){
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_NUM;
  v->num = x;
  return v;
}

lval* lval_err(char* err){
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_ERR;
  v->err = malloc(strlen(err)+1);
  strcpy(v->err, err);
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

void lval_del(lval* v){
  switch(v->type){
    case LVAL_NUM: break;

    case LVAL_ERR: free(v->err); break;
    case LVAL_SYM: free(v->symbol); break;

    case LVAL_SEXPR:
      for(int i = 0; i < v->count; i++){
        lval_del(v->cell[i]);
      }
      free(v->cell);
    break;
  }

  free(v);
}

lval* lval_add(lval* v, lval* next){
  v->count++;
  v->cell = realloc(v->cell, sizeof(lval*) * v->count);
  v->cell[v->count-1] = next;
  return v;
}

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
  }
}

void lval_println(lval* v){
  lval_print(v); putchar('\n');
}

// Evaluate the Abstract Syntax Tree
// ---------------------------------

lval* lval_read_num(mpc_ast_t* ast){
  errno = 0;
  long x = strtol(ast->contents, NULL, 10);
  return errno != ERANGE ?
    lval_num(x) : lval_err("invalid number");
}

lval* lval_read(mpc_ast_t* ast){

  if(strstr(ast->tag, "number")) {return lval_read_num(ast);}
  if(strstr(ast->tag, "symbol")) {return lval_sym(ast->contents);}

  lval* x = NULL;
  if(strcmp(ast->tag, ">") == 0) { x = lval_sexpr();}
  if(strstr(ast->tag, "sexpr")) {x = lval_sexpr();}

  for (int i = 0; i < ast->children_num; i++){
    if(strcmp(ast->children[i]->contents, "(") == 0){ continue; }
    if(strcmp(ast->children[i]->contents, ")") == 0){ continue; }
    if(strcmp(ast->children[i]->tag, "regex")  == 0){ continue; }
    x = lval_add(x, lval_read(ast->children[i]));
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

lval* lval_eval(lval* v){
  if(v->type == LVAL_SEXPR){ return lval_eval_sexpr(v); }
  return v;
}

lval* lval_eval_sexpr(lval* v){

  for(int i = 0; i < v->count; i++){
    v->cell[i] = lval_eval(v->cell[i]);
  }

  for (int i = 0; i < v->count; i++){
    if (v->cell[i]->type == LVAL_ERR){ return lval_take(v, i); }
  }

  if (v->count == 0){ return 0; }
  if (v->count == 1){ return lval_take(v, 0); }

  lval* f = lval_pop(v, 0);
  if (f->type != LVAL_SYM){
    lval_del(f); lval_del(v);
    return lval_err("S-expression should start with a symbol!");
  }

  lval* result = builtin_op(v, f->symbol);
  lval_del(f);
  return result;
}

lval* builtin_op(lval* a, char* op){
  for (int i = 0; i < a->count; i++){
    if (a->cell[i]->type != LVAL_NUM){
      lval_del(a);
      return lval_err("Cannot operate on a non-number");
    }
  }

  lval* x = lval_pop(a, 0);

  if((strcmp(op, "-") == 0) && a->count == 0){
    x->num = -x->num;
  }

  while (a->count > 0){
    lval* y = lval_pop(a, 0);

    if(strcmp(op, "+") == 0){ x->num += y->num; }
    if(strcmp(op, "-") == 0){ x->num -= y->num; }
    if(strcmp(op, "*") == 0){ x->num *= y->num; }
    if(strcmp(op, "/") == 0){
      if(y->num == 0){
        lval_del(x); lval_del(y);
        x = lval_err("Division by zero"); break;
      }
      x->num /= y->num;
    }

    lval_del(y);
  }

  lval_del(a); return x;
}

// main loop
// ---------

int main(int argc, char** argv){

    mpc_parser_t* Number    = mpc_new("number");
    mpc_parser_t* Symbol    = mpc_new("symbol");
    mpc_parser_t* SExpr     = mpc_new("sexpr");
    mpc_parser_t* Expr      = mpc_new("expr");
    mpc_parser_t* Slip      = mpc_new("slip");


    mpca_lang(MPCA_LANG_DEFAULT,
            "                                               \
            number  : /-?[0-9]+/;                           \
            symbol  : '+' | '-' | '*' | '/';                \
            sexpr   : '(' <expr>* ')';                      \
            expr    : <number> | <symbol> | <sexpr>;          \
            slip    : /^/ <expr>* /$/;                      \
            ",
            Number, Symbol, SExpr, Expr, Slip);

    puts("Slip version 0.0.0.0");
    puts("Press ctrl+c to exit");

    // REPL(oop)
    while(1){
        char* input = readline("slip> ");

        add_history(input);

        mpc_result_t r;

        if(mpc_parse("<stdin>", input, Slip, &r)){
          // AST successfully read
          // mpc_ast_print(r.output);

          // parse the AST
          lval* x = lval_read(r.output);
          // evaluate the l-values
          x = lval_eval(x);
          lval_println(x);
          lval_del(x);

          mpc_ast_delete(r.output);
        }else{
          mpc_err_print(r.error);
          mpc_err_delete(r.error);
        }

        free(input);
    }

    mpc_cleanup(4, Number, Symbol, SExpr, Expr, Slip);

    return 0;
}
