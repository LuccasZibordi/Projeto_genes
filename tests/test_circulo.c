
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "pontos.h"

#define N 12
#define EPS 1e-9

int main(void) {
    Ponto pontos[N];

    const double cx = 50.0;
    const double cy = 50.0;
    const double raio = 20.0;

    // Gerar 12 pontos em uma circunferencia
    assert(gerar_pontos_circulares(
        pontos, N, cx, cy, raio
    ) == 1);

    double distancia_perimetro = 0.0;

    printf("=== PONTOS CIRCULARES ===\n");

    for (int i = 0; i < N; i++) {

        printf(
            "Cidade %2d: (%.6f, %.6f)\n",
            i, pontos[i].x, pontos[i].y
        );

        // Verifica a distancia ate o centro
        double dx = pontos[i].x - cx;
        double dy = pontos[i].y - cy;

        assert(fabs(hypot(dx, dy) - raio) < EPS);

        // Proximo ponto, incluindo retorno ao primeiro
        int proximo = (i + 1) % N;

        distancia_perimetro += hypot(
            pontos[proximo].x - pontos[i].x,
            pontos[proximo].y - pontos[i].y
        );
    }

    double otima = distancia_otima_circular(N, raio);

    // Compara percurso calculado e valor teorico
    assert(fabs(distancia_perimetro - otima) < EPS);

    // Verifica parametros invalidos
    assert(gerar_pontos_circulares(
        NULL, N, cx, cy, raio
    ) == 0);

    assert(gerar_pontos_circulares(
        pontos, 7, cx, cy, raio
    ) == 0);

    assert(gerar_pontos_circulares(
        pontos, N, cx, cy, 0.0
    ) == 0);

    assert(isnan(distancia_otima_circular(7, raio)));

    printf("\nDistancia calculada: %.9f\n",
           distancia_perimetro);

    printf("Distancia otima: %.9f\n", otima);

    printf("\nTodos os testes do circulo passaram!\n");

    return EXIT_SUCCESS;
}
