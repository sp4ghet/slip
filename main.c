#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>
#include <editline/history.h>

#include "lib/mpc.h"

long eval_op(long x, char* op, long y);

long eval(mpc_ast_t* ast){

  // numbers are evaluated as numbers
  if (strstr(ast->tag, "number")){
    return atoi(ast->contents);
  }

  // If an expression is not a number, it is an operator
  // The operator is the second child
  char* op = ast->children[1]->contents;

  // The third child is the first expression for the operator
  long x = eval(ast->children[2]);

  int i = 3;
  while(strstr(ast->children[i]->tag, "expr")){
    x = eval_op(x, op, eval(ast->children[i]));
    i++;
  }

  return x;
}

long eval_op(long x, char* op, long y){
  if (strcmp(op, "+") == 0) { return x + y; }
  if (strcmp(op, "*") == 0) { return x * y; }
  if (strcmp(op, "-") == 0) { return x - y; }
  if (strcmp(op, "/") == 0) { return x / y; }

  puts("Error parsing operator.");
  return 0;
}

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
          long result = eval(a);

          printf("%li\n", result);
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
