// PageRank con SNAP + OpenMP
// Compara TSnap::GetPageRank (secuencial) contra TSnap::GetPageRankMP (paralelo)
// variando el numero de hilos, y reporta tiempo, speedup y eficiencia.
//
// Uso: ./pagerank <dataset.txt> [repeticiones=3]
// Los resultados se guardan en resultados/<nombre del dataset>/

#include "Snap.h"
#include <omp.h>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cmath>
#include <string>
#include <vector>
#include <algorithm>
#include <sys/stat.h>

// Mismos parametros para ambas versiones (los default de SNAP)
const double C = 0.85;
const double EPS = 1e-4;
const int MAX_ITER = 100;

// Todo lo que se imprime en pantalla tambien se escribe en salida.txt
FILE* Salida = NULL;
void Log(const char* Fmt, ...) {
  va_list Args;
  va_start(Args, Fmt);
  vprintf(Fmt, Args);
  va_end(Args);
  if (Salida != NULL) {
    va_start(Args, Fmt);
    vfprintf(Salida, Fmt, Args);
    va_end(Args);
  }
}

// Diferencia maxima entre dos resultados de PageRank, comparando nodo por nodo
double MaxDiff(const TIntFltH& A, const TIntFltH& B) {
  double MxDiff = 0;
  for (int i = A.FFirstKeyId(); A.FNextKeyId(i); ) {
    const int NId = A.GetKey(i);
    const double d = fabs(A[i] - B.GetDat(NId));
    if (d > MxDiff) { MxDiff = d; }
  }
  return MxDiff;
}

// Guarda una distribucion de grados (grado, cantidad de nodos) en CSV
void SaveDegCnt(const TIntPrV& DegCnt, const char* FNm) {
  FILE* F = fopen(FNm, "w");
  if (F == NULL) { return; }
  fprintf(F, "grado,nodos\n");
  for (int i = 0; i < DegCnt.Len(); i++) {
    fprintf(F, "%d,%d\n", DegCnt[i].Val1.Val, DegCnt[i].Val2.Val);
  }
  fclose(F);
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    Log("Uso: %s <dataset.txt> [repeticiones=3]\n", argv[0]);
    return 1;
  }
  const char* FNm = argv[1];
  const int Reps = argc > 2 ? atoi(argv[2]) : 3;
  const int NProcs = omp_get_num_procs();

  // Carpeta de salida: resultados/<nombre del archivo sin ruta ni extension>
  // p. ej. datos/web-Google.txt -> resultados/web-Google/
  std::string Nombre(FNm);
  Nombre = Nombre.substr(Nombre.find_last_of('/') + 1);
  Nombre = Nombre.substr(0, Nombre.find_last_of('.'));
  const std::string Dir = "resultados/" + Nombre + "/";
  mkdir("resultados", 0755);
  mkdir(Dir.c_str(), 0755);
  Salida = fopen((Dir + "salida.txt").c_str(), "w");

  // ---------------- 1. Carga del grafo ----------------
  Log("Cargando %s ...\n", FNm);
  double t0 = omp_get_wtime();
  PNGraph G = TSnap::LoadEdgeList<PNGraph>(FNm, 0, 1);
  Log("Cargado en %.2f s\n\n", omp_get_wtime() - t0);

  // ---------------- 2. Datos de la red ----------------
  int Colgantes = 0, SinEntrada = 0;
  int MxIn = 0, MxInNId = -1, MxOut = 0, MxOutNId = -1;
  for (TNGraph::TNodeI NI = G->BegNI(); NI < G->EndNI(); NI++) {
    if (NI.GetOutDeg() == 0) { Colgantes++; }
    if (NI.GetInDeg() == 0) { SinEntrada++; }
    if (NI.GetInDeg() > MxIn) { MxIn = NI.GetInDeg(); MxInNId = NI.GetId(); }
    if (NI.GetOutDeg() > MxOut) { MxOut = NI.GetOutDeg(); MxOutNId = NI.GetId(); }
  }
  const int NNodes = G->GetNodes();
  const int NEdges = G->GetEdges();

  Log("=== Datos de la red ===\n");
  Log("Nodos:                         %d\n", NNodes);
  Log("Aristas:                       %d\n", NEdges);
  Log("Grado promedio (salida):       %.2f\n", (double) NEdges / NNodes);
  Log("Nodos colgantes (sin salida):  %d (%.1f%%)\n", Colgantes, 100.0 * Colgantes / NNodes);
  Log("Nodos sin aristas de entrada:  %d (%.1f%%)\n", SinEntrada, 100.0 * SinEntrada / NNodes);
  Log("Grado maximo de entrada:       %d (nodo %d)\n", MxIn, MxInNId);
  Log("Grado maximo de salida:        %d (nodo %d)\n", MxOut, MxOutNId);
  Log("Autoaristas (self-loops):      %d\n\n", TSnap::CntSelfEdges(G));

  // Distribucion de grados (para graficar en el reporte)
  TIntPrV InDegCnt, OutDegCnt;
  TSnap::GetInDegCnt(G, InDegCnt);
  TSnap::GetOutDegCnt(G, OutDegCnt);
  SaveDegCnt(InDegCnt, (Dir + "grados_entrada.csv").c_str());
  SaveDegCnt(OutDegCnt, (Dir + "grados_salida.csv").c_str());

  Log("=== Configuracion ===\n");
  Log("Nucleos disponibles (omp_get_num_procs): %d\n", NProcs);
  Log("C = %.2f, Eps = %g, MaxIter = %d, repeticiones = %d\n\n", C, EPS, MAX_ITER, Reps);

  // ---------------- 3. Secuencial ----------------
  TIntFltH PRankSeq;
  TSnap::GetPageRank(G, PRankSeq, C, EPS, MAX_ITER);  // calentamiento (no se mide)
  double TSeq = 0;
  Log("Secuencial (GetPageRank):\n");
  for (int r = 0; r < Reps; r++) {
    t0 = omp_get_wtime();
    TSnap::GetPageRank(G, PRankSeq, C, EPS, MAX_ITER);
    const double t = omp_get_wtime() - t0;
    Log("  corrida %d: %.4f s\n", r + 1, t);
    TSeq += t;
  }
  TSeq /= Reps;
  Log("  promedio:  %.4f s\n\n", TSeq);

  // ---------------- 4. Paralelo ----------------
  // 1, 2, 4, 8 hilos (sin pasar del numero de nucleos) y ademas el total de nucleos
  std::vector<int> Hilos;
  for (int t = 1; t <= 8 && t <= NProcs; t *= 2) { Hilos.push_back(t); }
  if (Hilos.back() != NProcs) { Hilos.push_back(NProcs); }

  std::vector<double> TPar(Hilos.size());
  std::vector<double> Diff(Hilos.size());
  TIntFltH PRankPar;
  for (size_t h = 0; h < Hilos.size(); h++) {
    omp_set_num_threads(Hilos[h]);
    TSnap::GetPageRankMP(G, PRankPar, C, EPS, MAX_ITER);  // calentamiento
    Log("Paralelo (GetPageRankMP) con %d hilo(s):\n", Hilos[h]);
    double Suma = 0;
    for (int r = 0; r < Reps; r++) {
      t0 = omp_get_wtime();
      TSnap::GetPageRankMP(G, PRankPar, C, EPS, MAX_ITER);
      const double t = omp_get_wtime() - t0;
      Log("  corrida %d: %.4f s\n", r + 1, t);
      Suma += t;
    }
    TPar[h] = Suma / Reps;
    // ---------------- 5. Verificacion ----------------
    Diff[h] = MaxDiff(PRankSeq, PRankPar);
    Log("  promedio:  %.4f s   | dif. maxima vs secuencial: %.3e\n\n", TPar[h], Diff[h]);
  }

  // ---------------- 6. Top-10 ----------------
  TFltIntPrV Orden;
  double SumaPR = 0;
  for (int i = PRankSeq.FFirstKeyId(); PRankSeq.FNextKeyId(i); ) {
    Orden.Add(TFltIntPr(PRankSeq[i], PRankSeq.GetKey(i)));
    SumaPR += PRankSeq[i];
  }
  Orden.Sort(false);  // descendente por PageRank
  Log("=== Top-10 nodos por PageRank ===\n");
  Log("%4s %10s %14s %10s %10s\n", "#", "Nodo", "PageRank", "GradoEnt", "GradoSal");
  for (int i = 0; i < 10 && i < Orden.Len(); i++) {
    const int NId = Orden[i].Val2;
    TNGraph::TNodeI NI = G->GetNI(NId);
    Log("%4d %10d %14.8f %10d %10d\n", i + 1, NId, Orden[i].Val1.Val, NI.GetInDeg(), NI.GetOutDeg());
  }
  Log("Suma de todos los PageRank: %.6f (debe ser ~1)\n\n", SumaPR);

  // ---------------- 7. Tabla de resultados ----------------
  FILE* Csv = fopen((Dir + "resultados.csv").c_str(), "w");
  if (Csv != NULL) { fprintf(Csv, "hilos,tiempo_s,speedup,eficiencia,dif_max\n"); }
  Log("=== Tiempos (promedio de %d corridas) ===\n", Reps);
  Log("T secuencial = %.4f s\n", TSeq);
  Log("+-------+------------+---------+------------+\n");
  Log("| Hilos | Tiempo (s) | Speedup | Eficiencia |\n");
  Log("+-------+------------+---------+------------+\n");
  for (size_t h = 0; h < Hilos.size(); h++) {
    const double Speedup = TSeq / TPar[h];
    const double Efic = Speedup / Hilos[h];
    Log("| %5d | %10.4f | %7.2f | %9.1f%% |\n", Hilos[h], TPar[h], Speedup, 100.0 * Efic);
    if (Csv != NULL) { fprintf(Csv, "%d,%.6f,%.4f,%.4f,%.3e\n", Hilos[h], TPar[h], Speedup, Efic, Diff[h]); }
  }
  Log("+-------+------------+---------+------------+\n");
  if (Csv != NULL) { fclose(Csv); }
  Log("Guardado en %s: salida.txt, resultados.csv, grados_entrada.csv, grados_salida.csv\n", Dir.c_str());
  if (Salida != NULL) { fclose(Salida); }

  return 0;
}
