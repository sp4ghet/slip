#include <stdio.h>

static char input[2048];

int main(int argc, char** argv){
    
    puts("Slip version 0.0.0.0");
    puts("Press ctrl+c to exit");

    // REPL(oop)
    while(1){
        fputs("slip> ", stdout);
        fgets(input, 2048, stdin);
        
        printf("%s", input);
    }

    return 0;
}
