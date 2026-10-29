# Genera las graficas del reporte a partir de los CSV que escribe ./pagerank
# Requiere: sudo apt install -y python3-matplotlib
#
# Uso:
#   python3 graficar.py                        -> grafica todas las carpetas de resultados/
#   python3 graficar.py resultados/web-Google  -> grafica solo esa carpeta
import csv
import glob
import os
import sys
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt


def leer(fnm):
    with open(fnm) as f:
        return list(csv.DictReader(f))


def graficar(carpeta):
    nombre = os.path.basename(os.path.normpath(carpeta))
    res = leer(os.path.join(carpeta, "resultados.csv"))
    hilos = [int(r["hilos"]) for r in res]
    tiempo = [float(r["tiempo_s"]) for r in res]
    speedup = [float(r["speedup"]) for r in res]
    efic = [100 * float(r["eficiencia"]) for r in res]

    fig, ax = plt.subplots(1, 3, figsize=(15, 4.5))
    ax[0].plot(hilos, tiempo, "o-")
    ax[0].set(title="Tiempo de GetPageRankMP", xlabel="Hilos", ylabel="Tiempo (s)")
    ax[1].plot(hilos, speedup, "o-", label="Medido")
    ax[1].plot(hilos, hilos, "--", color="gray", label="Ideal")
    ax[1].set(title="Speedup", xlabel="Hilos", ylabel="T_sec / T_par")
    ax[1].legend()
    ax[2].plot(hilos, efic, "o-")
    ax[2].set(title="Eficiencia", xlabel="Hilos", ylabel="Eficiencia (%)", ylim=(0, 110))
    for a in ax:
        a.set_xticks(hilos)
        a.grid(alpha=0.3)
    fig.suptitle(nombre)
    fig.tight_layout()
    fig.savefig(os.path.join(carpeta, "speedup.png"), dpi=150)
    plt.close(fig)

    fig, ax = plt.subplots(figsize=(6, 4.5))
    for fnm, etiqueta in [("grados_entrada.csv", "Entrada"), ("grados_salida.csv", "Salida")]:
        d = [r for r in leer(os.path.join(carpeta, fnm)) if int(r["grado"]) > 0]
        ax.loglog([int(r["grado"]) for r in d], [int(r["nodos"]) for r in d], ".", label=etiqueta)
    ax.set(title="Distribucion de grados (log-log) - " + nombre, xlabel="Grado", ylabel="Numero de nodos")
    ax.legend()
    ax.grid(alpha=0.3)
    fig.tight_layout()
    fig.savefig(os.path.join(carpeta, "grados.png"), dpi=150)
    plt.close(fig)

    print("Guardado en %s: speedup.png, grados.png" % carpeta)


carpetas = sys.argv[1:] or sorted(glob.glob("resultados/*/"))
if not carpetas:
    print("No hay resultados. Primero corre: ./pagerank datos/web-Google.txt")
for c in carpetas:
    graficar(c)
