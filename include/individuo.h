
#ifndef INDIVIDUO_H
#define INDIVIDUO_H

typedef struct {
    int *rota;
    int quantidade;
} Individuo;


Individuo *criar_individuo(int quantidade);


int inicializar_rota(Individuo *individuo);


int validar_rota(const Individuo *individuo);


int copiar_individuo(
    Individuo *destino,
    const Individuo *origem
);


void imprimir_rota(const Individuo *individuo);


void liberar_individuo(Individuo *individuo);

#endif
