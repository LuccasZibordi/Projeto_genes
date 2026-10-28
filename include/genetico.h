#ifndef GENETICO_H
#define GENETICO_H

#include "pontos.h"

typedef struct {
    int populacao;
    int geracoes_max;
    int estagnacao_max;
    int elite;
    int tamanho_torneio;
    double taxa_cruzamento;
    double taxa_mutacao;
    unsigned int semente;
} ParametrosAG;

typedef enum {
    PARADA_MAX_GERACOES = 0,
    PARADA_ESTAGNACAO = 1,
    PARADA_ERRO = 2
} MotivoParada;

typedef struct {
    int epoca;
    int quantidade;
    int *rota;
    int *rota_global;
    double distancia_epoca;
    double distancia_global;
    double distancia_media;
    double distancia_pior;
    double tempo_segundos;
} Snapshot;

typedef struct {
    int quantidade;
    int *rota;
    double distancia;
    Snapshot *snapshots;
    int total_snapshots;
    MotivoParada motivo;
    ParametrosAG parametros;
    double tempo_total;
} Resultado;

void parametros_padrao(ParametrosAG *p);

Resultado *executar_ag(
    const Ponto *pontos,
    int quantidade,
    const ParametrosAG *p
);

void liberar_resultado(Resultado *r);

const char *nome_motivo(MotivoParada m);

int salvar_metricas_csv(const Resultado *r, const char *caminho);
int salvar_historico_json(const Resultado *r, const Ponto *pontos,
                          int quantidade, const char *caminho);
int comparar_epocas_csv(const char *caminho, int epoca_a, int epoca_b);

#endif