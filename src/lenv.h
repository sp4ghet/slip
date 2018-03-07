#ifndef lenv_h
#define lenv_h

#include "lval.h"

struct lenv{
  lenv* parent;
  int count;
  lval** values;
  char** symbols;
};

// Environments
lenv*  lenv_new(void);
void lenv_del(lenv* e);
lenv* lenv_copy(lenv* e);
lval* lenv_get(lenv* e, lval* symbol);

// define in function scope
void lenv_put(lenv* e, lval* symbol, lval* value);
// define in global
void lenv_def(lenv* e, lval* symbol, lval* value);

#endif
