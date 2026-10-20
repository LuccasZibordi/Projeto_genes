
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "pontos.h"

#define QUANTIDADE 10

int main(void) {
    Ponto pontos[QUANTIDADE];
    Ponto repetidos[QUANTIDADE];

    unsigned int semente = 42;

    // Primeiro teste: geracao normal
    configurar_semente(semente);

    int sucesso = gerar_pontos_aleatorios(
        pontos,
        QUANTIDADE,
        0.0, 100.0,
        0.0, 100.0
    );

    assert(sucesso == 1);

    printf("=== PONTOS ALEATORIOS ===\n");

    for (int i = 0; i < QUANTIDADE; i++) {
        printf(
            "Cidade %d: (%.2f, %.2f)\n",
            i,
            pontos[i].x,
            pontos[i].y
        );

        // Verifica os limites
        assert(pontos[i].x >= 0.0);
        assert(pontos[i].x <= 100.0);
        assert(pontos[i].y >= 0.0);
        assert(pontos[i].y <= 100.0);
    }

    // Segundo teste: reproducibilidade
    configurar_semente(semente);

    sucesso = gerar_pontos_aleatorios(
        repetidos,
        QUANTIDADE,
        0.0, 100.0,
        0.0, 100.0
    );

    assert(sucesso == 1);

    for (int i = 0; i < QUANTIDADE; i++) {
        assert(pontos[i].x == repetidos[i].x);
        assert(pontos[i].y == repetidos[i].y);
    }

    // Terceiro teste: quantidade invalida
    sucesso = gerar_pontos_aleatorios(
        pontos,
        5,
        0.0, 100.0,
        0.0, 100.0
    );

    assert(sucesso == 0);

    // Quarto teste: limites invalidos
    sucesso = gerar_pontos_aleatorios(
        pontos,
        QUANTIDADE,
        100.0, 0.0,
        0.0, 100.0
    );

    assert(sucesso == 0);

    // Quinto teste: ponteiro nulo
    sucesso = gerar_pontos_aleatorios(
        NULL,
        QUANTIDADE,
        0.0, 100.0,
        0.0, 100.0
    );

    assert(sucesso == 0);

    printf("\nTodos os testes passaram!\n");

    return EXIT_SUCCESS;
}
