#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>
#include <editline/history.h>

#include "lib/mpc.h"

#define LASSERT(args, cond, err)\
  if(!(cond)){lval_del(args); return lval_err(err);}

// create new language structure: lval
// -----------------------------------

struct lval;
struct lenv;
typedef struct lval lval;
typedef struct lenv lenv;

void lval_print(lval* v);
lval* lval_eval_sexpr(lenv* e, lval* v);
lval* builtin_op(lenv* e, lval* a, char* op);
lval* builtin_head(lenv* e, lval* v);
lval* builtin_tail(lenv* e, lval* v);
lval* builtin_list(lenv* e, lval* v);
lval* builtin_eval(lenv* e, lval* v);

//
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

struct lenv{
  int count;
  lval** values;
  char** symbols;
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
  if(strstr(ast->tag, "qexpr")) {x = lval_qexpr();}

  for (int i = 0; i < ast->children_num; i++){
    if(strcmp(ast->children[i]->contents, "(") == 0){ continue; }
    if(strcmp(ast->children[i]->contents, ")") == 0){ continue; }
    if(strcmp(ast->children[i]->contents, "{") == 0){ continue; }
    if(strcmp(ast->children[i]->contents, "}") == 0){ continue; }
    if(strcmp(ast->children[i]->tag, "regex")  == 0){ continue; }
    x = lval_add(x, lval_read(ast->children[i]));
  }

  return x;
}

lval* lval_eval(lenv* e, lval* v){
  if(v->type == LVAL_SYM){
    lval* x = lenv_get(e, v);
    lval_del(v);
    return x;
  }
  if(v->type == LVAL_SEXPR){ return lval_eval_sexpr(e, v); }
  return v;
}

lval* lval_eval_sexpr(lenv* e, lval* v){

  // evaluate child expressions
  for(int i = 0; i < v->count; i++){
    v->cell[i] = lval_eval(e, v->cell[i]);
  }

  // if there are any errors, return the error
  for (int i = 0; i < v->count; i++){
    if (v->cell[i]->type == LVAL_ERR){ return lval_take(v, i); }
  }

  // if it is an empty expression, return 0
  if (v->count == 0){ return 0; }
  // if it is a single expression, return the contents
  if (v->count == 1){ return lval_take(v, 0); }

  // Take the first expression in the S-expression
  // which should be a function
  lval* f = lval_pop(v, 0);
  if (f->type != LVAL_FUNC){
    lval_del(f); lval_del(v);
    return lval_err("S-expression should start with a function!");
  }

  // symbol (operator)
  lval* result = f->func(e,v);
  lval_del(f);
  return result;
}

lval* builtin_op(lenv* e, lval* a, char* op){
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

lval* builtin_head(lenv* e, lval* v){
  LASSERT(v, v->count == 1,
    "Too many arguments passed to `head`, expected: 1");
  LASSERT(v, v->cell[0]->type == LVAL_QEXPR,
    "`head` expects input of type QEXPR");
  LASSERT(v, v->cell[0]->count != 0,
    "`head` was passed list {}, `head` is undefined for {}");

  lval* argument_qexpr = lval_take(v, 0);
  // unsure why I can't just lval_take(argument_qexpr, 0)
  while(argument_qexpr->count > 1){ lval_del(lval_pop(argument_qexpr, 1)); }
  return argument_qexpr;
}

lval* builtin_tail(lenv* e, lval* v){
  LASSERT(v, v->count == 1,
    "Too many arguments passed to `tail`, expected: 1");
  LASSERT(v, v->cell[0]->type == LVAL_QEXPR,
    "`tail` expects input of type QEXPR");
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
    "First argument to `def` must be a q-expression");

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

void lenv_add_builtin(lenv* e, char* sym, lbuiltin func){
  lval* symbol = lval_sym(sym);
  lval* value = lval_func(func);
  lenv_put(e, symbol, value);
  lval_del(symbol); lval_del(value);
}

void lenv_add_builtins(lenv* e){
  lenv_add_builtin(e, "list", builtin_list);
  lenv_add_builtin(e, "eval", builtin_eval);
  lenv_add_builtin(e, "head", builtin_head);
  lenv_add_builtin(e, "tail", builtin_tail);
  lenv_add_builtin(e, "join", builtin_join);
  lenv_add_builtin(e, "def", builtin_def);

  lenv_add_builtin(e, "+", builtin_add);
  lenv_add_builtin(e, "-", builtin_sub);
  lenv_add_builtin(e, "/", builtin_div);
  lenv_add_builtin(e, "*", builtin_mul);
}

// main loop
// ---------

int main(int argc, char** argv){

    mpc_parser_t* Number    = mpc_new("number");
    mpc_parser_t* Symbol    = mpc_new("symbol");
    mpc_parser_t* SExpr     = mpc_new("sexpr");
    mpc_parser_t* QExpr     = mpc_new("qexpr");
    mpc_parser_t* Expr      = mpc_new("expr");
    mpc_parser_t* Slip      = mpc_new("slip");


    mpca_lang(MPCA_LANG_DEFAULT,
            "                                               \
            number  : /-?[0-9]+/;                           \
            symbol  : /[[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/;  \
            sexpr   : '(' <expr>* ')';                      \
            qexpr   : '{' <expr>* '}';                      \
            expr    : <number> | <symbol> | <sexpr> | <qexpr>;\
            slip    : /^/ <expr>* /$/;                      \
            ",
            Number, Symbol, SExpr, QExpr, Expr, Slip);

    puts("Slip version 0.0.0.0");
    puts("Press ctrl+c to exit");

    lenv* global = lenv_new();
    lenv_add_builtins(global);
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
          x = lval_eval(global, x);
          lval_println(x);
          lval_del(x);

          mpc_ast_delete(r.output);
        }else{
          mpc_err_print(r.error);
          mpc_err_delete(r.error);
        }

        free(input);
    }

    lenv_del(global);
    mpc_cleanup(4, Number, Symbol, SExpr, QExpr, Expr, Slip);

    return 0;
}
