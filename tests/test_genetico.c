#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "avaliacao.h"
#include "genetico.h"
#include "individuo.h"
#include "pontos.h"

static int total_testes = 0;
static int total_falhas = 0;

#define VERIFICAR(condicao, mensagem)                              \
    do {                                                           \
        total_testes++;                                            \
        if (!(condicao)) {                                         \
            total_falhas++;                                        \
            printf("  [FALHOU] %s (linha %d)\n",                   \
                   (mensagem), __LINE__);                          \
        }                                                          \
    } while (0)

#define EPS 1e-9

static ParametrosAG parametros_pequenos(void) {
    ParametrosAG p;
    parametros_padrao(&p);
    p.populacao = 20;
    p.geracoes_max = 20;
    p.estagnacao_max = 0;
    p.semente = 7;
    return p;
}

static Ponto *pontos_circulo(int n, double raio) {
    Ponto *pontos = malloc((size_t)n * sizeof(Ponto));
    if (pontos == NULL) return NULL;

    if (!gerar_pontos_circulares(pontos, n, 100.0, 100.0, raio)) {
        free(pontos);
        return NULL;
    }
    return pontos;
}

static int contar_linhas(const char *caminho) {
    FILE *f = fopen(caminho, "r");
    if (f == NULL) return -1;

    int linhas = 0;
    int c;

    while ((c = fgetc(f)) != EOF) {
        if (c == '\n') linhas++;
    }

    fclose(f);
    return linhas;
}

static void testar_parametros_invalidos(void) {
    printf("Parametros invalidos\n");

    const int n = 10;
    Ponto *pontos = pontos_circulo(n, 50.0);
    VERIFICAR(pontos != NULL, "pontos criados");
    if (pontos == NULL) return;

    ParametrosAG p = parametros_pequenos();

    VERIFICAR(executar_ag(NULL, n, &p) == NULL, "pontos NULL");
    VERIFICAR(executar_ag(pontos, 7, &p) == NULL, "menos de 8 pontos");
    VERIFICAR(executar_ag(pontos, n, NULL) == NULL, "parametros NULL");

    p.populacao = 1;
    VERIFICAR(executar_ag(pontos, n, &p) == NULL, "populacao < 2");
    p = parametros_pequenos();

    p.geracoes_max = 0;
    VERIFICAR(executar_ag(pontos, n, &p) == NULL, "geracoes_max = 0");
    p = parametros_pequenos();

    p.estagnacao_max = -1;
    VERIFICAR(executar_ag(pontos, n, &p) == NULL, "estagnacao negativa");
    p = parametros_pequenos();

    p.elite = -1;
    VERIFICAR(executar_ag(pontos, n, &p) == NULL, "elite negativo");
    p = parametros_pequenos();

    p.elite = p.populacao;
    VERIFICAR(executar_ag(pontos, n, &p) == NULL, "elite >= populacao");
    p = parametros_pequenos();

    p.tamanho_torneio = 0;
    VERIFICAR(executar_ag(pontos, n, &p) == NULL, "torneio zero");
    p = parametros_pequenos();

    p.taxa_cruzamento = 1.5;
    VERIFICAR(executar_ag(pontos, n, &p) == NULL, "taxa de cruzamento > 1");
    p = parametros_pequenos();

    p.taxa_mutacao = -0.1;
    VERIFICAR(executar_ag(pontos, n, &p) == NULL, "taxa de mutacao negativa");

    free(pontos);
}

static void testar_snapshots_consistentes(void) {
    printf("Snapshots\n");

    const int n = 12;
    Ponto *pontos = pontos_circulo(n, 50.0);
    VERIFICAR(pontos != NULL, "pontos criados");
    if (pontos == NULL) return;

    ParametrosAG p = parametros_pequenos();
    p.populacao = 20;
    p.geracoes_max = 50;
    p.estagnacao_max = 0;
    p.semente = 11;

    Resultado *r = executar_ag(pontos, n, &p);
    VERIFICAR(r != NULL, "execucao concluida");
    if (r == NULL) {
        free(pontos);
        return;
    }

    VERIFICAR(r->total_snapshots == 51,
              "uma epoca por geracao, mais a epoca 0");
    VERIFICAR(r->snapshots[0].epoca == 0, "primeira epoca e 0");
    VERIFICAR(fabs(r->distancia -
                   r->snapshots[r->total_snapshots - 1].distancia_global) < EPS,
              "distancia final igual ao melhor global");

    Individuo *tmp = criar_individuo(n);
    VERIFICAR(tmp != NULL, "individuo temporario");
    if (tmp == NULL) {
        liberar_resultado(r);
        free(pontos);
        return;
    }

    int epocas_ok = 1;
    int rotas_ok = 1;
    int monotonas = 1;
    int global_melhor = 1;

    for (int i = 0; i < r->total_snapshots; i++) {
        const Snapshot *s = &r->snapshots[i];

        if (s->epoca != i) epocas_ok = 0;

        memcpy(tmp->rota, s->rota, (size_t)n * sizeof(int));
        if (!validar_rota(tmp)) rotas_ok = 0;

        memcpy(tmp->rota, s->rota_global, (size_t)n * sizeof(int));
        if (!validar_rota(tmp)) rotas_ok = 0;

        if (s->distancia_global > s->distancia_epoca + EPS) {
            global_melhor = 0;
        }

        if (i > 0 &&
            s->distancia_global > r->snapshots[i - 1].distancia_global + EPS) {
            monotonas = 0;
        }
    }

    VERIFICAR(epocas_ok, "indice das epocas sequencial");
    VERIFICAR(rotas_ok, "todas as rotas sao permutacoes validas");
    VERIFICAR(monotonas, "melhor global nunca piora");
    VERIFICAR(global_melhor, "melhor global <= melhor da epoca");

    liberar_individuo(tmp);
    liberar_resultado(r);
    free(pontos);
}

static void testar_parada_max_geracoes(void) {
    printf("Parada por maximo de geracoes\n");

    Ponto *pontos = pontos_circulo(10, 50.0);
    VERIFICAR(pontos != NULL, "pontos criados");
    if (pontos == NULL) return;

    ParametrosAG p = parametros_pequenos();
    p.populacao = 10;
    p.geracoes_max = 3;
    p.estagnacao_max = 0;

    Resultado *r = executar_ag(pontos, 10, &p);
    VERIFICAR(r != NULL, "execucao concluida");
    if (r == NULL) {
        free(pontos);
        return;
    }

    VERIFICAR(r->total_snapshots == 4, "epocas 0, 1, 2 e 3");
    VERIFICAR(r->motivo == PARADA_MAX_GERACOES, "motivo = max_geracoes");

    liberar_resultado(r);
    free(pontos);
}

static void testar_parada_estagnacao(void) {
    printf("Parada por estagnacao\n");

    Ponto *pontos = pontos_circulo(8, 50.0);
    VERIFICAR(pontos != NULL, "pontos criados");
    if (pontos == NULL) return;

    ParametrosAG p = parametros_pequenos();
    p.populacao = 10;
    p.geracoes_max = 200;
    p.estagnacao_max = 5;
    p.semente = 3;

    Resultado *r = executar_ag(pontos, 8, &p);
    VERIFICAR(r != NULL, "execucao concluida");
    if (r == NULL) {
        free(pontos);
        return;
    }

    VERIFICAR(r->motivo == PARADA_ESTAGNACAO, "motivo = estagnacao");
    VERIFICAR(r->total_snapshots < 201, "parou antes do teto de geracoes");

    liberar_resultado(r);
    free(pontos);
}

static void testar_reprodutibilidade(void) {
    printf("Reprodutibilidade\n");

    Ponto *pontos = pontos_circulo(15, 50.0);
    VERIFICAR(pontos != NULL, "pontos criados");
    if (pontos == NULL) return;

    ParametrosAG p = parametros_pequenos();
    p.geracoes_max = 30;
    p.estagnacao_max = 10;
    p.semente = 2026;

    Resultado *a = executar_ag(pontos, 15, &p);
    Resultado *b = executar_ag(pontos, 15, &p);

    VERIFICAR(a != NULL && b != NULL, "duas execucoes concluidas");

    if (a != NULL && b != NULL) {
        VERIFICAR(fabs(a->distancia - b->distancia) < EPS,
                  "mesma distancia final");
        VERIFICAR(a->total_snapshots == b->total_snapshots,
                  "mesmo numero de epocas");
        VERIFICAR(memcmp(a->rota, b->rota,
                         (size_t)15 * sizeof(int)) == 0,
                  "mesma rota final");
    }

    liberar_resultado(a);
    liberar_resultado(b);
    free(pontos);
}

static void testar_circulo_otimo(void) {
    printf("Cenario circular\n");

    const int n = 20;
    const double raio = 50.0;

    Ponto *pontos = pontos_circulo(n, raio);
    VERIFICAR(pontos != NULL, "pontos criados");
    if (pontos == NULL) return;

    ParametrosAG p;
    parametros_padrao(&p);
    p.semente = 42;

    Resultado *r = executar_ag(pontos, n, &p);
    VERIFICAR(r != NULL, "execucao concluida");
    if (r == NULL) {
        free(pontos);
        return;
    }

    double otima = distancia_otima_circular(n, raio);
    double erro = fabs(r->distancia - otima) / otima;

    VERIFICAR(erro < 0.01, "erro menor que 1% do otimo teorico");
    VERIFICAR(r->distancia < r->snapshots[0].distancia_epoca,
              "melhorou em relacao a populacao inicial");

    liberar_resultado(r);
    free(pontos);
}

static void testar_aleatorio(void) {
    printf("Cenario aleatorio\n");

    const int n = 20;

    Ponto *pontos = malloc((size_t)n * sizeof(Ponto));
    VERIFICAR(pontos != NULL, "pontos alocados");
    if (pontos == NULL) return;

    configurar_semente(99);

    VERIFICAR(gerar_pontos_aleatorios(pontos, n, 0.0, 100.0, 0.0, 100.0) == 1,
              "pontos aleatorios gerados");

    ParametrosAG p;
    parametros_padrao(&p);
    p.semente = 99;

    Resultado *r = executar_ag(pontos, n, &p);
    VERIFICAR(r != NULL, "execucao concluida");
    if (r == NULL) {
        free(pontos);
        return;
    }

    VERIFICAR(r->distancia < r->snapshots[0].distancia_epoca,
              "a evolucao reduziu a distancia");
    VERIFICAR(r->total_snapshots >= 2, "pelo menos epocas 0 e 1");

    liberar_resultado(r);
    free(pontos);
}

static void testar_exportacao(void) {
    printf("Exportacao e comparacao\n");

    const char *csv = "/tmp/test_genetico_metricas.csv";
    const char *json = "/tmp/test_genetico_historico.json";

    Ponto *pontos = pontos_circulo(10, 50.0);
    VERIFICAR(pontos != NULL, "pontos criados");
    if (pontos == NULL) return;

    ParametrosAG p = parametros_pequenos();
    p.populacao = 10;
    p.geracoes_max = 10;
    p.estagnacao_max = 0;

    Resultado *r = executar_ag(pontos, 10, &p);
    VERIFICAR(r != NULL, "execucao concluida");
    if (r == NULL) {
        free(pontos);
        return;
    }

    VERIFICAR(salvar_metricas_csv(r, csv) == 1, "CSV gravado");
    VERIFICAR(salvar_historico_json(r, pontos, 10, json) == 1, "JSON gravado");

    VERIFICAR(contar_linhas(csv) == 1 + r->total_snapshots,
              "CSV tem cabecalho + uma linha por epoca");

    FILE *f = fopen(json, "r");
    VERIFICAR(f != NULL, "JSON pode ser reaberto");
    if (f != NULL) {
        int primeiro = fgetc(f);
        VERIFICAR(primeiro == '{', "JSON comeca com chave");
        fclose(f);
    }

    VERIFICAR(comparar_epocas_csv(csv, 0, r->total_snapshots - 1) == 1,
              "comparacao de epocas validas");
    VERIFICAR(comparar_epocas_csv(csv, 0, 9999) == 0,
              "epoca inexistente e rejeitada");

    remove(csv);
    remove(json);

    liberar_resultado(r);
    free(pontos);
}

int main(void) {
    printf("====================================\n");
    printf(" TESTES DO MOTOR EVOLUTIVO\n");
    printf("====================================\n");

    testar_parametros_invalidos();
    testar_snapshots_consistentes();
    testar_parada_max_geracoes();
    testar_parada_estagnacao();
    testar_reprodutibilidade();
    testar_circulo_otimo();
    testar_aleatorio();
    testar_exportacao();

    printf("\n%d verificacoes, %d falha(s)\n",
           total_testes, total_falhas);

    return total_falhas == 0 ? 0 : 1;
}
