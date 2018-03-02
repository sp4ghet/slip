CC=cc
CFLAGS=-std=c99 -Wall -I.

slip: object/slip.o object/mpc.o object/lval.o object/lenv.o object/builtins.o
	$(CC) -o slip object/slip.o object/mpc.o object/lval.o object/lenv.o object/builtins.o -ledit -lm $(CFLAGS)

object/slip.o: src/main.c
	$(CC) -o object/slip.o -c src/main.c $(CFLAGS)

object/lval.o: src/lval.c
	$(CC) -o object/lval.o -c src/lval.c $(CFLAGS)

object/lenv.o: src/lenv.c
	$(CC) -o object/lenv.o -c src/lenv.c $(CFLAGS)

object/builtins.o: src/builtins.c
	$(CC) -o object/builtins.o -c src/builtins.c $(CFLAGS)

object/mpc.o: lib/mpc.c lib/mpc.h
	$(CC) -o object/mpc.o -c lib/mpc.c $(FLAGS)

clean:
	rm object/slip.o object/mpc.o object/lval.o object/lenv.o object/builtins.o
