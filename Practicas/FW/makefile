COMP = mpicc
OPT2 = -O2
OPT3 = -O3
OPTS = ${COMP} ${OPT2}

INPUT = fw.c
OUTPUT = fw

# Por defecto dejamos la opcion de optimizacion -O2 por ser menos intrusiva
all:
	${OPTS} -o ${OUTPUT} ${INPUT}

op3:
	${COMP} ${OPT3} -o ${OUTPUT} ${INPUT}

op0:
	${COMP} -o ${OUTPUT} ${INPUT}

clean: 
	rm ${OUTPUT}
