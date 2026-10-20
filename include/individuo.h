
#ifndef INDIVIDUO_H
#define INDIVIDUO_H

typedef struct {
    int *rota;
    int quantidade;
} Individuo;

// Cria um individuo e aloca memoria
Individuo *criar_individuo(int quantidade);

// Inicializa a rota com 0, 1, 2, ..., n-1
int inicializar_rota(Individuo *individuo);

// Verifica se uma rota e uma permutacao valida
int validar_rota(const Individuo *individuo);

// Copia os dados de um individuo para outro
int copiar_individuo(
    Individuo *destino,
    const Individuo *origem
);

// Exibe a rota incluindo o retorno ao inicio
void imprimir_rota(const Individuo *individuo);

// Libera a memoria utilizada
void liberar_individuo(Individuo *individuo);

#endif
