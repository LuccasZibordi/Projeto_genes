#include <math.h>
#include <stddef.h>

#include "avaliacao.h"

// Calcula a distancia euclidiana entre dois pontos
double distancia_euclidiana(Ponto a, Ponto b) {

    double dx = b.x - a.x;
    double dy = b.y - a.y;

    return hypot(dx, dy);
}

// Calcula o comprimento completo de uma rota ciclica
double distancia_total(
    const Individuo *individuo,
    const Ponto *pontos,
    int quantidade_pontos
) {
    // Verifica os parametros
    if (individuo == NULL || pontos == NULL ||
        individuo->rota == NULL ||
        quantidade_pontos < 8 ||
        individuo->quantidade != quantidade_pontos) {
        return NAN;
    }

    // Garante que a rota e uma permutacao valida
    if (!validar_rota(individuo)) {
        return NAN;
    }

    double total = 0.0;
    int n = individuo->quantidade;

    for (int i = 0; i < n; i++) {

        int atual = individuo->rota[i];

        // O operador % faz a ultima cidade voltar
        // para a primeira cidade da rota
        int proximo = individuo->rota[(i + 1) % n];

        Ponto a = pontos[atual];
        Ponto b = pontos[proximo];

        double distancia = distancia_euclidiana(a, b);

        if (!isfinite(distancia)) {
            return NAN;
        }

        total += distancia;

        if (!isfinite(total)) {
            return NAN;
        }
    }

    return total;
}

// Calcula a aptidao do individuo
double calcular_fitness(
    const Individuo *individuo,
    const Ponto *pontos,
    int quantidade_pontos
) {
    double distancia = distancia_total(
        individuo, pontos, quantidade_pontos
    );

    if (!isfinite(distancia) || distancia < 0.0) {
        return NAN;
    }

    return 1.0 / (1.0 + distancia);
}
