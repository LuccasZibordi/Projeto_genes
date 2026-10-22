#include <math.h>
#include <stdlib.h>
#include <stddef.h>

#include "operadores.h" 

// Sorteia um real no intervalo [0, 1)
static double aleatorio_unitario(void) {
    return (double)rand() / ((double)RAND_MAX + 1.0);
}

// Sorteia um inteiro no intervalo [minimo, maximo]
static int inteiro_aleatorio(int minimo, int maximo) {

    if (minimo >= maximo) {
        return minimo;
    }

    long long faixa = (long long)maximo - (long long)minimo + 1;
    int valor = minimo + (int)(aleatorio_unitario() * (double)faixa);

    // Protecao contra arredondamento
    if (valor > maximo) {
        valor = maximo;
    }

    return valor;
}

// Verifica se a taxa e um numero finito entre 0 e 1
static int taxa_valida(double taxa) {
    return isfinite(taxa) && taxa >= 0.0 && taxa <= 1.0;
}

int selecao_torneio(
    const double *fitness,
    int tamanho_populacao,
    int tamanho_torneio
) {

    if (fitness == NULL || tamanho_populacao < 1 ||
        tamanho_torneio < 1) {
        return -1;
    }

    int melhor = -1;

    for (int i = 0; i < tamanho_torneio; i++) {

        int candidato = inteiro_aleatorio(0, tamanho_populacao - 1);

        // Fitness invalido (NAN/infinito) indica individuo invalido
        if (!isfinite(fitness[candidato])) {
            return -1;
        }

        if (melhor == -1 || fitness[candidato] > fitness[melhor]) {
            melhor = candidato;
        }
    }

    return melhor;
}

int cruzamento_ox_pontos(
    const Individuo *pai1,
    const Individuo *pai2,
    Individuo *filho,
    int inicio,
    int fim
) {

    // Verifica os parametros
    if (pai1 == NULL || pai2 == NULL || filho == NULL ||
        pai1->rota == NULL || pai2->rota == NULL ||
        filho->rota == NULL) {
        return 0;
    }

    int n = pai1->quantidade;

    if (n < 8 || pai2->quantidade != n || filho->quantidade != n) {
        return 0;
    }

    // O filho nao pode ser o mesmo objeto que um dos pais,
    // senao a escrita destruiria os dados que ainda sao lidos
    if (filho == pai1 || filho == pai2) {
        return 0;
    }

    if (inicio < 0 || fim >= n || inicio > fim) {
        return 0;
    }

    // Os pais precisam ser permutacoes validas
    if (!validar_rota(pai1) || !validar_rota(pai2)) {
        return 0;
    }

    // Marca as cidades que ja estao no filho
    unsigned char *usado = calloc((size_t)n, sizeof(unsigned char));

    if (usado == NULL) {
        return 0;
    }

    // 1) Copia o trecho [inicio, fim] do pai1
    for (int i = inicio; i <= fim; i++) {
        filho->rota[i] = pai1->rota[i];
        usado[pai1->rota[i]] = 1;
    }

    // 2) Preenche o restante com a ordem do pai2, comecando
    //    logo apos o segundo corte e voltando ao inicio (ciclico)
    int destino = (fim + 1) % n;

    for (int k = 0; k < n; k++) {

        int cidade = pai2->rota[(fim + 1 + k) % n];

        if (!usado[cidade]) {
            filho->rota[destino] = cidade;
            usado[cidade] = 1;
            destino = (destino + 1) % n;
        }
    }

    free(usado);

    return 1;
}

int cruzamento_ox(
    const Individuo *pai1,
    const Individuo *pai2,
    Individuo *filho
) {

    if (pai1 == NULL || pai1->quantidade < 8) {
        return 0;
    }

    int n = pai1->quantidade;

    int a = inteiro_aleatorio(0, n - 1);
    int b = inteiro_aleatorio(0, n - 1);

    // Garante inicio <= fim
    if (a > b) {
        int temp = a;
        a = b;
        b = temp;
    }

    return cruzamento_ox_pontos(pai1, pai2, filho, a, b);
}

int mutacao_swap_posicoes(Individuo *individuo, int i, int j) {

    if (individuo == NULL || individuo->rota == NULL ||
        individuo->quantidade < 8) {
        return 0;
    }

    int n = individuo->quantidade;

    if (i < 0 || i >= n || j < 0 || j >= n) {
        return 0;
    }

    int temp = individuo->rota[i];
    individuo->rota[i] = individuo->rota[j];
    individuo->rota[j] = temp;

    return 1;
}

int mutacao_swap(Individuo *individuo) {

    if (individuo == NULL || individuo->rota == NULL ||
        individuo->quantidade < 8) {
        return 0;
    }

    int n = individuo->quantidade;

    int i = inteiro_aleatorio(0, n - 1);

    // Sorteia j entre as demais posicoes (garante i != j)
    int j = inteiro_aleatorio(0, n - 2);

    if (j >= i) {
        j++;
    }

    return mutacao_swap_posicoes(individuo, i, j);
}

int aplicar_cruzamento(
    const Individuo *pai1,
    const Individuo *pai2,
    Individuo *filho,
    double taxa_cruzamento,
    int *aplicado
) {

    if (aplicado != NULL) {
        *aplicado = 0;
    }

    if (!taxa_valida(taxa_cruzamento)) {
        return 0;
    }

    if (aleatorio_unitario() < taxa_cruzamento) {

        if (!cruzamento_ox(pai1, pai2, filho)) {
            return 0;
        }

        if (aplicado != NULL) {
            *aplicado = 1;
        }

        return 1;
    }

    // Sem cruzamento: o filho e um clone do pai1
    if (!validar_rota(pai1)) {
        return 0;
    }

    return copiar_individuo(filho, pai1);
}

int aplicar_mutacao(
    Individuo *individuo,
    double taxa_mutacao,
    int *aplicado
) {

    if (aplicado != NULL) {
        *aplicado = 0;
    }

    if (!taxa_valida(taxa_mutacao) || !validar_rota(individuo)) {
        return 0;
    }

    if (aleatorio_unitario() < taxa_mutacao) {

        if (!mutacao_swap(individuo)) {
            return 0;
        }

        if (aplicado != NULL) {
            *aplicado = 1;
        }
    }

    return 1;
}
