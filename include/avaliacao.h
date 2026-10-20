
#ifndef AVALIACAO_H
#define AVALIACAO_H

#include "pontos.h"
#include "individuo.h"

// Calcula a distancia entre dois pontos
double distancia_euclidiana(Ponto a, Ponto b);

// Calcula a distancia total de uma rota ciclica
double distancia_total(
    const Individuo *individuo,
    const Ponto *pontos,
    int quantidade_pontos
);

// Calcula a aptidao do individuo
double calcular_fitness(
    const Individuo *individuo,
    const Ponto *pontos,
    int quantidade_pontos
);

#endif
