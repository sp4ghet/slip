#include "builtins.h"

// add, sub, mul, div functions
lval* builtin_op(lenv* e, lval* a, char* op){
  for (int i = 0; i < a->count; i++){
    LASSERT(a, a->cell[i]->type == LVAL_NUM,
      "Invalid type: expected `%s`, got: `%s`", ltype_name(LVAL_NUM), ltype_name(a->cell[i]->type));
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

lval* builtin_head(lenv* e, lval* v){
  LASSERT(v, v->count == 1,
    "Too many arguments passed to `head`, expected: 1, got: %i", v->count);
  LASSERT(v, v->cell[0]->type == LVAL_QEXPR,
    "`head` expects input of type `q-expression`, got: `%s`", ltype_name(v->cell[0]->type));
  LASSERT(v, v->cell[0]->count != 0,
    "`head` was passed list {}, `head` is undefined for {}");

  lval* argument_qexpr = lval_take(v, 0);
  // unsure why I can't just lval_take(argument_qexpr, 0)
  while(argument_qexpr->count > 1){ lval_del(lval_pop(argument_qexpr, 1)); }
  return argument_qexpr;
}

lval* builtin_tail(lenv* e, lval* v){
  LASSERT(v, v->count == 1,
    "Too many arguments passed to `tail`, expected: 1, got: %i", v->count);
  LASSERT(v, v->cell[0]->type == LVAL_QEXPR,
    "`tail` expects input of type `q-expression`, got: %s", ltype_name(v->cell[0]->type));
  LASSERT(v, v->cell[0]->count != 0,
    "`tail` was passed list {}, `tail` is undefined for {}");

  lval* list = lval_take(v, 0);
  lval_del(lval_pop(list, 0));
  return list;
}

// takes an S-expression and returns the contents as a Q-expression
// list(s...) -> q(s...)
lval* builtin_list(lenv* e, lval* v){
  v->type = LVAL_QEXPR;
  return v;
}

// takes a qexpression that is not {} and returns the contents evalutated
lval* builtin_eval(lenv* e, lval* v){
  LASSERT(v, v->count == 1,
    "Too many arguments passed to `eval`");
  LASSERT(v, v->cell[0]->type == LVAL_QEXPR,
    "Argument passed to `eval` should be Q-expression");
  LASSERT(v, v->cell[0]->count != 0,
    "`eval` was passed {}, need non-empty Q-expression");

  lval* x = lval_take(v, 0);
  x->type = LVAL_SEXPR;
  return lval_eval(e, x);
}

lval* builtin_join(lenv* e, lval* qs){
  for(int i = 0; i < qs->count; i++){
    LASSERT(qs, qs->cell[i]->type == LVAL_QEXPR,
      "Arguments passed to `join` need to be all Q-expressions");
  }

  // take out one q-expression
  // join {hoge} <this{fuga}> {piyo} ...
  lval* x = lval_pop(qs, 0);

  // while there are more q-expressions (piyo)
  // join the two q-expressions together
  while(qs->count > 0){
    x = lval_join(x, lval_pop(qs, 0));
  }

  lval_del(qs);
  return x;
}

lval* builtin_add(lenv* e, lval* v){
  return builtin_op(e, v, "+");
}

lval* builtin_sub(lenv* e, lval* v){
  return builtin_op(e, v, "-");
}

lval* builtin_mul(lenv* e, lval* v){
  return builtin_op(e, v, "*");
}

lval* builtin_div(lenv* e, lval* v){
  return builtin_op(e, v, "/");
}

lval* builtin_def(lenv* e, lval* v){
  LASSERT(v, v->cell[0]->type == LVAL_QEXPR,
    "First argument to `def` must be a q-expression, got: %s", ltype_name(v->cell[0]->type));

  // first argument is a list of symbols
  lval* symbols = v->cell[0];

  for(int i = 0; i < symbols->count; i++){
    LASSERT(v, symbols->cell[i]->type == LVAL_SYM,
      "Cannot bind to invalid variable name");
  }

  LASSERT(v, symbols->count == v->count - 1,
    "Number of symbols does not match number of expressions");

  for (int i = 0; i < symbols->count; i++){
    lenv_put(e, symbols->cell[i], v->cell[i+1]);
  }

  lval_del(v);
  return lval_sexpr();
}
