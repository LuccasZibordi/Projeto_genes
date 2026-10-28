#define _POSIX_C_SOURCE 200809L

#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "avaliacao.h"
#include "genetico.h"
#include "individuo.h"
#include "operadores.h"
#include "pontos.h"





static double agora_segundos(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

void parametros_padrao(ParametrosAG *p) {
    if (p == NULL) return;

    p->populacao = 100;
    p->geracoes_max = 500;
    p->estagnacao_max = 100;
    p->elite = 2;
    p->tamanho_torneio = 3;
    p->taxa_cruzamento = 0.80;
    p->taxa_mutacao = 0.10;
    p->semente = 42;
}

static int parametros_validos(const ParametrosAG *p) {
    if (p == NULL) return 0;
    if (p->populacao < 2) return 0;
    if (p->geracoes_max < 1) return 0;
    if (p->estagnacao_max < 0) return 0;
    if (p->elite < 0 || p->elite >= p->populacao) return 0;
    if (p->tamanho_torneio < 1) return 0;

    if (!isfinite(p->taxa_cruzamento) ||
        p->taxa_cruzamento < 0.0 || p->taxa_cruzamento > 1.0) return 0;

    if (!isfinite(p->taxa_mutacao) ||
        p->taxa_mutacao < 0.0 || p->taxa_mutacao > 1.0) return 0;

    return 1;
}






static void embaralhar_rota(int *rota, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = rota[i];
        rota[i] = rota[j];
        rota[j] = temp;
    }
}

static void liberar_populacao(Individuo **pop, int tamanho) {
    if (pop == NULL) return;

    for (int i = 0; i < tamanho; i++) {
        liberar_individuo(pop[i]);
    }
    free(pop);
}


static Individuo **criar_populacao(int n, int tamanho) {
    Individuo **pop = malloc((size_t)tamanho * sizeof(Individuo *));
    if (pop == NULL) return NULL;

    for (int i = 0; i < tamanho; i++) {
        pop[i] = criar_individuo(n);
        if (pop[i] == NULL) {
            liberar_populacao(pop, i);
            return NULL;
        }
        embaralhar_rota(pop[i]->rota, n);
    }
    return pop;
}






static int avaliar_populacao(
    Individuo **pop,
    double *distancias,
    double *fitness,
    const Ponto *pontos,
    int n,
    int tamanho
) {
    for (int i = 0; i < tamanho; i++) {
        double d = distancia_total(pop[i], pontos, n);
        if (!isfinite(d)) return 0;

        distancias[i] = d;
        fitness[i] = 1.0 / (1.0 + d);
    }
    return 1;
}

static int melhor_indice(const double *fitness, int tamanho) {
    int melhor = 0;

    for (int i = 1; i < tamanho; i++) {
        if (fitness[i] > fitness[melhor]) melhor = i;
    }
    return melhor;
}

static void estatisticas(
    const double *distancias,
    int tamanho,
    double *media,
    double *pior
) {
    double soma = 0.0;
    double maior = distancias[0];

    for (int i = 0; i < tamanho; i++) {
        soma += distancias[i];
        if (distancias[i] > maior) maior = distancias[i];
    }

    *media = soma / (double)tamanho;
    *pior = maior;
}


static void top_k_indices(
    const double *fitness,
    int tamanho,
    int k,
    int *saida
) {
    for (int e = 0; e < k; e++) {
        int melhor = -1;

        for (int i = 0; i < tamanho; i++) {
            int ja_escolhido = 0;

            for (int j = 0; j < e; j++) {
                if (saida[j] == i) {
                    ja_escolhido = 1;
                    break;
                }
            }

            if (!ja_escolhido &&
                (melhor == -1 || fitness[i] > fitness[melhor])) {
                melhor = i;
            }
        }

        saida[e] = melhor;
    }
}





static int registrar_snapshot(
    Resultado *r,
    int epoca,
    Individuo **pop,
    const double *distancias,
    int tamanho,
    int melhor,
    const int *rota_global,
    double distancia_global,
    double tempo
) {
    Snapshot *s = &r->snapshots[r->total_snapshots];
    int n = pop[melhor]->quantidade;

    s->rota = malloc((size_t)n * sizeof(int));
    s->rota_global = malloc((size_t)n * sizeof(int));

    if (s->rota == NULL || s->rota_global == NULL) {
        free(s->rota);
        free(s->rota_global);
        s->rota = NULL;
        s->rota_global = NULL;
        return 0;
    }

    memcpy(s->rota, pop[melhor]->rota, (size_t)n * sizeof(int));
    memcpy(s->rota_global, rota_global, (size_t)n * sizeof(int));

    s->epoca = epoca;
    s->quantidade = n;
    s->distancia_epoca = distancias[melhor];
    s->distancia_global = distancia_global;
    estatisticas(distancias, tamanho,
                 &s->distancia_media, &s->distancia_pior);
    s->tempo_segundos = tempo;

    r->total_snapshots++;
    return 1;
}





Resultado *executar_ag(
    const Ponto *pontos,
    int quantidade,
    const ParametrosAG *p
) {
    if (pontos == NULL || quantidade < 8 || !parametros_validos(p)) {
        return NULL;
    }

    int n = quantidade;
    int tamanho = p->populacao;
    int elite = p->elite;


    Resultado *r = NULL;
    Individuo **atual = NULL;
    Individuo **proxima = NULL;
    double *distancias = NULL;
    double *fitness = NULL;
    int *elite_idx = NULL;
    int *melhor_global = NULL;
    double melhor_global_dist = 0.0;
    double inicio = 0.0;
    double tempo = 0.0;
    int estagnacao = 0;
    int epoca = 0;
    int m = 0;
    MotivoParada motivo = PARADA_MAX_GERACOES;

    r = calloc(1, sizeof(Resultado));
    if (r == NULL) return NULL;


    r->snapshots = calloc((size_t)p->geracoes_max + 1, sizeof(Snapshot));
    if (r->snapshots == NULL) goto falha;

    configurar_semente(p->semente);

    atual = criar_populacao(n, tamanho);
    proxima = criar_populacao(n, tamanho);
    if (atual == NULL || proxima == NULL) goto falha;

    distancias = malloc((size_t)tamanho * sizeof(double));
    fitness = malloc((size_t)tamanho * sizeof(double));
    if (distancias == NULL || fitness == NULL) goto falha;

    if (elite > 0) {
        elite_idx = malloc((size_t)elite * sizeof(int));
        if (elite_idx == NULL) goto falha;
    }

    melhor_global = malloc((size_t)n * sizeof(int));
    if (melhor_global == NULL) goto falha;

    if (!avaliar_populacao(atual, distancias, fitness, pontos, n, tamanho)) {
        goto falha;
    }

    m = melhor_indice(fitness, tamanho);
    melhor_global_dist = distancias[m];
    memcpy(melhor_global, atual[m]->rota, (size_t)n * sizeof(int));

    inicio = agora_segundos();


    if (!registrar_snapshot(r, 0, atual, distancias, tamanho, m,
                            melhor_global, melhor_global_dist,
                            agora_segundos() - inicio)) {
        goto falha;
    }

    for (epoca = 1; epoca <= p->geracoes_max; epoca++) {

        if (elite > 0) {
            top_k_indices(fitness, tamanho, elite, elite_idx);
        }


        for (int i = 0; i < tamanho; i++) {
            int pai1 = selecao_torneio(fitness, tamanho, p->tamanho_torneio);
            int pai2 = selecao_torneio(fitness, tamanho, p->tamanho_torneio);

            if (pai1 < 0 || pai2 < 0) {
                motivo = PARADA_ERRO;
                goto fim;
            }

            if (!aplicar_cruzamento(atual[pai1], atual[pai2], proxima[i],
                                    p->taxa_cruzamento, NULL)) {
                motivo = PARADA_ERRO;
                goto fim;
            }

            if (!aplicar_mutacao(proxima[i], p->taxa_mutacao, NULL)) {
                motivo = PARADA_ERRO;
                goto fim;
            }
        }


        for (int e = 0; e < elite; e++) {
            if (!copiar_individuo(proxima[e], atual[elite_idx[e]])) {
                motivo = PARADA_ERRO;
                goto fim;
            }
        }

        Individuo **troca = atual;
        atual = proxima;
        proxima = troca;

        if (!avaliar_populacao(atual, distancias, fitness,
                               pontos, n, tamanho)) {
            motivo = PARADA_ERRO;
            goto fim;
        }

        m = melhor_indice(fitness, tamanho);


        if (distancias[m] < melhor_global_dist) {
            melhor_global_dist = distancias[m];
            memcpy(melhor_global, atual[m]->rota, (size_t)n * sizeof(int));
            estagnacao = 0;
        } else {
            estagnacao++;
        }


        tempo = agora_segundos() - inicio;

        if (!registrar_snapshot(r, epoca, atual, distancias, tamanho, m,
                                melhor_global, melhor_global_dist, tempo)) {
            goto falha;
        }


        if (p->estagnacao_max > 0 && estagnacao >= p->estagnacao_max) {
            motivo = PARADA_ESTAGNACAO;
            break;
        }
    }

fim:
    r->quantidade = n;
    r->rota = melhor_global;
    r->distancia = melhor_global_dist;
    r->motivo = motivo;
    r->parametros = *p;
    r->tempo_total = agora_segundos() - inicio;

    free(elite_idx);
    free(distancias);
    free(fitness);
    liberar_populacao(atual, tamanho);
    liberar_populacao(proxima, tamanho);
    return r;

falha:
    free(elite_idx);
    free(distancias);
    free(fitness);
    free(melhor_global);
    liberar_populacao(atual, tamanho);
    liberar_populacao(proxima, tamanho);
    liberar_resultado(r);
    return NULL;
}

void liberar_resultado(Resultado *r) {
    if (r == NULL) return;

    for (int i = 0; i < r->total_snapshots; i++) {
        free(r->snapshots[i].rota);
        free(r->snapshots[i].rota_global);
    }

    free(r->snapshots);
    free(r->rota);
    free(r);
}





const char *nome_motivo(MotivoParada m) {
    switch (m) {
        case PARADA_ESTAGNACAO: return "estagnacao";
        case PARADA_ERRO:       return "erro";
        default:                return "max_geracoes";
    }
}

int salvar_metricas_csv(const Resultado *r, const char *caminho) {
    if (r == NULL || caminho == NULL) return 0;

    FILE *f = fopen(caminho, "w");
    if (f == NULL) return 0;

    fprintf(f, "epoca,distancia_epoca,distancia_media,distancia_pior,"
               "distancia_global,tempo_segundos\n");

    for (int i = 0; i < r->total_snapshots; i++) {
        const Snapshot *s = &r->snapshots[i];

        fprintf(f, "%d,%.10f,%.10f,%.10f,%.10f,%.6f\n",
                s->epoca,
                s->distancia_epoca,
                s->distancia_media,
                s->distancia_pior,
                s->distancia_global,
                s->tempo_segundos);
    }

    return fclose(f) == 0;
}

int salvar_historico_json(const Resultado *r, const Ponto *pontos,
                          int quantidade, const char *caminho) {
    if (r == NULL || pontos == NULL || quantidade < 8 || caminho == NULL) {
        return 0;
    }

    FILE *f = fopen(caminho, "w");
    if (f == NULL) return 0;

    fprintf(f, "{\n");
    fprintf(f, "  \"semente\": %u,\n", r->parametros.semente);
    fprintf(f, "  \"motivo_parada\": \"%s\",\n", nome_motivo(r->motivo));
    fprintf(f, "  \"melhor_distancia\": %.10f,\n", r->distancia);
    fprintf(f, "  \"tempo_total\": %.6f,\n", r->tempo_total);

    fprintf(f, "  \"parametros\": {\n");
    fprintf(f, "    \"populacao\": %d,\n", r->parametros.populacao);
    fprintf(f, "    \"geracoes_max\": %d,\n", r->parametros.geracoes_max);
    fprintf(f, "    \"estagnacao_max\": %d,\n", r->parametros.estagnacao_max);
    fprintf(f, "    \"elite\": %d,\n", r->parametros.elite);
    fprintf(f, "    \"tamanho_torneio\": %d,\n", r->parametros.tamanho_torneio);
    fprintf(f, "    \"taxa_cruzamento\": %.6f,\n", r->parametros.taxa_cruzamento);
    fprintf(f, "    \"taxa_mutacao\": %.6f\n", r->parametros.taxa_mutacao);
    fprintf(f, "  },\n");

    fprintf(f, "  \"pontos\": [");
    for (int i = 0; i < quantidade; i++) {
        fprintf(f, "%s[%.10f, %.10f]", i > 0 ? ", " : "",
                pontos[i].x, pontos[i].y);
    }
    fprintf(f, "],\n");

    fprintf(f, "  \"snapshots\": [\n");
    for (int i = 0; i < r->total_snapshots; i++) {
        const Snapshot *s = &r->snapshots[i];

        fprintf(f, "    {\"epoca\": %d, \"distancia_epoca\": %.10f, "
                   "\"distancia_media\": %.10f, \"distancia_pior\": %.10f, "
                   "\"distancia_global\": %.10f, \"tempo_segundos\": %.6f, "
                   "\"rota\": [",
                s->epoca, s->distancia_epoca, s->distancia_media,
                s->distancia_pior, s->distancia_global, s->tempo_segundos);

        for (int k = 0; k < s->quantidade; k++) {
            fprintf(f, "%s%d", k > 0 ? ", " : "", s->rota[k]);
        }

        fprintf(f, "], \"rota_global\": [");
        for (int k = 0; k < s->quantidade; k++) {
            fprintf(f, "%s%d", k > 0 ? ", " : "", s->rota_global[k]);
        }

        fprintf(f, "]}%s\n", (i + 1 < r->total_snapshots) ? "," : "");
    }
    fprintf(f, "  ]\n}\n");

    return fclose(f) == 0;
}





int comparar_epocas_csv(const char *caminho, int epoca_a, int epoca_b) {
    if (caminho == NULL || epoca_a < 0 || epoca_b < 0) return 0;

    FILE *f = fopen(caminho, "r");
    if (f == NULL) return 0;

    char linha[256];
    double dist_a = NAN;
    double dist_b = NAN;
    double global_a = NAN;
    double global_b = NAN;


    if (fgets(linha, sizeof linha, f) == NULL) {
        fclose(f);
        return 0;
    }

    while (fgets(linha, sizeof linha, f) != NULL) {
        int epoca;
        double de, dm, dp, dg, t;

        if (sscanf(linha, "%d,%lf,%lf,%lf,%lf,%lf",
                   &epoca, &de, &dm, &dp, &dg, &t) != 6) {
            continue;
        }

        if (epoca == epoca_a) {
            dist_a = de;
            global_a = dg;
        }
        if (epoca == epoca_b) {
            dist_b = de;
            global_b = dg;
        }
    }
    fclose(f);

    if (!isfinite(dist_a) || !isfinite(dist_b)) return 0;

    double melhoria = dist_a - dist_b;
    double percentual = dist_a > 0.0 ? 100.0 * melhoria / dist_a : 0.0;

    printf("Epoca %d: distancia %.6f | melhor global %.6f\n",
           epoca_a, dist_a, global_a);
    printf("Epoca %d: distancia %.6f | melhor global %.6f\n",
           epoca_b, dist_b, global_b);
    printf("Melhoria %d -> %d: %.6f (%.3f%%)\n",
           epoca_a, epoca_b, melhoria, percentual);

    return 1;
}