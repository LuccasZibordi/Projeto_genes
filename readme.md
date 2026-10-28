# Algoritmo genético para o Problema do Caixeiro Viajante

Projeto em C para buscar rotas curtas entre pontos em duas dimensões, com scripts Python para analisar e visualizar a evolução. Cada rota visita todos os pontos uma vez e retorna ao ponto inicial. O custo é a soma das distâncias euclidianas, incluindo o retorno.

O objetivo é encontrar boas soluções por busca heurística. O algoritmo não garante o ótimo global. No cenário circular, comparamos o resultado com uma solução ótima conhecida.

## 1. Preparar e executar

Os comandos abaixo consideram Linux, GCC, Make e Python 3. No computador usado na revisão, o projeto fica em:

```bash
cd /home/luccas/Projects/Projeto_genes
```

Em outro computador, entre na pasta em que clonou o projeto.

### Compilar o algoritmo em C

```bash
make
make test
```

`make` gera `ag_tsp`. `make test` executa os testes de `tests/test_genetico.c`, que incluem parâmetros inválidos, snapshots, parada, reprodutibilidade, os dois cenários e exportação. Esse alvo não executa todos os arquivos de teste da pasta `tests/`.

### Preparar o Python para os gráficos

Neste computador, o ambiente `.venv` já foi preparado. Para usá-lo:

```bash
source .venv/bin/activate
```

Em uma instalação nova, crie o ambiente e instale as dependências:

```bash
python3 -m venv .venv
source .venv/bin/activate
python -m pip install matplotlib numpy pillow PyQt6
```

Se você usa `uv`, a alternativa é:

```bash
uv venv .venv
uv pip install --python .venv/bin/python matplotlib numpy pillow PyQt6
source .venv/bin/activate
```

O programa em C não depende do Python. Para os gráficos, usamos Matplotlib e NumPy; para GIFs, Pillow; para abrir a janela interativa, PyQt6.

### Gerar os dois cenários

```bash
./ag_tsp -c circular -n 20 -S 42 -o resultados/apresentacao_circular
./ag_tsp -c aleatorio -n 30 -S 42 -o resultados/apresentacao_aleatorio
```

Cada execução cria um CSV de métricas e um JSON com pontos, parâmetros e rotas por época. O diretório `resultados/` já existe no repositório. Para caminhos mais profundos, crie os diretórios pais com `mkdir -p` antes de executar o programa em C.

### Abrir a visualização por época

Com o ambiente Python ativado, execute no terminal da sua sessão gráfica:

```bash
MPLBACKEND=QtAgg python visualizacao.py \
  resultados/apresentacao_circular/historico_circular_42.json --interativo
```

Para o cenário aleatório:

```bash
MPLBACKEND=QtAgg python visualizacao.py \
  resultados/apresentacao_aleatorio/historico_aleatorio_42.json --interativo
```

Na janela, você acompanha a rota, a distância e o gráfico de evolução. Arraste o slider para selecionar qualquer época. As setas esquerda/direita avançam uma época; cima/baixo avançam dez.

A visualização reproduz o histórico após o término do AG. Ela não acompanha o processo em tempo real enquanto o programa em C executa.

Para visualizar os históricos que já acompanham o repositório, use:

```bash
MPLBACKEND=QtAgg python visualizacao.py resultados/historico_circular_42.json --interativo
MPLBACKEND=QtAgg python visualizacao.py resultados/historico_aleatorio_42.json --interativo
```

### Salvar gráficos e animações

```bash
python visualizacao.py \
  resultados/apresentacao_circular/historico_circular_42.json \
  resultados/apresentacao_aleatorio/historico_aleatorio_42.json \
  --saida graficos/apresentacao --gif
```

Esse comando salva a curva de convergência, sua versão em escala logarítmica, uma grade de rotas em épocas intermediárias, a rota final e um GIF de cada cenário.

O GIF usa uma amostra de épocas. Para examinar todas as épocas, use o slider ou os arquivos JSON/CSV.

Escolher épocas específicas para as figuras:

```bash
python visualizacao.py resultados/historico_circular_42.json \
  --epocas 0 10 30 50 100 --saida graficos/epocas
```

Escolha épocas que existam no histórico. Para uma época além do término, o visualizador usa o último snapshot disponível.

Comparar os cenários:

```bash
python visualizacao.py --comparar \
  resultados/apresentacao_aleatorio/historico_aleatorio_42.json \
  resultados/apresentacao_circular/historico_circular_42.json \
  --saida graficos/comparacao
```

A comparação salva `comparacao_cenarios.png`. A normalização pela distância final mostra a evolução relativa de cada execução; ela não mede o erro em relação ao ótimo do cenário aleatório.

### Se a janela não abrir

Confira se ativou `.venv` e execute o comando em um terminal do desktop. `MPLBACKEND=QtAgg` seleciona o backend Qt instalado com PyQt6, sem depender do Tkinter.

Em servidor ou terminal sem sessão gráfica, gere arquivos:

```bash
MPLBACKEND=Agg python visualizacao.py resultados/historico_circular_42.json \
  --saida graficos/sem_tela
```

O backend `Agg` não abre uma janela interativa.

## 2. Cenários e distância

- **Aleatório:** coordenadas uniformes no quadrado `[0,100] × [0,100]`. Neste projeto, não calculamos uma solução ótima de referência para essa instância.
- **Circular:** pontos igualmente espaçados em uma circunferência, com centro padrão `(100,100)` e raio padrão `50`. A rota ótima visita os pontos na ordem da circunferência, em qualquer um dos sentidos.

Para pontos `a` e `b`, a distância é:

```text
d(a,b) = sqrt((bx - ax)² + (by - ay)²)
D(rota) = soma de d(rota[i], rota[(i+1) mod n]), para i de 0 a n-1
```

No cenário circular, cada lado do polígono regular mede `2R × sen(π/n)`. Portanto:

```text
D_ótima = 2 × n × R × sen(π/n)
erro_percentual = 100 × (D_encontrada - D_ótima) / D_ótima
```

A quantidade de pontos é configurável por `-n`, com mínimo de 8 e limite prático de memória e tempo de execução.

## 3. Representação do cromossomo e dos genes

Cada indivíduo representa uma rota completa por uma permutação dos índices das cidades. Cada gene é o índice de uma cidade; sua posição no cromossomo define a ordem de visita.

Exemplo com oito cidades:

```text
Cromossomo: [0, 3, 1, 7, 4, 2, 6, 5]
Percurso:    0 → 3 → 1 → 7 → 4 → 2 → 6 → 5 → 0
```

Essa representação corresponde à restrição do problema: visitar cada cidade uma vez. Não precisamos codificar coordenadas no cromossomo, pois elas são fixas durante a execução. O retorno ao início entra no cálculo da distância, sem repetir a primeira cidade no vetor.

A população inicial contém permutações embaralhadas por Fisher–Yates. Os operadores preservam a validade das permutações, e o código verifica índices fora da faixa e cidades repetidas.

## 4. Aptidão e seleção

A função de aptidão é:

```text
fitness = 1 / (1 + D)
```

Quanto menor a distância `D`, maior a aptidão. O termo `1 + D` evita divisão por zero e mantém a aptidão positiva para distâncias válidas. Como a função é estritamente decrescente em `D`, maximizar a aptidão equivale a minimizar a distância.

Usamos seleção por torneio: sorteamos `k` candidatos com reposição e selecionamos aquele com maior aptidão. Repetimos o processo para escolher cada pai.

O padrão `k = 3` aplica pressão seletiva em favor de rotas curtas sem restringir a reprodução apenas ao melhor indivíduo. Torneios maiores aumentam essa pressão e podem acelerar a perda de diversidade. O torneio dispensa normalizar a soma das aptidões da população.

## 5. Cruzamento OX

Usamos **Order Crossover (OX)** com probabilidade padrão de `0,80` por filho:

1. Sorteamos dois pontos de corte.
2. Copiamos o trecho entre os cortes do primeiro pai.
3. Percorremos o segundo pai a partir da posição seguinte ao segundo corte, em ordem cíclica.
4. Preenchemos as posições restantes com as cidades que ainda não aparecem no filho.

Escolhemos OX porque ele preserva um segmento de um pai e a ordem relativa das cidades do outro, produzindo uma permutação sem repetir ou omitir cidades. Um cruzamento que apenas concatenasse partes das rotas poderia gerar percursos inválidos.

A taxa de 80% faz da recombinação a operação predominante. Quando o cruzamento não ocorre, copiamos o primeiro pai; esse filho ainda pode sofrer mutação. Essa taxa é uma configuração inicial de projeto, sem demonstração de que seja ótima para todas as instâncias.

## 6. Mutação e escolha da taxa

Usamos **swap**: sorteamos duas posições distintas e trocamos suas cidades. A operação mantém a permutação válida e introduz novas ordens de visita com uma alteração simples.

A taxa padrão `0,10` significa **10% de chance por filho**, e não por gene. Quando a mutação ocorre, fazemos uma troca. O código aplica a reprodução aos 100 filhos e depois substitui dois deles pelos indivíduos de elite; assim, o número esperado de filhos mutados que permanecem na nova população é `98 × 0,10 = 9,8`, com os parâmetros padrão.

A justificativa para 10% é introduzir variação sem modificar a maioria dos filhos gerados pelo cruzamento. Uma taxa muito baixa pode facilitar a estagnação; uma taxa maior aumenta a exploração, mas também altera mais rotas potencialmente boas. O elitismo protege os melhores indivíduos dessa alteração.

Não realizamos nesta documentação um estudo controlado que estabeleça 10% como a melhor taxa. Para avaliar a escolha, compare, por exemplo, `0,05`, `0,10`, `0,20` e `0,50`, mantendo os demais parâmetros e repetindo as execuções com várias sementes. Registre qualidade final, tempo e variabilidade.

## 7. População, elitismo e parada

### População de 100 indivíduos

Adotamos 100 indivíduos como configuração inicial para os exemplos de 20 e 30 pontos. Isso permite manter múltiplas ordens de visita e executar as gerações com baixo custo nos exemplos pequenos. Aumentar a população oferece mais candidatos, mas exige mais avaliações e operações de reprodução por geração.

O tamanho não deriva de uma garantia teórica: mesmo para 20 pontos, a quantidade de rotas possíveis é muito maior que 100. A população fornece uma amostra para a busca. Para instâncias maiores, o script de bônus aumenta a população para 500. Uma comparação experimental ainda é necessária para afirmar qual tamanho oferece o melhor equilíbrio entre tempo e qualidade.

### Elitismo de dois indivíduos

Copiamos os dois indivíduos de maior aptidão para a próxima geração sem modificá-los. Com isso, a melhor distância da população não piora entre gerações. Preservar apenas uma pequena parcela deixa espaço para novos candidatos. A flag `-e` permite alterar ou desativar o elitismo.

### Critério de parada

A execução termina quando atingir o primeiro destes limites:

- **500 gerações:** orçamento máximo de evolução.
- **100 gerações consecutivas sem melhorar o recorde global:** parada por estagnação.

O teto limita o custo de uma execução. A janela de estagnação evita continuar indefinidamente sem progresso, mas concede várias tentativas de recombinação e mutação antes de interromper a busca. Os valores 500 e 100 são limites iniciais para os exemplos pequenos; não constituem prova de convergência.

Com estagnação ativada, a parada ocorre no primeiro limite alcançado. Se a última melhora acontecer na época 174 e não houver outra, `-s 100` encerra na época 274, desde que o teto permita. Com `-s 0`, apenas o teto de gerações limita a execução normal.

A época 0 representa a população inicial. Portanto, uma execução terminada na época 137 contém **138 snapshots**. A linha `epocas` impressa pelo executável informa a quantidade de snapshots, incluindo a época 0.

Parar por estagnação não comprova ótimo local nem global; significa apenas que a execução passou o número configurado de gerações sem uma distância menor. O código compara distâncias em ponto flutuante sem uma tolerância de melhora.

## 8. Parametrização completa

| Flag | Parâmetro | Padrão |
| --- | --- | --- |
| `-n N` | Número de pontos, mínimo 8 | 20 |
| `-c C` | `circular` ou `aleatorio` | circular |
| `-p N` | Tamanho da população, mínimo 2 | 100 |
| `-g N` | Máximo de gerações, mínimo 1 | 500 |
| `-e N` | Elite, de 0 até população menos 1 | 2 |
| `-k N` | Tamanho do torneio, mínimo 1 | 3 |
| `-t X` | Probabilidade de cruzamento, de 0 a 1 | 0.80 |
| `-m X` | Probabilidade de mutação por filho, de 0 a 1 | 0.10 |
| `-s N` | Gerações sem melhora; 0 desativa | 100 |
| `-S N` | Semente do gerador pseudoaleatório | 42 |
| `-r X` | Raio positivo do círculo | 50 |
| `-o DIR` | Diretório de saída | resultados |
| `-C ARQ A B` | Comparar épocas A e B de um CSV existente | desativado |
| `-h` | Mostrar ajuda | — |

Exemplo sem parada por estagnação:

```bash
./ag_tsp -c aleatorio -n 30 -p 200 -g 1000 -s 0 -m 0.20 \
  -S 42 -o resultados/aleatorio_p200
```

**Arquivos com o mesmo cenário e semente usam os mesmos nomes.** O número de pontos e os parâmetros não entram no nome. Separe os experimentos em diretórios para evitar sobrescrever resultados.

## 9. Avaliar o desempenho e acompanhar as soluções

Em cada época, registramos:

| Campo | Significado |
| --- | --- |
| `epoca` | Índice da geração, incluindo a população inicial |
| `distancia_epoca` | Distância do melhor indivíduo da geração |
| `distancia_media` | Média das distâncias da população |
| `distancia_pior` | Distância do pior indivíduo |
| `distancia_global` | Melhor distância encontrada até a época |
| `tempo_segundos` | Tempo acumulado da evolução |

O JSON também registra as coordenadas, os parâmetros, a semente, o motivo de parada, a melhor rota da época e a melhor rota global até aquela época.

Para apresentar cada cenário, mostre a rota inicial, algumas rotas intermediárias, a rota final e a curva de distância por geração. Use o slider para inspecionar qualquer época. Compare a distância inicial com a final:

```text
redução_percentual = 100 × (D_inicial - D_final) / D_inicial
```

No circular, acrescente o erro em relação ao ótimo analítico. No aleatório, a redução mede progresso em relação à melhor rota inicial; ela não informa quanto falta para atingir o ótimo.

Comparação numérica entre duas épocas:

```bash
./ag_tsp -C resultados/metricas_circular_42.csv 0 50
```

O cronômetro do C começa após a criação e a avaliação inicial da população e termina antes da exportação. Ele mede a evolução com o registro dos snapshots; não inclui compilação, preparação inicial, escrita dos arquivos ou geração dos gráficos.

### Resultados que acompanham o repositório

Os números abaixo vêm dos JSONs existentes em `resultados/`. Os tempos são os registrados nesses arquivos, não medições portáveis entre máquinas.

| Histórico | Pontos | Distância inicial | Distância final | Redução | Erro vs. ótimo | Última época | Tempo registrado |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `historico_circular_42.json` | 20 | 1027,818666 | 312,868930 | 69,56% | ≈ 0% | 137 | 0,017225 s |
| `historico_aleatorio_42.json` | 30 | 1271,151671 | 503,479946 | 60,39% | Não calculado | 274 | 0,061111 s |
| `historico_circular_7.json` | 100 | 5383,808120 | 841,607103 | 84,37% | 167,94% | 1261 | 0,739566 s |

Os dois primeiros usam os parâmetros padrão. O terceiro usa população 200, máximo de 2000 gerações, estagnação 100 e semente 7; os demais parâmetros permanecem nos padrões.

O circular de 20 pontos alcançou o ótimo. O teste de 100 pontos melhorou a rota inicial, mas terminou distante do ótimo. Uma grande redução em relação à rota inicial, por si só, não comprova boa aproximação do ótimo.

### Repetições e comparação de parâmetros

```bash
python experimentos.py sementes --sem-gif
```

Esse modo executa dez sementes por cenário, com 30 pontos no aleatório e 20 no circular, e salva gráficos e uma tabela em `graficos/`.

No cenário aleatório, mudar a semente altera tanto os pontos quanto a busca. Portanto, a variação mistura diferenças entre instâncias e diferenças da evolução. Para comparar configurações, repita o mesmo conjunto de sementes em cada configuração; no circular, os pontos permanecem iguais para o mesmo número de pontos e raio.

A reprodução exata da sequência pseudoaleatória também depende da implementação de `rand()` da biblioteca C. Não pressuponha resultados idênticos entre sistemas diferentes apenas por usar a mesma semente.

## 10. Bônus: cenário circular com muitos pontos

Para executar o experimento e gerar as evidências:

```bash
python experimentos.py bonus --n-bonus 100
```

Quando você não informa substituições, esse modo usa:

| Parâmetro | Valor no bônus |
| --- | ---: |
| População | 500 |
| Máximo de gerações | 20000 |
| Estagnação | 3000 |
| Mutação swap | 0.50 |
| Torneio | 5 |
| Cruzamento | 0.80 |
| Elite | 2 |
| Semente | 42 |

A configuração amplia a população e o orçamento de busca e aumenta a mutação para explorar mais rotas. O torneio maior aumenta a pressão seletiva. Essas alterações podem melhorar a busca, mas não garantem atingir o ótimo. A revisão de 25/10/2026 não executou essa configuração ampliada; os resultados da tabela anterior usam outra configuração.

O script grava o histórico em `resultados/bonus_n100/` e os gráficos e tempos intermediários em `graficos/bonus_n100/`. No terminal, informa quando a distância entra pela primeira vez nas faixas de até **10%, 5% e 1% acima do ótimo**, ou indica que não atingiu a faixa.

Para apresentar o tempo de convergência, declare o critério adotado, por exemplo: “primeira época com erro de até 1%”. Diferencie esse tempo do tempo total até a parada e do instante em que encontrou a melhor solução final. Se não atingir o critério, registre esse resultado.

No histórico antigo de 100 pontos, a melhor distância final já aparece, dentro de tolerância numérica, na época 1155, aos 0,679117 s. A execução termina aos 0,739566 s, sem chegar a 1% do ótimo. Esses valores não representam convergência ao ótimo.

Para inspecionar a nova execução:

```bash
MPLBACKEND=QtAgg python visualizacao.py \
  resultados/bonus_n100/historico_circular_42.json --interativo
```

Para testar outro tamanho ou alterar o orçamento:

```bash
python experimentos.py bonus --n-bonus 200 -p 500 -g 20000 -s 3000 -m 0.50
```

O custo aumenta com a população, a quantidade de pontos e as gerações. O armazenamento de todas as rotas também cresce com pontos × épocas.

## 11. Outros experimentos automatizados

```bash
python experimentos.py circular -n 20 --sem-gif
python experimentos.py aleatorio -n 30 --sem-gif
python experimentos.py escala --tamanhos 10 20 50 100
python experimentos.py tudo --sem-gif
```

Os modos `circular` e `aleatorio` geram relatórios dos respectivos cenários. `escala` compara tamanhos usando os padrões ampliados do bônus. `tudo` inclui os dois cenários, comparação, escala, bônus e repetições por sementes; por isso, pode levar mais tempo.

Após editar o código C, execute `make` antes dos experimentos. O script pode reutilizar um executável já existente.

## 12. Limitações e possíveis melhorias

A mutação swap pode apresentar estagnação com rotas ainda longas, como no histórico circular de 100 pontos. Uma melhoria a investigar é a mutação por inversão de segmento; outra é uma busca local 2-opt que aceite trocas de arestas que reduzam a distância. Inversão aleatória e busca 2-opt com aceitação por melhora são operações distintas. Nenhuma delas está implementada nesta versão.

Também faltam um estudo controlado das taxas e tamanhos de população e uma solução de referência para medir o erro das instâncias aleatórias. As justificativas deste README explicam a configuração adotada, sem afirmar superioridade experimental desses valores.

O visualizador identifica o cenário pelo nome do JSON. Mantenha `circular` no nome dos históricos circulares para que ele exiba a referência analítica correta.

## 13. Estrutura e validação

```text
include/         Cabeçalhos C
src/             Geração de pontos, indivíduos, avaliação, operadores e AG
tests/           Testes C
resultados/      Históricos JSON e métricas CSV
visualizacao.py  Gráficos, slider, comparação e GIFs
experimentos.py  Execuções e relatórios automatizados
Makefile         Compilação e alvo de testes do motor
readme.md        Explicação e instruções de uso
```

Na revisão de 25/10/2026, a compilação e as 53 verificações do alvo `make test` passaram. Também conferimos as rotas e distâncias dos três históricos existentes, geramos os gráficos dos dois cenários e testamos o slider Qt em modo offscreen. A revisão não confirmou a abertura da janela no desktop do usuário nem executou o bônus com os parâmetros ampliados.

Para repetir os testes e consultar as opções:

```bash
make test
./ag_tsp -h
python visualizacao.py --help
python experimentos.py --help
```

Para remover os executáveis gerados pelo Makefile:

```bash
make clean
```

## exemplos para a arguição

make -B
./ag_tsp -c circular -n 35 -o resultados_35
.venv/bin/python visualizacao.py resultados_35/historico_circular_42.json --interativo