#ifndef OPERADORES_H
#define OPERADORES_H

#include "individuo.h"




int selecao_torneio(
    const double *fitness,
    int tamanho_populacao,
    int tamanho_torneio
);




int cruzamento_ox(
    const Individuo *pai1,
    const Individuo *pai2,
    Individuo *filho
);



int cruzamento_ox_pontos(
    const Individuo *pai1,
    const Individuo *pai2,
    Individuo *filho,
    int inicio,
    int fim
);



int mutacao_swap(Individuo *individuo);


int mutacao_swap_posicoes(Individuo *individuo, int i, int j);





int aplicar_cruzamento(
    const Individuo *pai1,
    const Individuo *pai2,
    Individuo *filho,
    double taxa_cruzamento,
    int *aplicado
);




int aplicar_mutacao(
    Individuo *individuo,
    double taxa_mutacao,
    int *aplicado
);

#endif
