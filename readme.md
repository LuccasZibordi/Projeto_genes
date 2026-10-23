como executar:

gcc -std=c11 -g -O1 -Wall -Wextra -Wpedantic \
    -fsanitize=address,undefined \
    -fno-omit-frame-pointer \
    -Iinclude \
    src/pontos.c src/individuo.c src/avaliacao.c \
    tests/test_integracao.c \
    -lm -o teste_integracao_san

## Algoritmo genetico (motor evolutivo)

Compilar e rodar:

```bash
make
./ag_tsp                                        # circular, 20 pontos
./ag_tsp -c aleatorio -n 30                     # pontos aleatorios
./ag_tsp -n 100 -p 200 -g 2000 -S 7             # outro ensaio
./ag_tsp -C resultados/metricas_circular_42.csv 0 50   # compara epocas
```

Todas as opcoes: `./ag_tsp -h`.

Os resultados vao para `resultados/`, um CSV por epoca e um JSON com os
pontos, parametros, semente, motivo da parada e as rotas dos snapshots:

```
resultados/metricas_<cenario>_<semente>.csv
resultados/historico_<cenario>_<semente>.json
```

Testes do motor:

```bash
make test
```

Limitacao conhecida: com mutacao swap (uma troca por filho), o cenario
circular acima de ~50 pontos converge para um otimo local. Exemplo: n=100
estabiliza em ~674 contra o otimo teorico de ~314, mesmo com 20.000
geracoes. Para o bonus de 100/200 pontos, a melhoria recomendada e usar
mutacao por inversao (2-opt) em vez de swap.
