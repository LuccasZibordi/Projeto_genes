#include <stdio.h>
#include <stdlib.h>

#include "pontos.h"
#include "individuo.h"

int main(void) {

    // Criando oito pontos manualmente
    Ponto pontos[8];

    pontos[0] = criar_ponto(10.0, 20.0);
    pontos[1] = criar_ponto(30.0, 40.0);
    pontos[2] = criar_ponto(50.0, 10.0);
    pontos[3] = criar_ponto(70.0, 80.0);
    pontos[4] = criar_ponto(20.0, 60.0);
    pontos[5] = criar_ponto(90.0, 30.0);
    pontos[6] = criar_ponto(40.0, 90.0);
    pontos[7] = criar_ponto(80.0, 50.0);

    printf("=== PONTOS ===\n");

    for (int i = 0; i < 8; i++) {
        printf(
            "Cidade %d: (%.2f, %.2f)\n",
            i,
            pontos[i].x,
            pontos[i].y
        );
    }

    // Criando um individuo
    Individuo *individuo = criar_individuo(8);

    if (individuo == NULL) {
        printf("Erro ao criar individuo.\n");
        return EXIT_FAILURE;
    }

    if (!inicializar_rota(individuo)) {
        printf("Erro ao inicializar rota.\n");
        liberar_individuo(individuo);
        return EXIT_FAILURE;
    }

    printf("\n=== ROTA ===\n");

    for (int i = 0; i < individuo->quantidade; i++) {
        printf("%d -> ", individuo->rota[i]);
    }

    // Retorno a cidade inicial
    printf("%d\n", individuo->rota[0]);

    // Liberando memoria
    liberar_individuo(individuo);

    printf("\nTeste concluido.\n");

    return EXIT_SUCCESS;
}