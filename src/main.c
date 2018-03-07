  #include <stdlib.h>
#include <stdio.h>

#include <editline/readline.h>
#include <editline/history.h>

#include "lib/mpc.h"

#include "lval.h"
#include "lenv.h"
#include "builtins.h"

lval* lval_eval_sexpr(lenv* e, lval* v);

// Evaluate the Abstract Syntax Tree
// ---------------------------------
lval* lval_read_num(mpc_ast_t* ast){
  errno = 0;
  long x = strtol(ast->contents, NULL, 10);
  return errno != ERANGE ?
    lval_num(x) : lval_err("invalid number %s", ast->contents);
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
  if (v->count == 0){ return v; }
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
  lval* result = lval_call(e, f, v);
  lval_del(f);
  return result;
}

lval* lval_call(lenv* e, lval* f, lval* v){
  // if there is a builtin function, call it.
  if(f->builtin != NULL){
    return f->builtin(e, v);
  }

  // we need to check that no more than the suppliable arguments are supplied
  // less is ok for currying
  LASSERT(v, v->count <= f->formals->count,
    "More arguments supplied than available in function.");

  int args_count = v->count;
  for(int i = 0; i < args_count; i++){
    lval* symbol = lval_pop(f->formals, 0);
    lval* bind  = lval_pop(v, 0);

    lenv_put(f->func_scope, symbol, bind);
    lval_del(symbol); lval_del(bind);
  }

  lval_del(v);

  if(f->formals->count == 0){
    
    f->func_scope->parent = e;

    return builtin_eval(f->func_scope,
      lval_add(lval_sexpr(), lval_copy(f->body)));
  }else{
    return lval_copy(f);
  }
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
  lenv_add_builtin(e, "let", builtin_put);
  lenv_add_builtin(e, "\\", builtin_lambda);

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
