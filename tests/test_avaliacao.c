
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "avaliacao.h"
#include "individuo.h"
#include "pontos.h"

#define N 8
#define EPS 1e-9

int main(void) {

    printf("=== TESTES DE AVALIACAO ===\n");

    // Oito pontos no contorno de um quadrado
    Ponto pontos[N] = {
        {0.0, 0.0},
        {1.0, 0.0},
        {2.0, 0.0},
        {2.0, 1.0},
        {2.0, 2.0},
        {1.0, 2.0},
        {0.0, 2.0},
        {0.0, 1.0}
    };

    // Teste 1: distancia euclidiana
    Ponto a = criar_ponto(0.0, 0.0);
    Ponto b = criar_ponto(3.0, 4.0);

    assert(fabs(distancia_euclidiana(a, b) - 5.0) < EPS);

    printf("Distancia euclidiana: OK\n");

    // Teste 2: rota inicial valida
    Individuo *individuo = criar_individuo(N);
    assert(individuo != NULL);

    double distancia = distancia_total(
        individuo, pontos, N
    );

    assert(fabs(distancia - 8.0) < EPS);

    printf("Distancia da rota: %.6f\n", distancia);

    // Teste 3: fitness
    double fitness = calcular_fitness(
        individuo, pontos, N
    );

    double fitness_esperado = 1.0 / 9.0;

    assert(fabs(fitness - fitness_esperado) < EPS);

    printf("Fitness: %.9f\n", fitness);

    // Teste 4: uma rota pior deve ter menor fitness
    individuo->rota[1] = 2;
    individuo->rota[2] = 1;

    assert(validar_rota(individuo) == 1);

    double distancia_pior = distancia_total(
        individuo, pontos, N
    );

    double fitness_pior = calcular_fitness(
        individuo, pontos, N
    );

    assert(distancia_pior > distancia);
    assert(fitness_pior < fitness);

    printf("Comparacao entre individuos: OK\n");

    // Teste 5: rota com cidade repetida
    individuo->rota[2] = 2;

    assert(validar_rota(individuo) == 0);
    assert(isnan(distancia_total(individuo, pontos, N)));
    assert(isnan(calcular_fitness(individuo, pontos, N)));

    printf("Rota invalida: OK\n");

    // Restaura a rota original
    assert(inicializar_rota(individuo) == 1);

    // Teste 6: numero de pontos incompatível
    assert(isnan(distancia_total(
        individuo, pontos, N + 1
    )));

    // Teste 7: ponteiros nulos
    assert(isnan(distancia_total(NULL, pontos, N)));
    assert(isnan(distancia_total(individuo, NULL, N)));
    assert(isnan(calcular_fitness(NULL, pontos, N)));

    printf("Parametros invalidos: OK\n");

    // Teste 8: benchmark circular
    Ponto circulo[12];

    assert(gerar_pontos_circulares(
        circulo, 12, 50.0, 50.0, 20.0
    ) == 1);

    Individuo *circular = criar_individuo(12);
    assert(circular != NULL);

    double calculada = distancia_total(
        circular, circulo, 12
    );

    double teorica = distancia_otima_circular(12, 20.0);

    assert(fabs(calculada - teorica) < EPS);

    printf("Benchmark circular: OK\n");

    // Libera memoria
    liberar_individuo(individuo);
    liberar_individuo(circular);

    printf("\nTodos os testes passaram!\n");

    return EXIT_SUCCESS;
}
