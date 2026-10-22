#ifndef OPERADORES_H
#define OPERADORES_H

#include "individuo.h"

// Selecao por torneio.
// Sorteia "tamanho_torneio" competidores (com reposicao) e devolve o
// indice do que tem maior fitness. Retorna -1 em caso de erro.
int selecao_torneio(
    const double *fitness,
    int tamanho_populacao,
    int tamanho_torneio
);

// Order Crossover (OX) com pontos de corte sorteados.
// O filho recebe o trecho [inicio, fim] do pai1 e as demais cidades
// na ordem em que aparecem no pai2. Retorna 1 em sucesso, 0 em erro.
int cruzamento_ox(
    const Individuo *pai1,
    const Individuo *pai2,
    Individuo *filho
);

// Mesmo que cruzamento_ox, mas com pontos de corte fornecidos
// (0 <= inicio <= fim < n). Util para testes deterministicos.
int cruzamento_ox_pontos(
    const Individuo *pai1,
    const Individuo *pai2,
    Individuo *filho,
    int inicio,
    int fim
);

// Mutacao Swap: troca duas posicoes distintas sorteadas da rota.
// Retorna 1 em sucesso, 0 em erro.
int mutacao_swap(Individuo *individuo);

// Mesmo que mutacao_swap, mas com posicoes fornecidas.
int mutacao_swap_posicoes(Individuo *individuo, int i, int j);

// Aplica o cruzamento OX com probabilidade "taxa_cruzamento" (0 a 1).
// Se o cruzamento nao ocorrer, o filho e uma copia do pai1.
// Se "aplicado" nao for NULL, recebe 1 se o cruzamento ocorreu, 0 se nao.
// Retorna 1 em sucesso, 0 em erro.
int aplicar_cruzamento(
    const Individuo *pai1,
    const Individuo *pai2,
    Individuo *filho,
    double taxa_cruzamento,
    int *aplicado
);

// Aplica a mutacao Swap com probabilidade "taxa_mutacao" (0 a 1).
// Se "aplicado" nao for NULL, recebe 1 se houve mutacao, 0 se nao.
// Retorna 1 em sucesso, 0 em erro.
int aplicar_mutacao(
    Individuo *individuo,
    double taxa_mutacao,
    int *aplicado
);

#endif
