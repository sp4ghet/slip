#ifndef builtins_h
#define builtins_h

#include "lenv.h"
#include "lval.h"

#define LASSERT(args, cond, fmt, ...)\
  if(!(cond)){\
    lval* err = lval_err(fmt, ##__VA_ARGS__); \
    lval_del(args);\
    return err;\
  }

#define LASSERT_NUM()

lval* builtin_op(lenv* e, lval* a, char* op);

lval* builtin_head(lenv* e, lval* v);
lval* builtin_tail(lenv* e, lval* v);
lval* builtin_list(lenv* e, lval* v);
lval* builtin_eval(lenv* e, lval* v);
lval* builtin_join(lenv* e, lval* qs);

lval* builtin_add(lenv* e, lval* v);
lval* builtin_sub(lenv* e, lval* v);
lval* builtin_mul(lenv* e, lval* v);
lval* builtin_div(lenv* e, lval* v);

lval* builtin_def(lenv* e, lval* v);
lval* builtin_put(lenv* e, lval* v);
lval* builtin_bind(lenv* e, lval* v, char* func);
lval* builtin_lambda(lenv* e, lval* v);

#endif
