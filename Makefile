CC     = gcc
CFLAGS = -std=c11 -g -O1 -Wall -Wextra -Wpedantic -Iinclude
LDLIBS = -lm

FONTES = src/pontos.c src/individuo.c src/avaliacao.c \
         src/operadores.c src/genetico.c

ag_tsp: $(FONTES) src/main.c
	$(CC) $(CFLAGS) $(FONTES) src/main.c -o $@ $(LDLIBS)

test_genetico: $(FONTES) tests/test_genetico.c
	$(CC) $(CFLAGS) $(FONTES) tests/test_genetico.c -o $@ $(LDLIBS)

test: test_genetico
	./test_genetico

clean:
	rm -f ag_tsp test_genetico

.PHONY: test clean
