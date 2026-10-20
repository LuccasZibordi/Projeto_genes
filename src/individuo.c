
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "individuo.h"

Individuo *criar_individuo(int quantidade) {

    if (quantidade < 8) {
        return NULL;
    }

    if ((size_t)quantidade > SIZE_MAX / sizeof(int)) {
        return NULL;
    }

    Individuo *individuo = malloc(sizeof(Individuo));

    if (individuo == NULL) {
        return NULL;
    }

    individuo->rota = malloc(
        (size_t)quantidade * sizeof(int)
    );

    if (individuo->rota == NULL) {
        free(individuo);
        return NULL;
    }

    individuo->quantidade = quantidade;

    // Inicializa imediatamente para evitar rota indefinida
    inicializar_rota(individuo);

    return individuo;
}

int inicializar_rota(Individuo *individuo) {

    if (individuo == NULL ||
        individuo->rota == NULL ||
        individuo->quantidade < 8) {
        return 0;
    }

    for (int i = 0; i < individuo->quantidade; i++) {
        individuo->rota[i] = i;
    }

    return 1;
}

int validar_rota(const Individuo *individuo) {

    if (individuo == NULL ||
        individuo->rota == NULL ||
        individuo->quantidade < 8) {
        return 0;
    }

    int n = individuo->quantidade;

    // Cada posicao indica se uma cidade ja apareceu
    unsigned char *visitados = calloc(
        (size_t)n, sizeof(unsigned char)
    );

    if (visitados == NULL) {
        return 0;
    }

    for (int i = 0; i < n; i++) {

        int cidade = individuo->rota[i];

        // Verifica indices fora do intervalo
        if (cidade < 0 || cidade >= n) {
            free(visitados);
            return 0;
        }

        // Verifica cidades repetidas
        if (visitados[cidade]) {
            free(visitados);
            return 0;
        }

        visitados[cidade] = 1;
    }

    free(visitados);

    return 1;
}

int copiar_individuo(
    Individuo *destino,
    const Individuo *origem
) {

    if (destino == NULL || origem == NULL ||
        destino->rota == NULL || origem->rota == NULL) {
        return 0;
    }

    if (destino->quantidade != origem->quantidade ||
        destino->quantidade < 8) {
        return 0;
    }

    // Aceita copiar o individuo para ele mesmo
    if (destino == origem) {
        return 1;
    }

    memcpy(
        destino->rota,
        origem->rota,
        (size_t)origem->quantidade * sizeof(int)
    );

    return 1;
}

void imprimir_rota(const Individuo *individuo) {

    if (individuo == NULL ||
        individuo->rota == NULL ||
        individuo->quantidade < 8) {
        printf("Individuo invalido.\n");
        return;
    }

    for (int i = 0; i < individuo->quantidade; i++) {
        printf("%d -> ", individuo->rota[i]);
    }

    printf("%d\n", individuo->rota[0]);
}

void liberar_individuo(Individuo *individuo) {

    if (individuo == NULL) {
        return;
    }

    free(individuo->rota);
    free(individuo);
}
