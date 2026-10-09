# 🧬 Algoritmo Genético para o Problema do Caixeiro Viajante

## 📌 Sobre o projeto

Este projeto tem como objetivo desenvolver e analisar um **Algoritmo Genético (AG)** para encontrar soluções aproximadas para o **Problema do Caixeiro Viajante (Traveling Salesman Problem – TSP)**.

O trabalho é desenvolvido para a disciplina de **Inteligência Artificial**, do curso de Engenharia de Computação da PUC-Campinas.

O algoritmo deverá encontrar um caminho cíclico que percorra um conjunto de pontos em um plano bidimensional, visitando todos os pontos e retornando à origem, buscando minimizar a distância euclidiana total percorrida.

Serão analisados dois cenários:

- Pontos distribuídos uniformemente de maneira aleatória.
- Pontos igualmente distribuídos sobre uma circunferência.

O projeto também deverá permitir acompanhar a evolução das soluções ao longo das gerações do algoritmo genético.

---

## 🎯 Objetivos

### Objetivo geral

Implementar um algoritmo genético capaz de encontrar boas soluções para o Problema do Caixeiro Viajante, avaliando seu desempenho em diferentes distribuições de pontos.

### Objetivos específicos

- Representar possíveis soluções utilizando cromossomos.
- Implementar os operadores de seleção, cruzamento e mutação.
- Definir uma função de aptidão para avaliar os indivíduos.
- Configurar parâmetros como tamanho da população, taxas de cruzamento e mutação e critérios de parada.
- Permitir a utilização de uma quantidade arbitrária de pontos, com no mínimo oito.
- Gerar e resolver instâncias com pontos aleatórios e circulares.
- Registrar a melhor solução encontrada a cada geração.
- Visualizar graficamente as rotas e sua evolução.
- Analisar a qualidade das soluções e o desempenho computacional.

---

## 🧩 O Problema do Caixeiro Viajante

O Problema do Caixeiro Viajante consiste em encontrar o menor percurso possível entre um conjunto de cidades, respeitando as seguintes condições:

1. Todas as cidades devem ser visitadas.
2. Cada cidade deve ser visitada exatamente uma vez, antes do retorno.
3. O percurso deve terminar na cidade inicial.
4. A distância total deve ser minimizada.

Neste projeto, as cidades serão representadas por pontos em um plano cartesiano bidimensional.

Cada ponto possui coordenadas `(x, y)`.

A distância euclidiana entre dois pontos é calculada por:

\[
d(A,B)=\sqrt{(x_B-x_A)^2+(y_B-y_A)^2}
\]

A distância total de uma rota será a soma das distâncias entre pontos consecutivos, incluindo o retorno ao ponto inicial.

O TSP é um problema de otimização combinatória NP-difícil. Por esse motivo, utilizaremos uma abordagem heurística baseada em algoritmos genéticos para encontrar soluções de boa qualidade sem realizar uma busca exaustiva.

---

## 🧬 Funcionamento do Algoritmo Genético

Algoritmos genéticos são métodos de otimização inspirados nos princípios da evolução natural.

O algoritmo trabalha com uma população de indivíduos, em que cada indivíduo representa uma possível solução para o problema.

A cada geração, os indivíduos são avaliados e novos indivíduos são produzidos por meio de seleção, cruzamento e mutação.

### 1. Representação dos indivíduos

Cada indivíduo será representado por um vetor contendo uma permutação dos índices dos pontos.

Exemplo com oito pontos:

```python
[0, 3, 7, 1, 5, 2, 6, 4]
```

Essa representação corresponde ao percurso:

```text
0 → 3 → 7 → 1 → 5 → 2 → 6 → 4 → 0
```

A representação por permutação garante que cada ponto apareça exatamente uma vez no cromossomo.

O retorno ao ponto inicial será considerado no cálculo da distância, sem precisar repetir esse ponto no vetor.

### 2. População inicial

A população inicial será composta por diferentes permutações aleatórias dos pontos.

Cada indivíduo representará uma rota válida.

O tamanho da população será configurável, permitindo avaliar como diferentes quantidades de indivíduos influenciam a diversidade genética, a qualidade das soluções e o tempo de execução.

### 3. Função de aptidão (fitness)

A aptidão de cada indivíduo será determinada pela distância total de sua rota.

Como o objetivo é minimizar essa distância, uma possível função de aptidão é:

\[
fitness=\frac{1}{1+D}
\]

Onde `D` representa a distância total percorrida.

Dessa forma, indivíduos com menores distâncias apresentam maiores valores de aptidão.

### 4. Seleção

A seleção será responsável por escolher os indivíduos utilizados na reprodução.

A abordagem inicialmente proposta é a **seleção por torneio**.

Nesse método, um pequeno grupo de indivíduos é selecionado aleatoriamente, e aquele com melhor aptidão é escolhido como um dos pais.

Esse processo favorece soluções de melhor qualidade, preservando a possibilidade de selecionar diferentes indivíduos.

### 5. Cruzamento (crossover)

O cruzamento combina informações genéticas de dois indivíduos para produzir descendentes.

A abordagem inicialmente proposta é o **Order Crossover (OX)**.

Esse método foi escolhido por ser adequado a representações baseadas em permutações.

O operador preserva uma sequência de posições de um dos pais e completa o restante do cromossomo respeitando a ordem relativa dos elementos do outro pai, evitando cidades duplicadas.

### 6. Mutação

A mutação introduz pequenas alterações aleatórias nos indivíduos para manter a diversidade genética da população.

A abordagem inicialmente proposta é a **Swap Mutation**, que consiste em selecionar duas posições do cromossomo e trocar seus elementos.

Exemplo:

Antes:

```python
[0, 1, 2, 3, 4, 5, 6, 7]
```

Depois:

```python
[0, 1, 5, 3, 4, 2, 6, 7]
```

A taxa de mutação será parametrizável para permitir a realização de experimentos.

### 7. Elitismo

O elitismo poderá ser utilizado para preservar os melhores indivíduos de uma geração na geração seguinte.

Esse mecanismo evita a perda das melhores soluções já encontradas durante a evolução.

### 8. Critério de parada

O algoritmo poderá utilizar dois critérios principais:

- Número máximo de gerações.
- Quantidade máxima de gerações consecutivas sem melhoria na melhor solução.

A execução será encerrada quando um desses critérios for atingido.

---

## ⚙️ Parâmetros iniciais propostos

Os parâmetros abaixo representam uma configuração inicial de referência. Seus valores poderão ser modificados e avaliados experimentalmente.

| Parâmetro | Configuração inicial |
|---|---|
| Tamanho da população | 100 indivíduos |
| Máximo de gerações | 500 |
| Seleção | Torneio |
| Tamanho do torneio | 3 indivíduos |
| Cruzamento | Order Crossover (OX) |
| Taxa de cruzamento | 80% |
| Mutação | Swap Mutation |
| Taxa de mutação | 10% por descendente |
| Elitismo | 2 melhores indivíduos |
| Parada por estagnação | 100 gerações sem melhoria |

Esses valores não representam necessariamente a melhor configuração. O desempenho deverá ser observado por meio de experimentos.

---

## 🌍 Cenários de teste

### Cenário 1: Distribuição uniforme aleatória

Nesse cenário, os pontos serão gerados aleatoriamente dentro de uma região bidimensional.

As coordenadas poderão ser obtidas a partir de distribuições uniformes.

Exemplo:

```python
pontos = [
    (10, 20),
    (35, 75),
    (60, 15),
    (90, 80),
    (25, 45),
    (70, 60),
    (15, 90),
    (85, 30)
]
```

O algoritmo deverá buscar uma rota de menor distância e apresentar a evolução da melhor solução durante as gerações.

### Cenário 2: Distribuição circular

Nesse cenário, os pontos serão distribuídos uniformemente sobre uma circunferência.

As coordenadas poderão ser calculadas por:

\[
x_i=x_c+R\cos\left(\frac{2\pi i}{n}\right)
\]

\[
y_i=y_c+R\sin\left(\frac{2\pi i}{n}\right)
\]

Onde:

- `R` é o raio da circunferência.
- `n` é a quantidade de pontos.
- `(x_c, y_c)` representa o centro do círculo.
- `i` é o índice de cada ponto.

Esse cenário será utilizado como benchmark, pois sua solução ótima é conhecida: percorrer os pontos em ordem circular.

Para pontos igualmente espaçados, a distância ótima é:

\[
D_{\text{ótima}}=2nR\sin\left(\frac{\pi}{n}\right)
\]

Isso permitirá comparar a distância obtida pelo algoritmo com o valor ótimo.

---

## 📊 Visualização e análise de desempenho

O projeto deverá registrar e apresentar o comportamento do algoritmo ao longo das gerações.

### Visualização das rotas

A visualização deverá apresentar:

- Posição dos pontos no plano cartesiano.
- Caminho correspondente à melhor solução.
- Melhor distância encontrada.
- Geração correspondente.

### Gráfico de evolução

Deverá ser gerado um gráfico relacionando:

- Eixo X: número da geração.
- Eixo Y: distância da melhor rota encontrada.

Esse gráfico permitirá acompanhar a convergência do algoritmo.

### Histórico das gerações

Para cada geração, deverão ser registrados pelo menos:

- Número da geração.
- Melhor distância encontrada até aquele momento.
- Rota correspondente à melhor solução.

Opcionalmente, poderão ser armazenados a distância média da população e outros indicadores.

### Avaliação dos resultados

A análise poderá considerar:

- Distância inicial e distância final.
- Redução percentual da distância.
- Número de gerações executadas.
- Tempo total de execução.
- Estabilidade dos resultados em diferentes execuções.

No cenário circular, também poderá ser calculado o erro percentual em relação à solução ótima:

\[
Erro(\%)=
\frac{D_{\text{obtida}}-D_{\text{ótima}}}
{D_{\text{ótima}}}\times100
\]

---

## 🚀 Teste adicional: grande quantidade de pontos

Como atividade complementar, será realizado, se possível, um experimento com uma quantidade elevada de pontos distribuídos sobre uma circunferência.

O objetivo será observar o comportamento do algoritmo diante do aumento do tamanho do problema.

Nesse experimento, serão analisados:

- Tempo total de execução.
- Tempo necessário para alcançar a melhor solução registrada.
- Quantidade de gerações necessárias.
- Distância final encontrada.
- Erro em relação à solução ótima.
- Soluções intermediárias em diferentes gerações.

O experimento permitirá discutir a escalabilidade e as limitações do algoritmo genético.

---

## 🛠️ Tecnologias propostas

O projeto poderá ser desenvolvido utilizando:

- **Python:** implementação do algoritmo.
- **NumPy:** operações numéricas e manipulação de coordenadas.
- **Matplotlib:** visualização das rotas e gráficos de evolução.
- **time:** medição do tempo de execução.

Bibliotecas adicionais poderão ser utilizadas conforme as necessidades do desenvolvimento.

---

## 📁 Estrutura planejada

```text
caixeiro-genetico/
│
├── main.py
├── pontos.py
├── individuo.py
├── genetico.py
├── visualizacao.py
│
├── resultados/
│
├── requirements.txt
└── README.md
```

### Responsabilidades dos arquivos

| Arquivo | Responsabilidade |
|---|---|
| `main.py` | Execução e configuração dos experimentos |
| `pontos.py` | Geração dos pontos nos dois cenários |
| `individuo.py` | Representação das soluções e cálculo das distâncias |
| `genetico.py` | Implementação do algoritmo genético |
| `visualizacao.py` | Visualização das rotas, gráficos e histórico |
| `resultados/` | Armazenamento dos dados experimentais |
| `requirements.txt` | Dependências do projeto |

A estrutura poderá ser adaptada durante o desenvolvimento.

---

## 🔄 Fluxo geral do programa

```text
INÍCIO
  |
  v
Definir os parâmetros
  |
  v
Gerar os pontos
  |
  v
Criar população inicial
  |
  v
Avaliar os indivíduos
  |
  v
Registrar a melhor solução
  |
  v
Critério de parada atingido?
  |
  +-- SIM --> Exibir resultados
  |
  +-- NÃO
       |
       v
   Selecionar pais
       |
       v
   Aplicar cruzamento
       |
       v
   Aplicar mutação
       |
       v
   Formar nova população
       |
       v
   Próxima geração
       |
       +----> Avaliar os indivíduos
```

---

## ✅ Requisitos do trabalho

- [ ] Implementar a representação dos cromossomos.
- [ ] Implementar a geração da população inicial.
- [ ] Implementar a função de aptidão.
- [ ] Implementar o processo de seleção.
- [ ] Implementar o operador de cruzamento.
- [ ] Implementar o operador de mutação.
- [ ] Definir e justificar os parâmetros do algoritmo.
- [ ] Implementar critérios de parada.
- [ ] Gerar pontos uniformemente aleatórios.
- [ ] Gerar pontos uniformemente distribuídos em círculo.
- [ ] Registrar a melhor solução de cada geração.
- [ ] Visualizar as rotas encontradas.
- [ ] Apresentar gráficos de desempenho.
- [ ] Comparar resultados nos dois cenários.
- [ ] Realizar teste com grande quantidade de pontos (bônus).
- [ ] Documentar e analisar os resultados.

---

## 📈 Resultados esperados

Espera-se que o algoritmo seja capaz de melhorar progressivamente a qualidade das rotas encontradas, reduzindo a distância total percorrida ao longo das gerações.

No cenário aleatório, o objetivo será encontrar soluções de boa qualidade, analisando a convergência e o desempenho do algoritmo.

No cenário circular, espera-se que o algoritmo encontre soluções próximas da rota ótima conhecida, permitindo avaliar quantitativamente a qualidade das soluções.

Também será investigada a influência dos parâmetros genéticos na eficiência e na qualidade dos resultados.

---

## 👥 Integrantes

Projeto desenvolvido por estudantes do curso de Engenharia de Computação da PUC-Campinas.

**Integrantes:**
- Bruno Vinicius Selingard
- João Vitor Bombo Guimarães
- Luccas Gomes Zibordi
- Vinicius de Moraes Chaves

---

## 📚 Considerações finais

O projeto busca demonstrar a aplicação prática de algoritmos genéticos em problemas de otimização combinatória.

Por meio da implementação dos operadores evolutivos e da análise dos resultados, será possível compreender como mecanismos de seleção, cruzamento e mutação contribuem para a busca de soluções eficientes.

A comparação entre diferentes cenários permitirá investigar tanto a capacidade de otimização quanto as limitações dessa abordagem.