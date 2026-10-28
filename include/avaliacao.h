
#ifndef AVALIACAO_H
#define AVALIACAO_H

#include "pontos.h"
#include "individuo.h"


double distancia_euclidiana(Ponto a, Ponto b);


double distancia_total(
    const Individuo *individuo,
    const Ponto *pontos,
    int quantidade_pontos
);


double calcular_fitness(
    const Individuo *individuo,
    const Ponto *pontos,
    int quantidade_pontos
);

#endif
