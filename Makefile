# Makefile para pagerank.cpp (SNAP + OpenMP)
# Banderas tomadas de ~/snap/Makefile.config (Linux)
# -isystem en lugar de -I: oculta los avisos que vienen de las cabeceras de SNAP

SNAP_DIR ?= $(HOME)/snap
CSNAP    = $(SNAP_DIR)/snap-core
CGLIB    = $(SNAP_DIR)/glib-core
CSNAPADV = $(SNAP_DIR)/snap-adv

CC       = g++
# -fopenmp define _OPENMP, y glib-core/base.h define USE_OPENMP a partir de ahi
# (por eso existe TSnap::GetPageRankMP).
CXXFLAGS = -std=c++98 -Wall -O3 -DNDEBUG -fopenmp
LIBS     = -lrt

MAIN    = pagerank
DATASET ?= datos/web-Google.txt

all: $(MAIN)

$(MAIN): $(MAIN).cpp $(CSNAP)/Snap.o
	$(CC) $(CXXFLAGS) -o $(MAIN) $(MAIN).cpp $(CSNAP)/Snap.o \
		-isystem $(CSNAP) -isystem $(CGLIB) -isystem $(CSNAPADV) $(LIBS)

$(CSNAP)/Snap.o:
	$(MAKE) -C $(CSNAP)

# Corre una red y grafica sus resultados (resultados/<red>/)
run: $(MAIN)
	./$(MAIN) $(DATASET)
	python3 graficar.py resultados/$(basename $(notdir $(DATASET)))

# Corre las 4 redes de datos/ y grafica todo
todos: $(MAIN)
	for f in datos/*.txt; do ./$(MAIN) $$f || exit 1; done
	python3 graficar.py

clean:
	rm -f $(MAIN) *.o

.PHONY: all run todos clean
