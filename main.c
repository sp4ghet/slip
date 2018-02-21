#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>
#include <editline/history.h>

#include "lib/mpc.h"

// create new language structure: lval
// -----------------------------------

// declares a new struct, lval
typedef struct {
  int type;
  long num;
  int err;
} lval;

// types in our system
enum {LVAL_ERR, LVAL_NUM};
// type of errors
enum {LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM};

// lval constructors
// -----------------
lval lval_num(long x){
  lval v;
  v.type = LVAL_NUM;
  v.num = x;
  return v;
}

// err should be of type enum LERR_*
lval lval_err(int err){
  lval v;
  v.type = LVAL_ERR;
  v.err = err;
  return v;
}

void lval_print(lval v){
  switch(v.type){
    case LVAL_NUM:
      printf("%li", v.num);
      break;

    case LVAL_ERR:
      switch(v.err){
        case LERR_DIV_ZERO:
          printf("Divide by zero error");
          break;
        case LERR_BAD_OP:
          printf("Invalid operator");
          break;
        case LERR_BAD_NUM:
          printf("Invalid number");
          break;
      }
    break;
  }
}

void lval_println(lval v){
  lval_print(v); putchar('\n');
}

// Evaluate the Abstract Syntax Tree
// ---------------------------------

lval eval_op(lval x, char* op, lval y){

  if (x.type == LVAL_ERR) { return x;}
  if (y.type == LVAL_ERR) { return y;}

  // The implicit assumption is that x and y are type LVAL_NUM at this point



  if (strcmp(op, "+") == 0) { return lval_num(x.num + y.num); }
  if (strcmp(op, "*") == 0) { return lval_num(x.num * y.num); }
  if (strcmp(op, "-") == 0) { return lval_num(x.num - y.num); }
  if (strcmp(op, "/") == 0) {
    return y.num == 0
      ? lval_err(LERR_DIV_ZERO)
      : lval_num(x.num / y.num);
  }

  return lval_err(LERR_BAD_OP);
}

lval eval(mpc_ast_t* ast){

  // numbers are evaluated as numbers
  if (strstr(ast->tag, "number")){
    long n = atoi(ast->contents);
    return lval_num(n);
  }

  // If an expression is not a number, it is an operator
  // The operator is the second child
  char* op = ast->children[1]->contents;

  // The third child is the first expression for the operator
  lval x = eval(ast->children[2]);

  int i = 3;
  while(strstr(ast->children[i]->tag, "expr")){
    x = eval_op(x, op, eval(ast->children[i]));
    i++;
  }

  return x;
}

// main loop
// ---------

int main(int argc, char** argv){

    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Operator = mpc_new("operator");
    mpc_parser_t* Expr      = mpc_new("expr");
    mpc_parser_t* Slip      = mpc_new("slip");


    mpca_lang(MPCA_LANG_DEFAULT,
            "                                               \
            number  : /-?[0-9]+/;                           \
            operator: '+' | '-' | '*' | '/';                \
            expr    : <number> | '(' <operator> <expr>+ ')';\
            slip    : /^/ <operator> <expr>+ /$/;           \
            ",
            Number, Operator, Expr, Slip);

    puts("Slip version 0.0.0.0");
    puts("Press ctrl+c to exit");

    // REPL(oop)
    while(1){
        char* input = readline("slip> ");

        add_history(input);


        mpc_result_t r;

        if(mpc_parse("<stdin>", input, Slip, &r)){
          // AST successfully read
          // mpc_ast_print(r.output);

          // evaluate the AST
          mpc_ast_t* a = r.output;
          lval result = eval(a);

          lval_println(result);
          mpc_ast_delete(r.output);
        }else{
          mpc_err_print(r.error);
          mpc_err_delete(r.error);
        }

        free(input);
    }

    mpc_cleanup(4, Number, Operator, Expr, Slip);

    return 0;
}
