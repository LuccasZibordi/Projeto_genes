#!/usr/bin/env python3
"""
visualizacao.py - Visualizacao e analise dos resultados do AG (TSP).

Le os arquivos gerados pelo programa em C:
    historico_<cenario>_<semente>.json   (pontos + rota de cada epoca)
    metricas_<cenario>_<semente>.csv     (distancias e tempo por epoca)

Funcoes principais:
    plotar_rota             rota de uma epoca
    plotar_convergencia     distancia x geracao
    plotar_epocas           varias epocas lado a lado
    visualizador_interativo slider para navegar pelas epocas
    gerar_animacao          GIF da evolucao da rota
    comparar_cenarios       aleatorio x circular
    gerar_relatorio         salva todos os graficos de um historico

Uso rapido (linha de comando):
    python visualizacao.py resultados/historico_circular_42.json
    python visualizacao.py resultados/historico_circular_42.json --epocas 0 10 50 100
    python visualizacao.py resultados/historico_circular_42.json --interativo
    python visualizacao.py --comparar resultados/historico_aleatorio_42.json \
                                      resultados/historico_circular_42.json
"""

import argparse
import json
import math
import os
import sys

import matplotlib

# Sem DISPLAY (servidor, WSL sem X) usa backend de arquivo.
if os.environ.get("MPLBACKEND") is None and sys.platform.startswith("linux") \
        and not os.environ.get("DISPLAY") and not os.environ.get("WAYLAND_DISPLAY"):
    matplotlib.use("Agg")

import matplotlib.pyplot as plt
import numpy as np
from matplotlib.widgets import Slider

# Paleta usada em todos os graficos
COR_MELHOR = "#1f77b4"
COR_GLOBAL = "#d62728"
COR_MEDIA = "#7f7f7f"
COR_PIOR = "#bbbbbb"
COR_OTIMO = "#2ca02c"


# ----------------------------------------------------------------------
# Leitura dos dados
# ----------------------------------------------------------------------
class Historico:
    """Conteudo de um historico_*.json, com acesso pratico por epoca."""

    def __init__(self, caminho):
        with open(caminho, encoding="utf-8") as f:
            dados = json.load(f)

        self.caminho = caminho
        self.semente = dados["semente"]
        self.motivo_parada = dados["motivo_parada"]
        self.melhor_distancia = dados["melhor_distancia"]
        self.tempo_total = dados["tempo_total"]
        self.parametros = dados["parametros"]
        self.pontos = np.array(dados["pontos"], dtype=float)
        self.snapshots = dados["snapshots"]
        self.n = len(self.pontos)

        nome = os.path.basename(caminho).lower()
        self.cenario = "circular" if "circular" in nome else "aleatorio"

        # Vetores prontos para plotar
        self.epocas = np.array([s["epoca"] for s in self.snapshots])
        self.d_epoca = np.array([s["distancia_epoca"] for s in self.snapshots])
        self.d_media = np.array([s["distancia_media"] for s in self.snapshots])
        self.d_pior = np.array([s["distancia_pior"] for s in self.snapshots])
        self.d_global = np.array([s["distancia_global"] for s in self.snapshots])
        self.tempo = np.array([s["tempo_segundos"] for s in self.snapshots])

    @property
    def total_epocas(self):
        return int(self.epocas[-1])

    @property
    def distancia_otima(self):
        """Otimo analitico do poligono regular (so existe no cenario circular)."""
        if self.cenario != "circular":
            return None
        centro = self.pontos.mean(axis=0)
        raio = np.linalg.norm(self.pontos - centro, axis=1).mean()
        return 2.0 * self.n * raio * math.sin(math.pi / self.n)

    def snapshot(self, epoca):
        """Devolve o snapshot da epoca pedida (ou o mais proximo anterior)."""
        idx = int(np.searchsorted(self.epocas, epoca, side="right")) - 1
        return self.snapshots[max(idx, 0)]

    def rotulo(self):
        return f"{self.cenario} (n={self.n}, semente={self.semente})"


def carregar_historico(caminho):
    return Historico(caminho)


def carregar_metricas_csv(caminho):
    """Le o CSV de metricas como dict de arrays numpy."""
    dados = np.genfromtxt(caminho, delimiter=",", names=True)
    return {nome: dados[nome] for nome in dados.dtype.names}


# ----------------------------------------------------------------------
# Rotas
# ----------------------------------------------------------------------
def plotar_rota(ax, hist, epoca, usar_global=False, otimo=True):
    """Desenha a rota de uma epoca em ax.

    usar_global=False -> melhor individuo daquela geracao
    usar_global=True  -> melhor rota encontrada ate aquela epoca
    """
    s = hist.snapshot(epoca)
    rota = s["rota_global"] if usar_global else s["rota"]
    dist = s["distancia_global"] if usar_global else s["distancia_epoca"]

    xs = hist.pontos[rota + [rota[0]], 0]
    ys = hist.pontos[rota + [rota[0]], 1]

    grande = hist.n > 60
    ax.plot(xs, ys, "-", color=COR_MELHOR, lw=1.6 if grande else 1.4, zorder=1)
    ax.scatter(hist.pontos[:, 0], hist.pontos[:, 1], s=6 if grande else 28, c="black", zorder=2)

    # Ponto de partida e seta de sentido
    ax.scatter(*hist.pontos[rota[0]], s=70, c=COR_GLOBAL, zorder=3)
    if hist.n <= 60:
        for k in range(len(rota)):
            a = hist.pontos[rota[k]]
            b = hist.pontos[rota[(k + 1) % len(rota)]]
            ax.annotate("", xy=(a + b) / 2 + (b - a) * 0.01, xytext=(a + b) / 2 - (b - a) * 0.01,
                        arrowprops=dict(arrowstyle="->", color=COR_MELHOR, lw=1.2), zorder=1)

    titulo = f"Epoca {s['epoca']}  |  dist = {dist:.2f}"
    otimo_val = hist.distancia_otima
    if otimo and otimo_val is not None:
        erro = 100.0 * (dist - otimo_val) / otimo_val
        titulo += f"  ({erro:+.1f}% do otimo)"
    ax.set_title(titulo, fontsize=9)
    ax.set_aspect("equal", adjustable="datalim")
    ax.tick_params(labelsize=7)
    ax.grid(alpha=0.2)


def plotar_epocas(hist, epocas=None, usar_global=False, colunas=4, salvar=None):
    """Grade com varias epocas. Sem 'epocas', escolhe 8 espacadas em escala log."""
    if epocas is None:
        epocas = epocas_representativas(hist, 8)

    linhas = math.ceil(len(epocas) / colunas)
    fig, axes = plt.subplots(linhas, colunas, figsize=(3.6 * colunas, 3.6 * linhas),
                             squeeze=False)

    for ax, e in zip(axes.flat, epocas):
        plotar_rota(ax, hist, e, usar_global)
    for ax in list(axes.flat)[len(epocas):]:
        ax.axis("off")

    tipo = "melhor rota global" if usar_global else "melhor da geracao"
    fig.suptitle(f"Evolucao da rota ({tipo}) - {hist.rotulo()}", fontsize=12)
    fig.tight_layout()
    return _finalizar(fig, salvar)


def epocas_representativas(hist, quantidade=8):
    """Epocas espacadas em escala logaritmica (a maior parte do ganho vem cedo)."""
    ultima = hist.total_epocas
    if ultima < quantidade:
        return list(range(ultima + 1))
    valores = np.unique(np.round(np.geomspace(1, ultima, quantidade - 1)).astype(int))
    return [0] + [int(v) for v in valores]


# ----------------------------------------------------------------------
# Convergencia
# ----------------------------------------------------------------------
def plotar_convergencia(hist, log_y=False, salvar=None):
    """Distancia por geracao: melhor da epoca, recorde global, media e pior."""
    fig, ax = plt.subplots(figsize=(9, 5))

    ax.fill_between(hist.epocas, hist.d_epoca, hist.d_pior, color=COR_PIOR, alpha=0.35,
                    label="faixa melhor-pior da populacao")
    ax.plot(hist.epocas, hist.d_media, color=COR_MEDIA, lw=1.2, label="media da populacao")
    ax.plot(hist.epocas, hist.d_epoca, color=COR_MELHOR, lw=1.2, label="melhor da geracao")
    ax.plot(hist.epocas, hist.d_global, color=COR_GLOBAL, lw=2, label="melhor global (recorde)")

    otimo = hist.distancia_otima
    if otimo is not None:
        ax.axhline(otimo, color=COR_OTIMO, ls="--", lw=1.5, label=f"otimo analitico ({otimo:.2f})")

    if log_y:
        ax.set_yscale("log")

    ax.set_xlabel("Geracao (epoca)")
    ax.set_ylabel("Distancia total da rota")
    ax.set_title(f"Convergencia - {hist.rotulo()}\n"
                 f"final = {hist.melhor_distancia:.2f}  |  parada: {hist.motivo_parada}  |  "
                 f"{hist.tempo_total:.3f} s")
    ax.grid(alpha=0.3)
    ax.legend(fontsize=8)
    fig.tight_layout()
    return _finalizar(fig, salvar)


# ----------------------------------------------------------------------
# Navegacao interativa e animacao
# ----------------------------------------------------------------------
def visualizador_interativo(hist):
    """Janela com a rota e um slider de epoca (setas esquerda/direita tambem)."""
    fig = plt.figure(figsize=(11, 5.5))
    ax_rota = fig.add_axes([0.05, 0.22, 0.42, 0.68])
    ax_graf = fig.add_axes([0.56, 0.22, 0.40, 0.68])
    ax_slider = fig.add_axes([0.15, 0.07, 0.70, 0.04])

    slider = Slider(ax_slider, "Epoca", 0, hist.total_epocas, valinit=0, valstep=1)

    ax_graf.plot(hist.epocas, hist.d_epoca, color=COR_MELHOR, lw=1, label="melhor da geracao")
    ax_graf.plot(hist.epocas, hist.d_global, color=COR_GLOBAL, lw=1.8, label="melhor global")
    if hist.distancia_otima is not None:
        ax_graf.axhline(hist.distancia_otima, color=COR_OTIMO, ls="--", label="otimo")
    marcador = ax_graf.axvline(0, color="black", lw=1)
    ax_graf.set_xlabel("Geracao")
    ax_graf.set_ylabel("Distancia")
    ax_graf.grid(alpha=0.3)
    ax_graf.legend(fontsize=8)

    def atualizar(valor):
        ax_rota.clear()
        plotar_rota(ax_rota, hist, int(valor))
        marcador.set_xdata([valor, valor])
        fig.canvas.draw_idle()

    def tecla(evento):
        passo = {"right": 1, "left": -1, "up": 10, "down": -10}.get(evento.key)
        if passo:
            slider.set_val(min(max(slider.val + passo, 0), hist.total_epocas))

    slider.on_changed(atualizar)
    fig.canvas.mpl_connect("key_press_event", tecla)
    atualizar(0)
    fig.suptitle(hist.rotulo())
    plt.show()
    return slider  # evita o slider ser coletado pelo GC


def gerar_animacao(hist, caminho, quadros=60, fps=10, usar_global=True):
    """Salva um GIF mostrando a rota evoluir (quadros espacados em escala log)."""
    from matplotlib.animation import FuncAnimation, PillowWriter

    ultima = hist.total_epocas
    if ultima + 1 <= quadros:
        epocas = list(range(ultima + 1))
    else:
        epocas = sorted({0, ultima} | {int(round(v)) for v in
                                       np.geomspace(1, ultima, quadros - 2)})

    fig, ax = plt.subplots(figsize=(5.5, 5.5))

    def quadro(i):
        ax.clear()
        plotar_rota(ax, hist, epocas[i], usar_global)

    anim = FuncAnimation(fig, quadro, frames=len(epocas))
    _garantir_pasta(caminho)
    anim.save(caminho, writer=PillowWriter(fps=fps))
    plt.close(fig)
    return caminho


# ----------------------------------------------------------------------
# Comparacao de cenarios
# ----------------------------------------------------------------------
def comparar_cenarios(hist_a, hist_b, salvar=None):
    """Compara dois historicos (tipicamente aleatorio x circular).

    As distancias sao normalizadas pela distancia final de cada execucao,
    porque os dois cenarios tem escalas diferentes.
    """
    fig, axes = plt.subplots(2, 2, figsize=(12, 9))

    for hist, cor in ((hist_a, COR_MELHOR), (hist_b, COR_GLOBAL)):
        # Distancia absoluta
        axes[0, 0].plot(hist.epocas, hist.d_global, color=cor, lw=1.8, label=hist.rotulo())
        # Distancia relativa ao valor final
        axes[0, 1].plot(hist.epocas, hist.d_global / hist.melhor_distancia, color=cor, lw=1.8,
                        label=hist.rotulo())
        # Convergencia no tempo real
        axes[1, 0].plot(hist.tempo, hist.d_global / hist.melhor_distancia, color=cor, lw=1.8,
                        label=hist.rotulo())

    axes[0, 0].set_title("Melhor distancia global por geracao")
    axes[0, 0].set_xlabel("Geracao")
    axes[0, 0].set_ylabel("Distancia")

    axes[0, 1].set_title("Distancia / distancia final (normalizada)")
    axes[0, 1].set_xlabel("Geracao")
    axes[0, 1].set_yscale("log")

    axes[1, 0].set_title("Convergencia no tempo")
    axes[1, 0].set_xlabel("Tempo (s)")
    axes[1, 0].set_yscale("log")

    for ax in axes.flat[:3]:
        ax.grid(alpha=0.3)
        ax.legend(fontsize=8)

    # Tabela-resumo
    ax_t = axes[1, 1]
    ax_t.axis("off")
    cabecalho = ["", hist_a.cenario, hist_b.cenario]
    linhas = [
        ["pontos", hist_a.n, hist_b.n],
        ["epocas", hist_a.total_epocas, hist_b.total_epocas],
        ["dist. inicial", f"{hist_a.d_global[0]:.2f}", f"{hist_b.d_global[0]:.2f}"],
        ["dist. final", f"{hist_a.melhor_distancia:.2f}", f"{hist_b.melhor_distancia:.2f}"],
        ["reducao", f"{100 * (1 - hist_a.melhor_distancia / hist_a.d_global[0]):.1f}%",
         f"{100 * (1 - hist_b.melhor_distancia / hist_b.d_global[0]):.1f}%"],
        ["tempo (s)", f"{hist_a.tempo_total:.3f}", f"{hist_b.tempo_total:.3f}"],
        ["parada", hist_a.motivo_parada, hist_b.motivo_parada],
        ["otimo", _fmt_otimo(hist_a), _fmt_otimo(hist_b)],
        ["erro vs otimo", _fmt_erro(hist_a), _fmt_erro(hist_b)],
    ]
    tabela = ax_t.table(cellText=linhas, colLabels=cabecalho, loc="center", cellLoc="center")
    tabela.auto_set_font_size(False)
    tabela.set_fontsize(10)
    tabela.scale(1, 1.6)

    fig.suptitle("Comparacao entre cenarios", fontsize=13)
    fig.tight_layout()
    return _finalizar(fig, salvar)


def _fmt_otimo(h):
    return f"{h.distancia_otima:.2f}" if h.distancia_otima is not None else "n/d"


def _fmt_erro(h):
    o = h.distancia_otima
    return f"{100 * (h.melhor_distancia - o) / o:.3f}%" if o is not None else "n/d"


# ----------------------------------------------------------------------
# Relatorio completo
# ----------------------------------------------------------------------
def gerar_relatorio(hist, pasta, epocas=None, animacao=False):
    """Salva convergencia, grade de epocas (melhor da geracao e global) e,
    opcionalmente, o GIF. Devolve a lista de arquivos criados."""
    base = f"{hist.cenario}_n{hist.n}_s{hist.semente}"
    arquivos = []

    caminho = os.path.join(pasta, f"convergencia_{base}.png")
    plotar_convergencia(hist, salvar=caminho)
    arquivos.append(caminho)

    caminho = os.path.join(pasta, f"convergencia_log_{base}.png")
    plotar_convergencia(hist, log_y=True, salvar=caminho)
    arquivos.append(caminho)

    caminho = os.path.join(pasta, f"epocas_{base}.png")
    plotar_epocas(hist, epocas, usar_global=True, salvar=caminho)
    arquivos.append(caminho)

    caminho = os.path.join(pasta, f"rota_final_{base}.png")
    fig, ax = plt.subplots(figsize=(6, 6))
    plotar_rota(ax, hist, hist.total_epocas, usar_global=True)
    fig.tight_layout()
    _finalizar(fig, caminho)
    arquivos.append(caminho)

    if animacao:
        arquivos.append(gerar_animacao(hist, os.path.join(pasta, f"animacao_{base}.gif")))

    return arquivos


# ----------------------------------------------------------------------
# Utilidades
# ----------------------------------------------------------------------
def _garantir_pasta(caminho):
    pasta = os.path.dirname(caminho)
    if pasta:
        os.makedirs(pasta, exist_ok=True)


def _finalizar(fig, salvar):
    """Salva (se pedido) e fecha a figura quando nao ha janela para mostrar."""
    if salvar:
        _garantir_pasta(salvar)
        fig.savefig(salvar, dpi=150, bbox_inches="tight")
        plt.close(fig)
        return salvar
    return fig


def main():
    ap = argparse.ArgumentParser(description="Visualizacao dos resultados do AG-TSP")
    ap.add_argument("json", nargs="*", help="historico_*.json")
    ap.add_argument("--epocas", type=int, nargs="+", help="epocas a mostrar na grade")
    ap.add_argument("--saida", default="graficos", help="pasta de saida (padrao: graficos)")
    ap.add_argument("--interativo", action="store_true", help="abre o slider de epocas")
    ap.add_argument("--gif", action="store_true", help="gera tambem a animacao")
    ap.add_argument("--comparar", action="store_true",
                    help="compara dois JSONs (ex.: aleatorio e circular)")
    args = ap.parse_args()

    if not args.json:
        ap.error("informe ao menos um historico_*.json")

    if args.comparar:
        if len(args.json) != 2:
            ap.error("--comparar exige exatamente 2 arquivos")
        a, b = carregar_historico(args.json[0]), carregar_historico(args.json[1])
        destino = os.path.join(args.saida, "comparacao_cenarios.png")
        print("salvo:", comparar_cenarios(a, b, salvar=destino))
        return

    for caminho in args.json:
        hist = carregar_historico(caminho)
        if args.interativo:
            visualizador_interativo(hist)
        else:
            for arq in gerar_relatorio(hist, args.saida, args.epocas, args.gif):
                print("salvo:", arq)


if __name__ == "__main__":
    main()