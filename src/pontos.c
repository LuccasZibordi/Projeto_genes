
#include <stdlib.h>
#include <math.h>

#include "pontos.h"

Ponto criar_ponto(double x, double y) {
    Ponto ponto;

    ponto.x = x;
    ponto.y = y;

    return ponto;
}

void configurar_semente(unsigned int semente) {
    srand(semente);
}

double aleatorio_intervalo(double minimo, double maximo) {
    double fator = (double)rand() / (double)RAND_MAX;

    return minimo + (maximo - minimo) * fator;
}

int gerar_pontos_aleatorios(
    Ponto *pontos,
    int quantidade,
    double minimo_x,
    double maximo_x,
    double minimo_y,
    double maximo_y
) {

    if (pontos == NULL) {
        return 0;
    }


    if (quantidade < 8) {
        return 0;
    }


    if (!isfinite(minimo_x) || !isfinite(maximo_x) ||
        !isfinite(minimo_y) || !isfinite(maximo_y)) {
        return 0;
    }

    if (minimo_x >= maximo_x || minimo_y >= maximo_y) {
        return 0;
    }


    if (!isfinite(maximo_x - minimo_x) ||
        !isfinite(maximo_y - minimo_y)) {
        return 0;
    }


    for (int i = 0; i < quantidade; i++) {
        pontos[i].x = aleatorio_intervalo(minimo_x, maximo_x);
        pontos[i].y = aleatorio_intervalo(minimo_y, maximo_y);
    }

    return 1;
}

int gerar_pontos_circulares(
    Ponto *pontos,
    int quantidade,
    double centro_x,
    double centro_y,
    double raio
) {

    if (pontos == NULL || quantidade < 8 ||
        !isfinite(centro_x) || !isfinite(centro_y) ||
        !isfinite(raio) || raio <= 0.0 ||
        !isfinite(centro_x + raio) ||
        !isfinite(centro_x - raio) ||
        !isfinite(centro_y + raio) ||
        !isfinite(centro_y - raio)) {
        return 0;
    }

    const double pi = acos(-1.0);


    for (int i = 0; i < quantidade; i++) {
        double angulo =
            2.0 * pi * (double)i / (double)quantidade;

        pontos[i].x = centro_x + raio * cos(angulo);
        pontos[i].y = centro_y + raio * sin(angulo);
    }

    return 1;
}

double distancia_otima_circular(int quantidade, double raio) {

    if (quantidade < 8 || !isfinite(raio) || raio <= 0.0) {
        return NAN;
    }

    const double pi = acos(-1.0);

    return 2.0 * (double)quantidade * raio *
           sin(pi / (double)quantidade);
}
