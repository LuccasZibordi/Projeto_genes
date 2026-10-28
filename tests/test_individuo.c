
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "individuo.h"

int main(void) {

    printf("=== TESTE DE INDIVIDUOS ===\n");


    Individuo *a = criar_individuo(8);
    assert(a != NULL);

    assert(validar_rota(a) == 1);

    printf("\nRota inicial:\n");
    imprimir_rota(a);


    int rota_valida[8] = {0, 3, 7, 1, 5, 2, 6, 4};

    for (int i = 0; i < 8; i++) {
        a->rota[i] = rota_valida[i];
    }

    assert(validar_rota(a) == 1);

    printf("\nRota modificada:\n");
    imprimir_rota(a);


    a->rota[5] = 3;

    assert(validar_rota(a) == 0);

    printf("\nTeste de cidade repetida: OK\n");


    a->rota[5] = 9;

    assert(validar_rota(a) == 0);

    printf("Teste de indice invalido: OK\n");


    a->rota[5] = 2;
    assert(validar_rota(a) == 1);


    Individuo *b = criar_individuo(8);
    assert(b != NULL);

    assert(copiar_individuo(b, a) == 1);

    for (int i = 0; i < 8; i++) {
        assert(b->rota[i] == a->rota[i]);
    }


    b->rota[0] = 4;

    assert(a->rota[0] == 0);

    printf("Teste de copia independente: OK\n");


    Individuo *invalido = criar_individuo(5);

    assert(invalido == NULL);

    printf("Teste de quantidade invalida: OK\n");


    assert(validar_rota(NULL) == 0);
    assert(copiar_individuo(NULL, a) == 0);

    printf("Teste de ponteiros nulos: OK\n");


    liberar_individuo(a);
    liberar_individuo(b);
    liberar_individuo(invalido);

    printf("\nTodos os testes passaram!\n");

    return EXIT_SUCCESS;
}
