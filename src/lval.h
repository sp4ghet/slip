#ifndef lval_h
#define lval_h

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

// create new language structure: lval
// -----------------------------------
struct lval;
typedef struct lval lval;
struct lenv;
typedef struct lenv lenv;

typedef lval*(*lbuiltin)(lenv*, lval*);


// declares a new struct, lval
struct lval {
  int type;
  long num;
  char* err;
  char* symbol;
  int count;
  struct lval** cell;
  lbuiltin func;
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

#endif
