#ifndef GENETICO_H
#define GENETICO_H

#include "pontos.h"

typedef struct {
    int populacao;  //individuos por geração
    int geracoes_max;   //teto de gerações
    int estagnacao_max;  //teto de gerações sem melhora
    int elite;  //individuos copiados sem alteração para a próxima geração
    int tamanho_torneio;    //k da seleção
    double taxa_cruzamento; //0..1
    double taxa_mutacao;    //0..1
    unsigned int semente;   //semente do RNG
} ParametrosAG;

typedef enum {
    PARADA_MAX_GERACOES = 0,
    PARADA_ESTAGNACAO = 1,
    PARADA_ERRO = 2
} MotivoParada;

typedef struct {
    int epoca;                // inclui a epoca 0
    int quantidade;           // n
    int *rota;                // melhor rota DA EPOCA
    int *rota_global;         // melhor rota GLOBAL ate a epoca
    double distancia_epoca;
    double distancia_global;
    double distancia_media;
    double distancia_pior;
    double tempo_segundos;    // acumulado
} Snapshot;

typedef struct {
    int quantidade;
    int *rota;                // melhor rota global final
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