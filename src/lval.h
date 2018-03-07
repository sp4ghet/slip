#ifndef lval_h
#define lval_h

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

// #include "lenv.h"

// create new language structure: lval
// -----------------------------------
struct lval;
typedef struct lval lval;
struct lenv;
typedef struct lenv lenv;

typedef lval*(*lbuiltin)(lenv*, lval*);


// declares a new struct, lval
struct lval {
  // type
  int type;

  // primitives
  long num;
  char* err;
  char* symbol;

  // lists
  int count;
  struct lval** cell;

  // functions
  lbuiltin builtin;
  lval* formals;
  lval* body;
  lenv* func_scope;
};
// valid types of LVAL
enum {
  LVAL_ERR,
  LVAL_NUM,
  LVAL_SYM,
  LVAL_SEXPR,
  LVAL_QEXPR,
  LVAL_FUNC
};

char* ltype_name(int ltype);

//constructors
lval* lval_num(long x);
lval* lval_err(char* err, ...);
lval* lval_sym(char* sym);
lval* lval_sexpr(void);
lval* lval_qexpr(void);
lval* lval_func(lbuiltin func);
lval* lval_lambda(lval* formal, lval* body);

//add, delete
void lval_del(lval* v);
lval* lval_add(lval* v, lval* next);

// print
void lval_expr_print(lval* v, char open, char close);
void lval_print(lval* v);
void lval_println(lval* v);

lval* lval_pop(lval* v, int i);
lval* lval_copy(lval* v);
lval* lval_take(lval* v, int i);
lval* lval_join(lval* x, lval* y);

// eval
lval* lval_eval(lenv* e, lval* v);
lval* lval_eval_sexpr(lenv* e, lval* v);
lval* lval_call(lenv* e, lval* f, lval* v);

#endif
