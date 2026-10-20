
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "pontos.h"
#include "individuo.h"
#include "avaliacao.h"

#define EPS 1e-9

// Compara valores de ponto flutuante usando
// tolerancia relativa e absoluta.
int aproximadamente_igual(double a, double b) {
    return fabs(a - b) <= EPS * fmax(1.0, fmax(fabs(a), fabs(b)));
}

// Testa todos os modulos no cenario aleatorio.
void testar_aleatorio(int n) {

    printf("\n[TESTE ALEATORIO] %d pontos\n", n);

    Ponto *pontos = malloc((size_t)n * sizeof(Ponto));
    Ponto *copia_pontos = malloc((size_t)n * sizeof(Ponto));

    assert(pontos != NULL);
    assert(copia_pontos != NULL);

    // Usa uma semente fixa para reproduzir o teste.
    configurar_semente(42);

    assert(gerar_pontos_aleatorios(
        pontos, n,
        0.0, 100.0,
        0.0, 100.0
    ) == 1);

    // Verifica se todos os pontos estao na regiao.
    for (int i = 0; i < n; i++) {
        assert(isfinite(pontos[i].x));
        assert(isfinite(pontos[i].y));

        assert(pontos[i].x >= 0.0);
        assert(pontos[i].x <= 100.0);
        assert(pontos[i].y >= 0.0);
        assert(pontos[i].y <= 100.0);
    }

    // Verifica a reproducibilidade da semente.
    configurar_semente(42);

    assert(gerar_pontos_aleatorios(
        copia_pontos, n,
        0.0, 100.0,
        0.0, 100.0
    ) == 1);

    for (int i = 0; i < n; i++) {
        assert(pontos[i].x == copia_pontos[i].x);
        assert(pontos[i].y == copia_pontos[i].y);
    }

    // Cria e valida o individuo.
    Individuo *individuo = criar_individuo(n);
    assert(individuo != NULL);
    assert(validar_rota(individuo) == 1);

    double distancia = distancia_total(individuo, pontos, n);
    double fitness = calcular_fitness(individuo, pontos, n);

    assert(isfinite(distancia));
    assert(distancia > 0.0);

    assert(isfinite(fitness));
    assert(fitness > 0.0);
    assert(fitness < 1.0);

    assert(aproximadamente_igual(
        fitness, 1.0 / (1.0 + distancia)
    ));

    // Testa a copia independente.
    Individuo *copia = criar_individuo(n);
    assert(copia != NULL);

    assert(copiar_individuo(copia, individuo) == 1);

    // A copia possui os mesmos valores.
    for (int i = 0; i < n; i++) {
        assert(copia->rota[i] == individuo->rota[i]);
    }

    // Mas nao compartilha o mesmo vetor.
    assert(copia->rota != individuo->rota);

    // Trocar duas cidades mantem uma rota valida.
    int temp = copia->rota[0];
    copia->rota[0] = copia->rota[1];
    copia->rota[1] = temp;

    assert(validar_rota(copia) == 1);

    // A alteracao nao modifica o original.
    assert(individuo->rota[0] == 0);
    assert(individuo->rota[1] == 1);

    // Verifica se duplicatas sao detectadas.
    copia->rota[1] = copia->rota[0];

    assert(validar_rota(copia) == 0);
    assert(isnan(distancia_total(copia, pontos, n)));

    printf("Distancia inicial: %.6f\n", distancia);
    printf("Fitness inicial: %.9f\n", fitness);
    printf("Validacao e copia: OK\n");

    liberar_individuo(copia);
    liberar_individuo(individuo);

    free(copia_pontos);
    free(pontos);

    printf("Resultado: OK\n");
}

// Testa todos os modulos no cenario circular.
void testar_circular(int n) {

    printf("\n[TESTE CIRCULAR] %d pontos\n", n);

    const double raio = 50.0;
    const double cx = 100.0;
    const double cy = 100.0;

    Ponto *pontos = malloc((size_t)n * sizeof(Ponto));
    assert(pontos != NULL);

    assert(gerar_pontos_circulares(
        pontos, n, cx, cy, raio
    ) == 1);

    // Verifica a distancia de cada ponto ao centro.
    for (int i = 0; i < n; i++) {
        double d = hypot(
            pontos[i].x - cx,
            pontos[i].y - cy
        );

        assert(aproximadamente_igual(d, raio));
    }

    Individuo *individuo = criar_individuo(n);
    assert(individuo != NULL);

    // A rota 0,1,2,...,n-1 percorre o circulo
    // na ordem geometrica correta.
    double obtida = distancia_total(
        individuo, pontos, n
    );

    double teorica = distancia_otima_circular(
        n, raio
    );

    assert(isfinite(obtida));
    assert(isfinite(teorica));
    assert(aproximadamente_igual(obtida, teorica));

    // Modifica a ordem para criar uma rota pior.
    int meio = n / 2;

    int temp = individuo->rota[1];
    individuo->rota[1] = individuo->rota[meio];
    individuo->rota[meio] = temp;

    assert(validar_rota(individuo) == 1);

    double pior = distancia_total(
        individuo, pontos, n
    );

    assert(pior > obtida);

    printf("Distancia teorica: %.9f\n", teorica);
    printf("Distancia calculada: %.9f\n", obtida);
    printf("Rota modificada: %.9f\n", pior);

    printf("Erro relativo: %.12f%%\n",
        100.0 * fabs(obtida - teorica) / teorica
    );

    liberar_individuo(individuo);
    free(pontos);

    printf("Resultado: OK\n");
}

int main(void) {

    const int tamanhos[] = {8, 20, 100, 1000};
    const int quantidade_testes =
        (int)(sizeof(tamanhos) / sizeof(tamanhos[0]));

    clock_t inicio = clock();

    printf("====================================\n");
    printf(" TESTES INTEGRADOS - INTEGRANTE 1\n");
    printf("====================================\n");

    for (int i = 0; i < quantidade_testes; i++) {
        testar_aleatorio(tamanhos[i]);
        testar_circular(tamanhos[i]);
    }

    clock_t fim = clock();

    if (inicio != (clock_t)-1 && fim != (clock_t)-1) {
        double tempo = (double)(fim - inicio) /
                       CLOCKS_PER_SEC;

        printf("\nTempo de CPU dos testes: %.6f s\n",
               tempo);
    }

    printf("\n====================================\n");
    printf(" TODOS OS TESTES PASSARAM!\n");
    printf("====================================\n");

    return EXIT_SUCCESS;
}
