#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "genetico.h"
#include "pontos.h"

#define RAIO_PADRAO   50.0
#define CENTRO_PADRAO 100.0

static void uso(const char *programa) {
    printf(
        "uso: %s [opcoes]\n"
        "  -n N        numero de pontos (>= 8, padrao 20)\n"
        "  -c CENARIO  aleatorio | circular (padrao circular)\n"
        "  -p N        tamanho da populacao (padrao 100)\n"
        "  -g N        geracoes maximas (padrao 500)\n"
        "  -e N        individuos de elite (padrao 2)\n"
        "  -k N        tamanho do torneio (padrao 3)\n"
        "  -t X        taxa de cruzamento 0..1 (padrao 0.80)\n"
        "  -m X        taxa de mutacao 0..1 (padrao 0.10)\n"
        "  -s N        estagnacao maxima, 0 desativa (padrao 100)\n"
        "  -S N        semente do RNG (padrao 42)\n"
        "  -r X        raio do circulo (padrao 50)\n"
        "  -o DIR      diretorio de saida (padrao resultados)\n"
        "  -C ARQ A B  compara as epocas A e B do CSV informado\n"
        "  -h          mostra esta ajuda\n",
        programa);
}

// mkdir sem reclamar se o diretorio ja existe
static int garantir_diretorio(const char *caminho) {
    if (mkdir(caminho, 0777) == 0 || errno == EEXIST) {
        return 1;
    }
    return 0;
}

int main(int argc, char **argv) {
    int n = 20;
    int cenario = 0;                 // 0 = circular, 1 = aleatorio
    int comparar = 0;
    const char *arquivo_comparar = NULL;
    const char *saida = "resultados";
    double raio = RAIO_PADRAO;

    ParametrosAG p;
    parametros_padrao(&p);

    int opcao;

    while ((opcao = getopt(argc, argv, "n:c:p:g:e:k:t:m:s:S:r:o:C:h")) != -1) {
        switch (opcao) {
            case 'n': n = atoi(optarg); break;
            case 'p': p.populacao = atoi(optarg); break;
            case 'g': p.geracoes_max = atoi(optarg); break;
            case 'e': p.elite = atoi(optarg); break;
            case 'k': p.tamanho_torneio = atoi(optarg); break;
            case 't': p.taxa_cruzamento = atof(optarg); break;
            case 'm': p.taxa_mutacao = atof(optarg); break;
            case 's': p.estagnacao_max = atoi(optarg); break;
            case 'S': p.semente = (unsigned int)atoi(optarg); break;
            case 'r': raio = atof(optarg); break;
            case 'o': saida = optarg; break;

            case 'c':
                if (strcmp(optarg, "aleatorio") == 0) {
                    cenario = 1;
                } else if (strcmp(optarg, "circular") == 0) {
                    cenario = 0;
                } else {
                    fprintf(stderr, "cenario invalido: %s\n", optarg);
                    return EXIT_FAILURE;
                }
                break;

            case 'C':
                comparar = 1;
                arquivo_comparar = optarg;
                break;

            case 'h':
                uso(argv[0]);
                return EXIT_SUCCESS;

            default:
                uso(argv[0]);
                return EXIT_FAILURE;
        }
    }

    // Modo comparacao: le um CSV ja gravado, nao executa o AG.
    if (comparar) {
        if (argc - optind != 2) {
            fprintf(stderr, "uso: %s -C ARQUIVO.csv EPOCA_A EPOCA_B\n", argv[0]);
            return EXIT_FAILURE;
        }

        int epoca_a = atoi(argv[optind]);
        int epoca_b = atoi(argv[optind + 1]);

        if (!comparar_epocas_csv(arquivo_comparar, epoca_a, epoca_b)) {
            fprintf(stderr, "nao foi possivel comparar as epocas\n");
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

    if (n < 8) {
        fprintf(stderr, "o numero de pontos deve ser >= 8\n");
        return EXIT_FAILURE;
    }

    Ponto *pontos = malloc((size_t)n * sizeof(Ponto));
    if (pontos == NULL) {
        fprintf(stderr, "sem memoria para os pontos\n");
        return EXIT_FAILURE;
    }

    configurar_semente(p.semente);

    int gerou;
    if (cenario == 1) {
        gerou = gerar_pontos_aleatorios(pontos, n, 0.0, 100.0, 0.0, 100.0);
    } else {
        gerou = gerar_pontos_circulares(pontos, n, CENTRO_PADRAO, CENTRO_PADRAO, raio);
    }

    if (!gerou) {
        fprintf(stderr, "erro ao gerar os pontos\n");
        free(pontos);
        return EXIT_FAILURE;
    }

    Resultado *r = executar_ag(pontos, n, &p);
    if (r == NULL) {
        fprintf(stderr, "parametros invalidos para o AG\n");
        free(pontos);
        return EXIT_FAILURE;
    }

    const char *nome_cenario = (cenario == 1) ? "aleatorio" : "circular";
    char caminho_csv[512];
    char caminho_json[512];

    if (!garantir_diretorio(saida)) {
        fprintf(stderr, "nao foi possivel criar o diretorio %s\n", saida);
        free(pontos);
        liberar_resultado(r);
        return EXIT_FAILURE;
    }

    snprintf(caminho_csv, sizeof caminho_csv, "%s/metricas_%s_%u.csv",
             saida, nome_cenario, p.semente);
    snprintf(caminho_json, sizeof caminho_json, "%s/historico_%s_%u.json",
             saida, nome_cenario, p.semente);

    printf("cenario    : %s\n", nome_cenario);
    printf("pontos     : %d\n", n);
    printf("populacao  : %d\n", p.populacao);
    printf("torneio    : %d\n", p.tamanho_torneio);
    printf("elite      : %d\n", p.elite);
    printf("cruzamento : %.2f\n", p.taxa_cruzamento);
    printf("mutacao    : %.2f\n", p.taxa_mutacao);
    printf("semente    : %u\n", p.semente);
    printf("epocas     : %d\n", r->total_snapshots);
    printf("parada     : %s\n", nome_motivo(r->motivo));
    printf("tempo      : %.3f s\n", r->tempo_total);
    printf("inicial    : %.6f\n", r->snapshots[0].distancia_epoca);
    printf("final      : %.6f\n", r->distancia);

    if (cenario == 0) {
        double otima = distancia_otima_circular(n, raio);
        printf("otima      : %.6f\n", otima);
        printf("erro       : %.3f%%\n", 100.0 * (r->distancia - otima) / otima);
    }

    int salvou_csv = salvar_metricas_csv(r, caminho_csv);
    int salvou_json = salvar_historico_json(r, pontos, n, caminho_json);

    printf("csv        : %s%s\n", caminho_csv,
           salvou_csv ? "" : " (falhou)");
    printf("json       : %s%s\n", caminho_json,
           salvou_json ? "" : " (falhou)");

    free(pontos);
    liberar_resultado(r);

    return (salvou_csv && salvou_json) ? EXIT_SUCCESS : EXIT_FAILURE;
}
