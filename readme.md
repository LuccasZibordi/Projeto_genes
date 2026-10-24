# 🧬 Algoritmo Genético para o TSP

Trabalho de **Pesquisa Operacional**: um algoritmo genético que busca boas rotas
para o **Problema do Caixeiro Viajante (TSP)** em pontos 2D.

O programa gera uma instância, evolui uma população de rotas por várias gerações
e grava **um snapshot por época** em CSV e JSON.

```
     população inicial (época 0)
              │
              ▼
   ┌──────────────────────┐
   │ seleção   (torneio)  │
   │ cruzamento (OX)      │
   │ mutação   (swap)     │
   │ elitismo             │
   └──────────┬───────────┘
              ▼
      avalia → melhorou? → snapshot → parou?
              │
              └──── repete por geração
```

---

## ⚡ Início rápido

```bash
make          # compila o programa
./ag_tsp      # roda o cenário circular com 20 pontos
```

Saída esperada:

```
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
tempo      : 0.021 s
inicial    : 1027.818666
final      : 312.868930
otima      : 312.868930
erro       : 0.000%
csv        : resultados/metricas_circular_42.csv
json       : resultados/historico_circular_42.json
```

---

## 🗺️ Os dois cenários

| Cenário | Flag | Pontos | Ótimo conhecido? | Para que serve |
|---|---|---|---|---|
| **Circular** | `-c circular` (padrão) | igualmente espaçados num círculo | ✅ `D* = 2nR·sen(π/n)` | benchmark; mostra o erro % |
| **Aleatório** | `-c aleatorio` | uniformes em `[0,100] × [0,100]` | ❌ | instância sem resposta pronta |

```bash
./ag_tsp -c circular     # benchmark (padrão)
./ag_tsp -c aleatorio    # instância aleatória
```

No circular aparecem as linhas `otima` e `erro`. No aleatório elas não existem,
porque não há ótimo conhecido para comparar.

---

## 🎛️ Todas as flags

| Flag | Nome | Padrão | O que faz |
|---|---|---|---|
| `-n N` | pontos | `20` | número de cidades (mínimo **8**) |
| `-c C` | cenário | `circular` | `circular` ou `aleatorio` |
| `-p N` | população | `100` | indivíduos por geração |
| `-g N` | gerações | `500` | teto de gerações |
| `-e N` | elite | `2` | melhores copiados intactos para a próxima geração |
| `-k N` | torneio | `3` | competidores sorteados em cada seleção |
| `-t X` | cruzamento | `0.80` | probabilidade de cruzamento (0 a 1) |
| `-m X` | mutação | `0.10` | probabilidade de mutação por filho (0 a 1) |
| `-s N` | estagnação | `100` | para após N gerações sem melhorar (`0` desativa) |
| `-S N` | semente | `42` | semente do gerador aleatório |
| `-r X` | raio | `50` | raio do círculo (só no cenário circular) |
| `-o DIR` | saída | `resultados` | pasta onde grava CSV e JSON (criada se não existir) |
| `-C ARQ A B` | comparar | — | compara as épocas **A** e **B** de um CSV já gravado |
| `-h` | ajuda | — | mostra todas as flags no terminal |

### Como a parada funciona

```
epoca_final = min(geracoes_max, ultima_melhora + estagnacao_max)
snapshots   = epoca_final + 1        (inclui a época 0)
```

- `-g` é o **teto duro**.
- `-s` é a **parada antecipada**: quantas gerações seguidas sem bater o recorde.
- O que acontecer primeiro vence.

Exemplo: última melhora na época 174, `-s 100` → para na 274.

---

## 📋 Receitas de comandos

### Rodar

```bash
# padrão: circular, 20 pontos
./ag_tsp

# circular com mais pontos
./ag_tsp -c circular -n 30
./ag_tsp -c circular -n 50

# aleatório
./ag_tsp -c aleatorio -n 20
./ag_tsp -c aleatorio -n 30
./ag_tsp -c aleatorio -n 50

# ensaio maior (mais população e gerações)
./ag_tsp -c circular -n 100 -p 200 -g 2000 -S 7

# forçar rodar até o teto, ignorando a estagnação
./ag_tsp -c aleatorio -n 30 -g 1000 -s 0

# mudar o raio do círculo
./ag_tsp -c circular -n 20 -r 100
```

### Repetir com sementes diferentes

```bash
for s in 1 2 3 4 5; do
    ./ag_tsp -c aleatorio -n 30 -S $s
done
```

> ⚠️ No cenário aleatório, a semente controla **os pontos e o AG**.
> Para comparar só o AG, use o cenário circular (os pontos são sempre os mesmos).

### Separar cada experimento numa pasta

Os nomes dos arquivos são `metricas_<cenario>_<semente>.csv`. O `n` **não** entra
no nome, então rodar com outro `n` e a mesma semente **sobrescreve** o resultado.
Use `-o` para evitar:

```bash
./ag_tsp -c circular -n 20  -S 42 -o resultados/n20
./ag_tsp -c circular -n 100 -S 42 -o resultados/n100
./ag_tsp -c aleatorio -n 30 -S 42 -o resultados/aleat30
```

### Comparar duas épocas

```bash
./ag_tsp -C resultados/metricas_circular_42.csv 0 50
```

Saída:

```
Epoca 0: distancia 1027.818666 | melhor global 1027.818666
Epoca 50: distancia 312.868930 | melhor global 312.868930
Melhoria 0 -> 50: 714.949735 (69.560%)
```

### Testes e limpeza

```bash
make test     # roda os testes do motor (53 verificações)
make clean    # apaga os binários
./ag_tsp -h   # lista as flags
```

---

## 🧠 O que o algoritmo faz

| Conceito | No TSP |
|---|---|
| Gene | índice de uma cidade (`0..n-1`) |
| Cromossomo | uma permutação = uma rota |
| Aptidão | `fitness = 1 / (1 + D)` |
| Distância | `D = Σ d(P[rota[i]], P[rota[(i+1) % n]])` |
| Objetivo | **minimizar** `D` |

- **Seleção:** torneio de `k` indivíduos (padrão 3).
- **Cruzamento:** Order Crossover (OX), que preserva a ordem relativa dos pais
  sem gerar cidades duplicadas.
- **Mutação:** swap de duas posições, com probabilidade `-m` por filho.
- **Elitismo:** os `-e` melhores passam intactos — o recorde nunca piora.
- **Snapshots:** cada época guarda a melhor rota da época e a melhor global,
  além de média, pior e tempo acumulado.

---

## 📊 Arquivos gerados

```
resultados/
├── metricas_<cenario>_<semente>.csv    # uma linha por época
└── historico_<cenario>_<semente>.json  # pontos + parâmetros + rotas
```

### Colunas do CSV

| Coluna | Significado |
|---|---|
| `epoca` | índice da época (começa em 0) |
| `distancia_epoca` | melhor distância **daquela** época |
| `distancia_media` | média da população na época |
| `distancia_pior` | pior indivíduo da população |
| `distancia_global` | melhor distância **até aquela** época |
| `tempo_segundos` | tempo acumulado desde o início |

### JSON

Contém os pontos da instância, os parâmetros, a semente, o motivo da parada e,
para cada época, as rotas `rota` (da época) e `rota_global` (recorde até ali).

---

## 🧪 Testes

```bash
make test
```

Cobrem: parâmetros inválidos, snapshots, as duas paradas, reprodutibilidade,
cenário circular, cenário aleatório e exportação/comparação.

---

## ⚠️ Limitação conhecida

Com mutação **swap** (uma troca por filho), o cenário circular acima de ~50
pontos fica preso em ótimo local:

| n | resultado | ótimo | erro |
|---|---|---|---|
| 30 | 313,59 | 313,59 | 0% |
| 50 | 530,16 | 314,16 | 69% |
| 100 | 674,08 | 314,11 | 115% |
| 200 | 1.502,55 | 314,16 | 378% |

Para o bônus de 100/200 pontos, a melhoria recomendada é usar **mutação por
inversão (2-opt)** em vez de swap.

---

## 📁 Estrutura do projeto

```
Projeto_genes/
├── include/
│   ├── pontos.h
│   ├── individuo.h
│   ├── avaliacao.h
│   ├── operadores.h
│   └── genetico.h
├── src/
│   ├── pontos.c
│   ├── individuo.c
│   ├── avaliacao.c
│   ├── operadores.c
│   ├── genetico.c        ← motor evolutivo
│   └── main.c            ← CLI
├── tests/
│   ├── test_genetico.c
│   └── ...
├── resultados/
├── Makefile
└── readme.md
```

---

## 🔧 Compilação manual (sem Makefile)

```bash
gcc -std=c11 -g -O1 -Wall -Wextra -Wpedantic -Iinclude \
    src/pontos.c src/individuo.c src/avaliacao.c \
    src/operadores.c src/genetico.c src/main.c \
    -lm -o ag_tsp
```

### Teste de integração do Integrante 1

```bash
gcc -std=c11 -g -O1 -Wall -Wextra -Wpedantic \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -Iinclude \
    src/pontos.c src/individuo.c src/avaliacao.c \
    tests/test_integracao.c \
    -lm -o /tmp/teste_integracao && /tmp/teste_integracao
```
