#ifndef lenv_h
#define lenv_h

#include "lval.h"

struct lenv{
  int count;
  lval** values;
  char** symbols;
};

// Environments
lenv*  lenv_new(void);
void lenv_del(lenv* e);
lval* lenv_get(lenv* e, lval* symbol);
void lenv_put(lenv* e, lval* symbol, lval* value);

#endif
