PageRank con SNAP + OpenMP

Implementación de PageRank que compara la versión **secuencial** (`TSnap::GetPageRank`) contra la versión **paralela** (`TSnap::GetPageRankMP`) de la librería SNAP, variando el número de hilos con OpenMP. Mide tiempo de ejecución, speedup y eficiencia para cuatro grafos web reales.

---

Dependencias

| Herramienta | Versión recomendada |
|-------------|-------------------|
| g++ | ≥ 7 (con soporte C++98 y OpenMP) |
| OpenMP | incluido en g++ |
| Python 3 | ≥ 3.6 |
| matplotlib | `pip install matplotlib` |
| SNAP | ver instalación abajo |

---

Instalación de SNAP

```bash
Clonar SNAP en el home
cd ~
git clone https://github.com/snap-stanford/snap.git

Compilar la librería base
cd ~/snap/snap-core
make
```

> El `Makefile` del proyecto asume que SNAP está en `~/snap`. Si lo pusiste en otro lugar, pasa la ruta con `make SNAP_DIR=/ruta/a/snap`.

---

 Datasets

Los grafos usados vienen de [SNAP Stanford](https://snap.stanford.edu/data/index.html#web). Descarga los que quieras y ponlos en la carpeta `datos/`:

| Dataset | Nodos | Aristas | Enlace |
|---------|-------|---------|--------|
| web-Stanford | 281 903 | 2 312 497 | https://snap.stanford.edu/data/web-Stanford.html |
| web-NotreDame | 325 729 | 1 497 134 | https://snap.stanford.edu/data/web-NotreDame.html |
| web-BerkStan | 685 230 | 7 600 595 | https://snap.stanford.edu/data/web-BerkStan.html |
| web-Google | 875 713 | 5 105 039 | https://snap.stanford.edu/data/web-Google.html |

```bash
mkdir -p datos
 Ejemplo para web-Stanford:
wget -P datos/ https://snap.stanford.edu/data/web-Stanford.txt.gz
gunzip datos/web-Stanford.txt.gz
```

---

Compilar

```bash
make
```

Genera el binario `pagerank` en el directorio raíz.

---

Ejecutar

**Una sola red:**
```bash
make run DATASET=datos/web-Stanford.txt
```

**Las 4 redes de `datos/` de un jalón:**
```bash
make todos
```

**Directamente con el binario** (también acepta número de repeticiones):
```bash
./pagerank datos/web-Google.txt 5
```

---

Resultados

Por cada dataset se genera una carpeta en `resultados/<nombre>/` con:

| Archivo | Contenido |
|---------|-----------|
| `salida.txt` | Log completo: tiempos, speedup y eficiencia por número de hilos |
| `resultados.csv` | Tabla con hilos, tiempo secuencial, tiempo paralelo, speedup y eficiencia |
| `speedup.png` | Gráfica de speedup vs. número de hilos |
| `grados.png` | Distribución de grados de entrada y salida |
| `grados_entrada.csv` | Distribución de grado de entrada |
| `grados_salida.csv` | Distribución de grado de salida |

---

Parámetros de PageRank

Los mismos para ambas versiones (defaults de SNAP):

- Factor de amortiguamiento: **0.85**
- Tolerancia de convergencia: **1e-4**
- Máximo de iteraciones: **100**

---

Limpieza

```bash
make clean   # elimina el binario y archivos .o
```
