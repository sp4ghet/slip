#include "lenv.h"

// Environments
lenv*  lenv_new(void){
  lenv* e = malloc(sizeof(lenv));
  e->parent = NULL;
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

lenv* lenv_copy(lenv* e){
  lenv* new = malloc(sizeof(lenv));
  new->parent = e->parent;
  new->count = e->count;
  new->symbols = malloc(sizeof(char*) * new->count);
  new->values = malloc(sizeof(lval) * new->count);
  for(int i = 0; i < new->count; i++){
    new->symbols[i] = malloc(strlen(e->symbols[i]) + 1);
    strcpy(new->symbols[i], e->symbols[i]);
    new->values[i] = lval_copy(e->values[i]);
  }

  return new;
}

lval* lenv_get(lenv* e, lval* symbol){
  for(int i = 0; i < e->count; i++){
    if(strcmp(e->symbols[i], symbol->symbol) == 0){
      return lval_copy(e->values[i]);
    }
  }

  // if symbol is not found in the local scope, check in the parent scope
  if(e->parent != NULL){
    return lenv_get(e->parent, symbol);
  }

  return lval_err("unbound symbol");
}

void lenv_put(lenv* e, lval* symbol, lval* value){
  for(int i = 0; i < e->count; i++){
    if(strcmp(e->symbols[i], symbol->symbol) == 0){
      lval_del(e->values[i]);
      e->values[i] = lval_copy(value);
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

void lenv_def(lenv* e, lval* symbol, lval* value){
  // global scope has no parent
  while(e->parent != NULL){
    e = e->parent;
  }

  lenv_put(e, symbol, value);
}
