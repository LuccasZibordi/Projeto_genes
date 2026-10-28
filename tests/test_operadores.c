#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pontos.h"
#include "individuo.h"
#include "operadores.h"

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


static Individuo *criar_com_rota(const int *rota, int n) {
    Individuo *ind = criar_individuo(n);
    if (ind != NULL) {
        memcpy(ind->rota, rota, (size_t)n * sizeof(int));
    }
    return ind;
}


static void embaralhar(Individuo *ind) {
    for (int i = ind->quantidade - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int t = ind->rota[i];
        ind->rota[i] = ind->rota[j];
        ind->rota[j] = t;
    }
}

static int rotas_iguais(const Individuo *a, const Individuo *b) {
    return memcmp(a->rota, b->rota,
                  (size_t)a->quantidade * sizeof(int)) == 0;
}




static void testar_selecao_torneio(void) {

    printf("Selecao por torneio\n");

    double fitness[10];
    for (int i = 0; i < 10; i++) {
        fitness[i] = 0.1 * (i + 1);
    }


    VERIFICAR(selecao_torneio(NULL, 10, 3) == -1, "fitness NULL");
    VERIFICAR(selecao_torneio(fitness, 0, 3) == -1, "populacao vazia");
    VERIFICAR(selecao_torneio(fitness, 10, 0) == -1, "torneio zero");


    VERIFICAR(selecao_torneio(fitness, 1, 5) == 0, "populacao unitaria");


    double ruim[3] = {0.5, 0.0 / 0.0, 0.2};
    int detectou = 0;
    for (int t = 0; t < 100; t++) {
        if (selecao_torneio(ruim, 3, 3) == -1) {
            detectou = 1;
        }
    }
    VERIFICAR(detectou, "detecta fitness NAN");


    int dentro = 1;
    for (int t = 0; t < 5000; t++) {
        int s = selecao_torneio(fitness, 10, 3);
        if (s < 0 || s >= 10) {
            dentro = 0;
        }
    }
    VERIFICAR(dentro, "indice sempre valido");



    int vezes_melhor = 0, vezes_pior = 0;
    int tentativas = 20000;
    for (int t = 0; t < tentativas; t++) {
        int s = selecao_torneio(fitness, 10, 3);
        if (s == 9) vezes_melhor++;
        if (s == 0) vezes_pior++;
    }
    VERIFICAR(vezes_melhor > tentativas * 0.20,
              "melhor e favorecido (k=3)");
    VERIFICAR(vezes_pior < tentativas * 0.02,
              "pior raramente vence (k=3)");


    int sempre_melhor = 0;
    for (int t = 0; t < 1000; t++) {
        if (selecao_torneio(fitness, 10, 100) == 9) {
            sempre_melhor++;
        }
    }
    VERIFICAR(sempre_melhor > 990, "torneio grande escolhe o melhor");
}




static void testar_cruzamento_ox(void) {

    printf("Cruzamento OX\n");

    int r1[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    int r2[8] = {7, 6, 5, 4, 3, 2, 1, 0};

    Individuo *p1 = criar_com_rota(r1, 8);
    Individuo *p2 = criar_com_rota(r2, 8);
    Individuo *f = criar_individuo(8);




    int esperado[8] = {6, 5, 2, 3, 4, 1, 0, 7};
    VERIFICAR(cruzamento_ox_pontos(p1, p2, f, 2, 4) == 1,
              "OX com cortes fixos retorna sucesso");
    VERIFICAR(memcmp(f->rota, esperado, sizeof(esperado)) == 0,
              "OX com cortes [2,4] gera o filho esperado");
    VERIFICAR(validar_rota(f), "filho do exemplo e permutacao valida");


    VERIFICAR(cruzamento_ox_pontos(p1, p2, f, 0, 7) == 1,
              "OX com trecho completo");
    VERIFICAR(rotas_iguais(f, p1), "trecho completo copia o pai1");


    VERIFICAR(cruzamento_ox_pontos(p1, p2, f, 3, 3) == 1,
              "OX com trecho unitario");
    VERIFICAR(validar_rota(f) && f->rota[3] == p1->rota[3],
              "trecho unitario valido e preserva a posicao 3");


    VERIFICAR(memcmp(p1->rota, r1, sizeof(r1)) == 0, "pai1 intacto");
    VERIFICAR(memcmp(p2->rota, r2, sizeof(r2)) == 0, "pai2 intacto");


    VERIFICAR(!cruzamento_ox_pontos(NULL, p2, f, 1, 2), "pai1 NULL");
    VERIFICAR(!cruzamento_ox_pontos(p1, NULL, f, 1, 2), "pai2 NULL");
    VERIFICAR(!cruzamento_ox_pontos(p1, p2, NULL, 1, 2), "filho NULL");
    VERIFICAR(!cruzamento_ox_pontos(p1, p2, f, -1, 2), "inicio < 0");
    VERIFICAR(!cruzamento_ox_pontos(p1, p2, f, 1, 8), "fim >= n");
    VERIFICAR(!cruzamento_ox_pontos(p1, p2, f, 5, 2), "inicio > fim");
    VERIFICAR(!cruzamento_ox_pontos(p1, p2, p1, 1, 2),
              "filho nao pode ser o pai1");


    Individuo *grande = criar_individuo(9);
    VERIFICAR(!cruzamento_ox(p1, grande, f), "pais de tamanhos diferentes");
    liberar_individuo(grande);


    int ruim[8] = {0, 1, 2, 3, 4, 5, 6, 6};
    Individuo *pi = criar_com_rota(ruim, 8);
    VERIFICAR(!cruzamento_ox(pi, p2, f), "pai invalido e rejeitado");
    liberar_individuo(pi);

    liberar_individuo(p1);
    liberar_individuo(p2);
    liberar_individuo(f);



    int tamanhos[] = {8, 9, 15, 50, 200};
    int todos_validos = 1;
    int cortes_ok = 1;

    for (int t = 0; t < 5; t++) {

        int n = tamanhos[t];
        Individuo *a = criar_individuo(n);
        Individuo *b = criar_individuo(n);
        Individuo *c = criar_individuo(n);

        for (int rep = 0; rep < 2000; rep++) {
            embaralhar(a);
            embaralhar(b);

            if (!cruzamento_ox(a, b, c) || !validar_rota(c)) {
                todos_validos = 0;
            }


            if (!validar_rota(a) || !validar_rota(b)) {
                cortes_ok = 0;
            }
        }

        liberar_individuo(a);
        liberar_individuo(b);
        liberar_individuo(c);
    }

    VERIFICAR(todos_validos, "10000 filhos aleatorios sao permutacoes validas");
    VERIFICAR(cortes_ok, "pais permanecem validos apos cruzamentos");


    Individuo *x = criar_individuo(12);
    Individuo *y = criar_individuo(12);
    Individuo *z = criar_individuo(12);
    embaralhar(x);
    copiar_individuo(y, x);
    int iguais = 1;
    for (int rep = 0; rep < 500; rep++) {
        cruzamento_ox(x, y, z);
        if (!rotas_iguais(z, x)) {
            iguais = 0;
        }
    }
    VERIFICAR(iguais, "pais identicos geram filho identico");
    liberar_individuo(x);
    liberar_individuo(y);
    liberar_individuo(z);
}




static void testar_mutacao_swap(void) {

    printf("Mutacao Swap\n");

    int base[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    Individuo *ind = criar_com_rota(base, 8);


    VERIFICAR(mutacao_swap_posicoes(ind, 1, 6) == 1, "swap fixo retorna sucesso");
    int esperado[8] = {0, 6, 2, 3, 4, 5, 1, 7};
    VERIFICAR(memcmp(ind->rota, esperado, sizeof(esperado)) == 0,
              "swap(1,6) gera a rota esperada");
    VERIFICAR(validar_rota(ind), "rota continua valida apos swap");


    memcpy(ind->rota, base, sizeof(base));
    VERIFICAR(mutacao_swap_posicoes(ind, 3, 3) == 1, "swap(i,i)");
    VERIFICAR(memcmp(ind->rota, base, sizeof(base)) == 0,
              "swap(i,i) nao altera a rota");


    VERIFICAR(!mutacao_swap_posicoes(NULL, 0, 1), "individuo NULL");
    VERIFICAR(!mutacao_swap_posicoes(ind, -1, 1), "posicao negativa");
    VERIFICAR(!mutacao_swap_posicoes(ind, 0, 8), "posicao >= n");
    VERIFICAR(!mutacao_swap(NULL), "mutacao_swap com NULL");



    Individuo *antes = criar_individuo(20);
    Individuo *mut = criar_individuo(20);
    int sempre_duas = 1;
    int sempre_valida = 1;

    for (int rep = 0; rep < 5000; rep++) {
        embaralhar(antes);
        copiar_individuo(mut, antes);

        if (!mutacao_swap(mut)) {
            sempre_valida = 0;
            continue;
        }

        int diferencas = 0;
        for (int i = 0; i < 20; i++) {
            if (antes->rota[i] != mut->rota[i]) {
                diferencas++;
            }
        }

        if (diferencas != 2) {
            sempre_duas = 0;
        }
        if (!validar_rota(mut)) {
            sempre_valida = 0;
        }
    }

    VERIFICAR(sempre_duas, "mutacao altera exatamente 2 posicoes");
    VERIFICAR(sempre_valida, "5000 mutacoes mantem permutacao valida");


    int visto[20] = {0};
    for (int rep = 0; rep < 5000; rep++) {
        for (int i = 0; i < 20; i++) mut->rota[i] = i;
        mutacao_swap(mut);
        for (int i = 0; i < 20; i++) {
            if (mut->rota[i] != i) visto[i] = 1;
        }
    }
    int cobertura = 1;
    for (int i = 0; i < 20; i++) {
        if (!visto[i]) cobertura = 0;
    }
    VERIFICAR(cobertura, "todas as posicoes participam de alguma troca");

    liberar_individuo(ind);
    liberar_individuo(antes);
    liberar_individuo(mut);
}




static void testar_taxas(void) {

    printf("Taxas de cruzamento e mutacao\n");

    Individuo *p1 = criar_individuo(10);
    Individuo *p2 = criar_individuo(10);
    Individuo *f = criar_individuo(10);
    int aplicado = -1;


    VERIFICAR(!aplicar_cruzamento(p1, p2, f, -0.1, &aplicado),
              "taxa de cruzamento negativa");
    VERIFICAR(!aplicar_cruzamento(p1, p2, f, 1.5, &aplicado),
              "taxa de cruzamento > 1");
    VERIFICAR(!aplicar_mutacao(p1, -0.1, &aplicado),
              "taxa de mutacao negativa");
    VERIFICAR(!aplicar_mutacao(p1, 1.5, &aplicado),
              "taxa de mutacao > 1");


    int sempre_clone = 1;
    int nunca_aplicou = 1;
    for (int rep = 0; rep < 1000; rep++) {
        embaralhar(p1);
        embaralhar(p2);
        if (!aplicar_cruzamento(p1, p2, f, 0.0, &aplicado)) {
            sempre_clone = 0;
        }
        if (aplicado != 0) nunca_aplicou = 0;
        if (!rotas_iguais(f, p1)) sempre_clone = 0;
    }
    VERIFICAR(sempre_clone, "taxa 0: filho e copia do pai1");
    VERIFICAR(nunca_aplicou, "taxa 0: cruzamento nunca ocorre");


    int sempre_aplicou = 1;
    int sempre_valido = 1;
    for (int rep = 0; rep < 1000; rep++) {
        embaralhar(p1);
        embaralhar(p2);
        if (!aplicar_cruzamento(p1, p2, f, 1.0, &aplicado)) {
            sempre_valido = 0;
        }
        if (aplicado != 1) sempre_aplicou = 0;
        if (!validar_rota(f)) sempre_valido = 0;
    }
    VERIFICAR(sempre_aplicou, "taxa 1: cruzamento sempre ocorre");
    VERIFICAR(sempre_valido, "taxa 1: filhos sempre validos");


    int ocorreu = 0, tentativas = 20000;
    for (int rep = 0; rep < tentativas; rep++) {
        aplicar_cruzamento(p1, p2, f, 0.8, &aplicado);
        ocorreu += aplicado;
        if (!validar_rota(f)) sempre_valido = 0;
    }
    double freq = (double)ocorreu / tentativas;
    VERIFICAR(freq > 0.77 && freq < 0.83,
              "taxa 0.8: frequencia observada proxima de 80%");
    VERIFICAR(sempre_valido, "taxa 0.8: filhos sempre validos");


    VERIFICAR(aplicar_cruzamento(p1, p2, f, 0.5, NULL) == 1,
              "aplicado == NULL e aceito (cruzamento)");


    Individuo *ind = criar_individuo(10);
    Individuo *copia = criar_individuo(10);
    int intacto = 1;
    for (int rep = 0; rep < 1000; rep++) {
        embaralhar(ind);
        copiar_individuo(copia, ind);
        if (!aplicar_mutacao(ind, 0.0, &aplicado)) intacto = 0;
        if (aplicado != 0 || !rotas_iguais(ind, copia)) intacto = 0;
    }
    VERIFICAR(intacto, "taxa 0: mutacao nunca ocorre");


    int sempre_mudou = 1;
    int valida = 1;
    for (int rep = 0; rep < 1000; rep++) {
        embaralhar(ind);
        copiar_individuo(copia, ind);
        if (!aplicar_mutacao(ind, 1.0, &aplicado)) valida = 0;
        if (aplicado != 1 || rotas_iguais(ind, copia)) sempre_mudou = 0;
        if (!validar_rota(ind)) valida = 0;
    }
    VERIFICAR(sempre_mudou, "taxa 1: mutacao sempre altera a rota");
    VERIFICAR(valida, "taxa 1: rota segue valida");


    int mutou = 0;
    for (int rep = 0; rep < tentativas; rep++) {
        aplicar_mutacao(ind, 0.05, &aplicado);
        mutou += aplicado;
        if (!validar_rota(ind)) valida = 0;
    }
    freq = (double)mutou / tentativas;
    VERIFICAR(freq > 0.04 && freq < 0.06,
              "taxa 0.05: frequencia observada proxima de 5%");
    VERIFICAR(valida, "taxa 0.05: rota segue valida");

    VERIFICAR(aplicar_mutacao(ind, 0.5, NULL) == 1,
              "aplicado == NULL e aceito (mutacao)");

    liberar_individuo(p1);
    liberar_individuo(p2);
    liberar_individuo(f);
    liberar_individuo(ind);
    liberar_individuo(copia);
}

int main(void) {

    configurar_semente(12345);

    testar_selecao_torneio();
    testar_cruzamento_ox();
    testar_mutacao_swap();
    testar_taxas();

    printf("\n%d verificacoes, %d falha(s)\n",
           total_testes, total_falhas);

    return total_falhas == 0 ? 0 : 1;
}
