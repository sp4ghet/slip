#include "lenv.h"

// Environments
lenv*  lenv_new(void){
  lenv* e = malloc(sizeof(lenv));
  e->count = 0;
  e->symbols = NULL;
  e->values = NULL;
  return e;
}

void lenv_del(lenv* e){
  for(int i = 0; i < e->count; i++){
    free(e->symbols[i]);
    lval_del(e->values[i]);
  }
  free(e->symbols);
  free(e->values);
  free(e);
}

lval* lenv_get(lenv* e, lval* symbol){
  for(int i = 0; i < e->count; i++){
    if(strcmp(e->symbols[i], symbol->symbol) == 0){
      return lval_copy(e->values[i]);
    }
  }

  return lval_err("unbound symbol");
}

void lenv_put(lenv* e, lval* symbol, lval* value){
  for(int i = 0; i < e->count; i++){
    if(strcmp(e->symbols[i], symbol->symbol) == 0){
      lval_del(e->values[i]);
      e->values[i] = value;
      return;
    }
  }

  e->count++;
  e->symbols = realloc(e->symbols, sizeof(lval*) * e->count);
  e->values = realloc(e->values, sizeof(char*) * e->count);

  e->values[e->count - 1] = lval_copy(value);
  e->symbols[e->count - 1] = malloc(strlen(symbol->symbol) + 1);
  strcpy(e->symbols[e->count - 1], symbol->symbol);
}
