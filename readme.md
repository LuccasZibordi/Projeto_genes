# Algoritmo genetico para o TSP

Algoritmo genetico que busca boas rotas para o Problema do Caixeiro Viajante
(TSP) em pontos 2D. O programa gera uma instancia, evolui uma populacao de
rotas e grava um snapshot por epoca em CSV e JSON.

## Compilar

```bash
make
```

## Rodar

```bash
./ag_tsp
```

Saida esperada:

```text
cenario    : circular
pontos     : 20
populacao  : 100
torneio    : 3
elite      : 2
cruzamento : 0.80
mutacao    : 0.10
semente    : 42
epocas     : 138
parada     : estagnacao
tempo      : 0.017 s
inicial    : 1027.818666
final      : 312.868930
otima      : 312.868930
erro       : 0.000%
csv        : resultados/metricas_circular_42.csv
json       : resultados/historico_circular_42.json
```

## Cenarios

circular: pontos igualmente espacados em um circulo. Tem otimo conhecido,
D = 2nR*sen(pi/n), e o programa mostra as linhas otima e erro.

aleatorio: pontos uniformes em [0,100] x [0,100]. Nao tem otimo conhecido.

```bash
./ag_tsp -c circular
./ag_tsp -c aleatorio
```

## Flags

```text
-n N        numero de pontos (padrao 20, minimo 8)
-c C        cenario: circular ou aleatorio (padrao circular)
-p N        tamanho da populacao (padrao 100)
-g N        geracoes maximas (padrao 500)
-e N        individuos de elite (padrao 2)
-k N        tamanho do torneio (padrao 3)
-t X        taxa de cruzamento, 0 a 1 (padrao 0.80)
-m X        taxa de mutacao, 0 a 1 (padrao 0.10)
-s N        estagnacao maxima, 0 desativa (padrao 100)
-S N        semente do gerador aleatorio (padrao 42)
-r X        raio do circulo (padrao 50)
-o DIR      diretorio de saida (padrao resultados)
-C ARQ A B  compara as epocas A e B de um CSV ja gravado
-h          mostra a ajuda
```

## Como a parada funciona

epoca_final = min(geracoes_max, ultima_melhora + estagnacao_max)

snapshots = epoca_final + 1, incluindo a epoca 0.

O -g e o teto duro. O -s e a parada antecipada: quantas geracoes seguidas sem
bater o recorde global. O que acontecer primeiro vence.

Exemplo: ultima melhora na epoca 174 com -s 100 para na epoca 274.

## Comandos

Rodar o padrao:

```bash
./ag_tsp
```

Circular com mais pontos:

```bash
./ag_tsp -c circular -n 30
./ag_tsp -c circular -n 50
```

Aleatorio:

```bash
./ag_tsp -c aleatorio -n 20
./ag_tsp -c aleatorio -n 30
./ag_tsp -c aleatorio -n 50
```

Ensaio maior:

```bash
./ag_tsp -c circular -n 100 -p 200 -g 2000 -S 7
```

Rodar ate o teto, sem parada por estagnacao:

```bash
./ag_tsp -c aleatorio -n 30 -g 1000 -s 0
```

Mudar o raio do circulo:

```bash
./ag_tsp -c circular -n 20 -r 100
```

Repetir com varias sementes:

```bash
for s in 1 2 3 4 5; do
    ./ag_tsp -c aleatorio -n 30 -S $s
done
```

No cenario aleatorio a semente controla os pontos e o AG. Para comparar so o
AG, use o cenario circular, onde os pontos sao sempre os mesmos.

Separar cada experimento em uma pasta:

```bash
./ag_tsp -c circular -n 20  -S 42 -o resultados/n20
./ag_tsp -c circular -n 100 -S 42 -o resultados/n100
./ag_tsp -c aleatorio -n 30 -S 42 -o resultados/aleat30
```

Os arquivos se chamam metricas_cenario_semente.csv. O n nao entra no nome, entao
rodar com outro n e a mesma semente sobrescreve o resultado anterior.

Comparar duas epocas:

```bash
./ag_tsp -C resultados/metricas_circular_42.csv 0 50
```

Saida:

```text
Epoca 0: distancia 1027.818666 | melhor global 1027.818666
Epoca 50: distancia 312.868930 | melhor global 312.868930
Melhoria 0 -> 50: 714.949735 (69.560%)
```

Testes e limpeza:

```bash
make test
make clean
./ag_tsp -h
```

## Saidas

```text
resultados/metricas_cenario_semente.csv
resultados/historico_cenario_semente.json
```

O CSV tem uma linha por epoca. O JSON tem os pontos, os parametros, a semente,
o motivo da parada e as rotas de cada epoca.

## Colunas do CSV

epoca: indice da epoca, comeca em 0.

distancia_epoca: melhor distancia daquela epoca.

distancia_media: media da populacao na epoca.

distancia_pior: pior individuo da populacao.

distancia_global: melhor distancia ate aquela epoca.

tempo_segundos: tempo acumulado desde o inicio.

## O que o algoritmo faz

Cada individuo e uma permutacao das cidades, ou seja, uma rota. A distancia e a
soma das distancias entre cidades vizinhas, fechando o ciclo. O objetivo e
minimizar essa distancia. A aptidao usada e fitness = 1 / (1 + D).

Selecao: torneio de k individuos.

Cruzamento: Order Crossover (OX), que preserva a ordem relativa dos pais sem
duplicar cidades.

Mutacao: troca de duas posicoes, com probabilidade -m por filho.

Elitismo: os -e melhores passam intactos, entao o recorde nunca piora.

## Testes

```bash
make test
```

Cobrem parametros invalidos, snapshots, as duas formas de parada,
reprodutibilidade, os dois cenarios e exportacao/comparacao.

## Limitacao conhecida

Com mutacao swap, uma troca por filho, o cenario circular acima de 50 pontos
fica preso em otimo local.

n 30: resultado 313,59, otimo 313,59, erro 0%.

n 50: resultado 530,16, otimo 314,16, erro 69%.

n 100: resultado 674,08, otimo 314,11, erro 115%.

n 200: resultado 1502,55, otimo 314,16, erro 378%.

Para o bonus de 100 e 200 pontos, a melhoria recomendada e usar mutacao por
inversao (2-opt) em vez de swap.

## Estrutura

```text
include/    cabecalhos
src/        codigo-fonte
tests/      testes
resultados/ saidas geradas
Makefile
readme.md
```

## Compilacao manual

```bash
gcc -std=c11 -g -O1 -Wall -Wextra -Wpedantic -Iinclude \
    src/pontos.c src/individuo.c src/avaliacao.c \
    src/operadores.c src/genetico.c src/main.c \
    -lm -o ag_tsp
```

Teste de integracao do Integrante 1:

```bash
gcc -std=c11 -g -O1 -Wall -Wextra -Wpedantic \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -Iinclude \
    src/pontos.c src/individuo.c src/avaliacao.c \
    tests/test_integracao.c \
    -lm -o /tmp/teste_integracao && /tmp/teste_integracao
```
