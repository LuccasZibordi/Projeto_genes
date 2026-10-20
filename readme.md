como executar:

gcc -std=c11 -g -O1 -Wall -Wextra -Wpedantic \
    -fsanitize=address,undefined \
    -fno-omit-frame-pointer \
    -Iinclude \
    src/pontos.c src/individuo.c src/avaliacao.c \
    tests/test_integracao.c \
    -lm -o teste_integracao_san