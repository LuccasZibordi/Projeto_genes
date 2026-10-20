
#ifndef PONTOS_H
#define PONTOS_H

typedef struct {
    double x;
    double y;
} Ponto;

Ponto criar_ponto(double x, double y);

void configurar_semente(unsigned int semente);

double aleatorio_intervalo(double minimo, double maximo);

int gerar_pontos_aleatorios(
    Ponto *pontos,
    int quantidade,
    double minimo_x,
    double maximo_x,
    double minimo_y,
    double maximo_y
);

// Gera pontos igualmente espacados sobre uma circunferencia
int gerar_pontos_circulares(
    Ponto *pontos,
    int quantidade,
    double centro_x,
    double centro_y,
    double raio
);

// Calcula a distancia otima do cenario circular
double distancia_otima_circular(int quantidade, double raio);

#endif
