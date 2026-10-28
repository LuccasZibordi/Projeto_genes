#!/usr/bin/env python3
"""
experimentos.py - Executa o AG em C e organiza os experimentos do trabalho.

Experimentos:
    aleatorio   cenario de pontos uniformemente distribuidos
    circular    benchmark circular (otimo conhecido)
    escala      circular com n crescente: tempo de convergencia x n
    bonus       circular com muitos pontos: tempo e epocas intermediarias
    sementes    repete aleatorio/circular com varias sementes (robustez)
    tudo        todos os anteriores + comparacao entre cenarios

Uso:
    python experimentos.py tudo
    python experimentos.py bonus --n-bonus 500
    python experimentos.py circular -n 30 -g 1000
"""

import argparse
import csv
import glob
import os
import shutil
import subprocess
import sys

import matplotlib

if not os.environ.get("DISPLAY") and not os.environ.get("WAYLAND_DISPLAY") \
        and sys.platform.startswith("linux"):
    matplotlib.use("Agg")

import matplotlib.pyplot as plt
import numpy as np

import visualizacao as viz

DIR_PROJETO = os.path.dirname(os.path.abspath(__file__))



NOMES_EXECUTAVEL = ("ag_tsp", "genetico", "tsp", "main", "ag", "projeto")
PASTAS_EXECUTAVEL = (".", "build", "bin", "out")
EXECUTAVEL = None
SAIDA = os.path.join(DIR_PROJETO, "resultados")
GRAFICOS = os.path.join(DIR_PROJETO, "graficos")


TOLERANCIAS = (0.10, 0.05, 0.01)





def _achar_executavel():
    """Procura o executavel nas pastas/nomes mais comuns."""
    for pasta in PASTAS_EXECUTAVEL:
        for nome in NOMES_EXECUTAVEL:
            for sufixo in ("", ".exe"):
                c = os.path.join(DIR_PROJETO, pasta, nome + sufixo)
                if os.path.isfile(c) and os.access(c, os.X_OK):
                    return c
    return None


def compilar(forcar=False, exe=None):
    """Define EXECUTAVEL: usa o informado, o ja existente, o Makefile ou o gcc."""
    global EXECUTAVEL

    exe = exe or os.environ.get("AG_EXE")
    if exe:
        EXECUTAVEL = os.path.abspath(exe)
        if not os.path.isfile(EXECUTAVEL):
            sys.exit(f"executavel nao encontrado: {EXECUTAVEL}")
        return

    if not forcar:
        EXECUTAVEL = _achar_executavel()
        if EXECUTAVEL:
            return

    makefile = os.path.join(DIR_PROJETO, "Makefile")
    if os.path.exists(makefile):
        for ferramenta in ("make", "mingw32-make"):
            if shutil.which(ferramenta):
                print(f"compilando com {ferramenta}")
                r = subprocess.run([ferramenta], cwd=DIR_PROJETO)
                EXECUTAVEL = _achar_executavel()
                if r.returncode == 0 and EXECUTAVEL:
                    return
                print(f"{ferramenta} nao gerou um executavel reconhecido; tentando gcc direto")
                break


    fontes = sorted(glob.glob(os.path.join(DIR_PROJETO, "src", "*.c"))) \
        or sorted(glob.glob(os.path.join(DIR_PROJETO, "*.c")))
    if not fontes:
        sys.exit("nenhum .c encontrado; informe o executavel com --exe")
    EXECUTAVEL = os.path.join(DIR_PROJETO, "genetico.exe" if os.name == "nt" else "genetico")
    cmd = ["gcc", "-O2", "-Wall", "-I", os.path.join(DIR_PROJETO, "include"),
           "-I", DIR_PROJETO, "-o", EXECUTAVEL] + fontes + ["-lm"] \
        + (["-lpthread"] if os.name == "nt" else [])
    if not shutil.which("gcc"):
        sys.exit("gcc nao encontrado no PATH. Compile o projeto do seu jeito e rode com "
                 "--exe CAMINHO_DO_EXECUTAVEL")
    print("compilando:", " ".join(cmd))
    subprocess.run(cmd, check=True)


def executar(cenario, n, pasta, semente=42, **ag):
    """Roda o AG e devolve o Historico carregado.

    ag aceita: populacao, geracoes, elite, torneio, cruzamento, mutacao,
               estagnacao (nomes em portugues, mapeados para as flags do main.c)
    """
    flags = {"populacao": "-p", "geracoes": "-g", "elite": "-e", "torneio": "-k",
             "cruzamento": "-t", "mutacao": "-m", "estagnacao": "-s"}

    cmd = [EXECUTAVEL, "-c", cenario, "-n", str(n), "-S", str(semente), "-o", pasta]
    for chave, valor in ag.items():
        if valor is not None:
            cmd += [flags[chave], str(valor)]

    os.makedirs(pasta, exist_ok=True)
    proc = subprocess.run(cmd, capture_output=True, text=True)
    if proc.returncode != 0:
        raise RuntimeError(f"AG falhou ({' '.join(cmd)}):\n{proc.stdout}\n{proc.stderr}")

    return viz.carregar_historico(os.path.join(pasta, f"historico_{cenario}_{semente}.json"))





def primeira_epoca_ate(hist, tolerancia):
    """Primeira epoca em que o recorde global fica a 'tolerancia' do otimo.
    Devolve (epoca, tempo_s) ou (None, None) se nunca chegou."""
    otimo = hist.distancia_otima
    if otimo is None:
        return None, None
    ok = np.where(hist.d_global <= otimo * (1.0 + tolerancia))[0]
    if len(ok) == 0:
        return None, None
    i = ok[0]
    return int(hist.epocas[i]), float(hist.tempo[i])


def epoca_da_melhor_solucao(hist):
    """Primeira epoca em que o recorde final foi alcancado (vale p/ qualquer cenario)."""
    i = np.where(hist.d_global <= hist.melhor_distancia + 1e-9)[0][0]
    return int(hist.epocas[i]), float(hist.tempo[i])


def resumo(hist):
    """Dicionario com os numeros principais de uma execucao."""
    ep_best, t_best = epoca_da_melhor_solucao(hist)
    r = {
        "cenario": hist.cenario, "n": hist.n, "semente": hist.semente,
        "populacao": hist.parametros["populacao"],
        "epocas": hist.total_epocas, "parada": hist.motivo_parada,
        "dist_inicial": hist.d_global[0], "dist_final": hist.melhor_distancia,
        "reducao_pct": 100 * (1 - hist.melhor_distancia / hist.d_global[0]),
        "epoca_melhor": ep_best, "tempo_melhor_s": t_best,
        "tempo_total_s": hist.tempo_total,
        "otimo": hist.distancia_otima, "erro_pct": None,
    }
    if hist.distancia_otima is not None:
        r["erro_pct"] = 100 * (hist.melhor_distancia - hist.distancia_otima) / hist.distancia_otima
        for tol in TOLERANCIAS:
            ep, t = primeira_epoca_ate(hist, tol)
            r[f"epoca_{int(tol * 100)}pct"] = ep
            r[f"tempo_{int(tol * 100)}pct_s"] = t
    return r


def salvar_tabela(linhas, caminho):
    os.makedirs(os.path.dirname(caminho), exist_ok=True)
    campos = []
    for l in linhas:
        for k in l:
            if k not in campos:
                campos.append(k)
    with open(caminho, "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=campos)
        w.writeheader()
        w.writerows(linhas)
    return caminho


def imprimir_resumo(r):
    print(f"  {r['cenario']:9s} n={r['n']:<5d} semente={r['semente']:<4d} "
          f"epocas={r['epocas']:<5d} parada={r['parada']:<12s} "
          f"final={r['dist_final']:.4f}  tempo={r['tempo_total_s']:.3f}s", end="")
    if r["erro_pct"] is not None:
        print(f"  erro={r['erro_pct']:.3f}%", end="")
    print()





def exp_cenario(cenario, n, semente=42, animacao=True, **ag):
    """Executa um cenario e gera todos os graficos (convergencia, epocas, GIF)."""
    print(f"[{cenario}] n={n}")
    pasta = os.path.join(SAIDA, cenario)
    hist = executar(cenario, n, pasta, semente, **ag)
    imprimir_resumo(resumo(hist))
    for arq in viz.gerar_relatorio(hist, os.path.join(GRAFICOS, cenario), animacao=animacao):
        print("  salvo:", os.path.relpath(arq, DIR_PROJETO))
    return hist


def exp_comparar(hist_a, hist_b):
    destino = os.path.join(GRAFICOS, "comparacao_cenarios.png")
    viz.comparar_cenarios(hist_a, hist_b, salvar=destino)
    print("  salvo:", os.path.relpath(destino, DIR_PROJETO))


def exp_escala(tamanhos, semente=42, **ag):
    """Circular com n crescente: como o tempo e as geracoes crescem com o problema."""
    print(f"[escala] n = {tamanhos}")
    linhas = []
    for n in tamanhos:
        pasta = os.path.join(SAIDA, f"escala_n{n}")
        hist = executar("circular", n, pasta, semente, **ag)
        r = resumo(hist)
        imprimir_resumo(r)
        linhas.append(r)

    tabela = salvar_tabela(linhas, os.path.join(GRAFICOS, "escala_circular.csv"))
    print("  salvo:", os.path.relpath(tabela, DIR_PROJETO))

    ns = [r["n"] for r in linhas]
    fig, axes = plt.subplots(1, 3, figsize=(15, 4.5))

    axes[0].plot(ns, [r["tempo_total_s"] for r in linhas], "o-", label="tempo total")
    axes[0].plot(ns, [r["tempo_melhor_s"] for r in linhas], "s--", label="ate a melhor solucao")
    axes[0].set_ylabel("Tempo (s)")
    axes[0].set_title("Tempo x numero de pontos")

    axes[1].plot(ns, [r["epocas"] for r in linhas], "o-", label="epocas executadas")
    axes[1].plot(ns, [r["epoca_melhor"] for r in linhas], "s--", label="epoca da melhor solucao")
    axes[1].set_ylabel("Geracoes")
    axes[1].set_title("Geracoes x numero de pontos")

    axes[2].plot(ns, [r["erro_pct"] for r in linhas], "o-", color=viz.COR_GLOBAL)
    axes[2].set_ylabel("Erro vs otimo (%)")
    axes[2].set_title("Qualidade final x numero de pontos")

    for ax in axes:
        ax.set_xlabel("Numero de pontos")
        ax.grid(alpha=0.3)
        if ax.get_legend_handles_labels()[0]:
            ax.legend(fontsize=8)

    fig.suptitle("Benchmark circular - escala")
    fig.tight_layout()
    destino = os.path.join(GRAFICOS, "escala_circular.png")
    fig.savefig(destino, dpi=150, bbox_inches="tight")
    plt.close(fig)
    print("  salvo:", os.path.relpath(destino, DIR_PROJETO))
    return linhas


def exp_bonus(n=100, semente=42, **ag):
    """Circular com muitos pontos: tempo ate convergir e rotas em epocas intermediarias."""
    print(f"[bonus] circular com n={n}")
    pasta = os.path.join(SAIDA, f"bonus_n{n}")
    hist = executar("circular", n, pasta, semente, **ag)
    r = resumo(hist)
    imprimir_resumo(r)

    print("  tempo ate chegar perto do otimo:")
    for tol in TOLERANCIAS:
        ep, t = r[f"epoca_{int(tol * 100)}pct"], r[f"tempo_{int(tol * 100)}pct_s"]
        if ep is None:
            print(f"    ate {tol:.0%} do otimo: nao atingiu")
        else:
            print(f"    ate {tol:.0%} do otimo: epoca {ep}, {t:.3f} s")

    pasta_g = os.path.join(GRAFICOS, f"bonus_n{n}")
    arquivos = viz.gerar_relatorio(hist, pasta_g, animacao=False)


    epocas = viz.epocas_representativas(hist, 8)
    arq = os.path.join(pasta_g, f"epocas_intermediarias_n{n}.png")
    viz.plotar_epocas(hist, epocas, usar_global=True, salvar=arq)
    arquivos.append(arq)

    arq = os.path.join(pasta_g, f"tempos_n{n}.csv")
    salvar_tabela(
        [{"epoca": e, "tempo_s": hist.snapshot(e)["tempo_segundos"],
          "dist_global": hist.snapshot(e)["distancia_global"],
          "erro_pct": 100 * (hist.snapshot(e)["distancia_global"] - hist.distancia_otima)
          / hist.distancia_otima} for e in epocas], arq)
    arquivos.append(arq)

    for a in arquivos:
        print("  salvo:", os.path.relpath(a, DIR_PROJETO))
    return hist


def exp_sementes(n_aleatorio=30, n_circular=20, sementes=range(1, 11), **ag):
    """Repete cada cenario com varias sementes para mostrar a variabilidade."""
    print(f"[sementes] {len(list(sementes))} execucoes por cenario")
    linhas = []
    fig, axes = plt.subplots(1, 2, figsize=(13, 4.8))

    for ax, (cenario, n) in zip(axes, (("aleatorio", n_aleatorio), ("circular", n_circular))):
        finais = []
        for s in sementes:
            hist = executar(cenario, n, os.path.join(SAIDA, f"sementes_{cenario}"), s, **ag)
            r = resumo(hist)
            linhas.append(r)
            finais.append(r["dist_final"])
            ax.plot(hist.epocas, hist.d_global, lw=1, alpha=0.7)
        finais = np.array(finais)
        titulo = f"{cenario} (n={n}): final = {finais.mean():.2f} ± {finais.std():.2f}"
        if cenario == "circular":
            otimo = hist.distancia_otima
            ax.axhline(otimo, color=viz.COR_OTIMO, ls="--", label="otimo")
            ax.legend()
        ax.set_title(titulo)
        ax.set_xlabel("Geracao")
        ax.set_ylabel("Melhor distancia global")
        ax.grid(alpha=0.3)
        print(f"  {titulo}")

    fig.suptitle("Variabilidade entre sementes")
    fig.tight_layout()
    destino = os.path.join(GRAFICOS, "sementes.png")
    fig.savefig(destino, dpi=150, bbox_inches="tight")
    plt.close(fig)
    salvar_tabela(linhas, os.path.join(GRAFICOS, "sementes.csv"))
    print("  salvo:", os.path.relpath(destino, DIR_PROJETO))





def main():
    ap = argparse.ArgumentParser(description="Experimentos do AG para o TSP")
    ap.add_argument("experimento",
                    choices=["aleatorio", "circular", "escala", "bonus", "sementes", "tudo"])
    ap.add_argument("-n", type=int, help="pontos (aleatorio/circular)")
    ap.add_argument("-S", "--semente", type=int, default=42)
    ap.add_argument("-p", "--populacao", type=int)
    ap.add_argument("-g", "--geracoes", type=int)
    ap.add_argument("-e", "--elite", type=int)
    ap.add_argument("-k", "--torneio", type=int)
    ap.add_argument("-t", "--cruzamento", type=float)
    ap.add_argument("-m", "--mutacao", type=float)
    ap.add_argument("-s", "--estagnacao", type=int)
    ap.add_argument("--n-bonus", type=int, default=100, help="pontos no teste bonus")
    ap.add_argument("--tamanhos", type=int, nargs="+", default=[10, 20, 50, 100],
                    help="tamanhos usados no experimento de escala")
    ap.add_argument("--sem-gif", action="store_true")
    ap.add_argument("--recompilar", action="store_true")
    ap.add_argument("--exe", help="caminho do executavel do AG (pula a compilacao)")
    a = ap.parse_args()

    compilar(a.recompilar, a.exe)

    ag = {k: getattr(a, k) for k in
          ("populacao", "geracoes", "elite", "torneio", "cruzamento", "mutacao", "estagnacao")}
    gif = not a.sem_gif




    ag_grande = dict(ag)
    padroes_grande = {"geracoes": 20000, "estagnacao": 3000, "populacao": 500,
                      "mutacao": 0.5, "torneio": 5}
    for chave, valor in padroes_grande.items():
        if ag_grande[chave] is None:
            ag_grande[chave] = valor

    exp = a.experimento
    hist_a = hist_c = None

    if exp in ("aleatorio", "tudo"):
        hist_a = exp_cenario("aleatorio", a.n or 30, a.semente, gif, **ag)
    if exp in ("circular", "tudo"):
        hist_c = exp_cenario("circular", a.n or 20, a.semente, gif, **ag)
    if exp == "tudo" and hist_a and hist_c:
        print("[comparacao]")
        exp_comparar(hist_a, hist_c)
    if exp in ("escala", "tudo"):
        exp_escala(a.tamanhos, a.semente, **ag_grande)
    if exp in ("bonus", "tudo"):
        exp_bonus(a.n_bonus, a.semente, **ag_grande)
    if exp in ("sementes", "tudo"):
        exp_sementes(**ag)


if __name__ == "__main__":
    main()