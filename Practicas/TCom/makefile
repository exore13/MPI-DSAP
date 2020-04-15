COMP = mpicc
OPT2 = -O2
OPT3 = -O3
OPTS = ${COMP} ${OPT2}
LM = -lm

INPUT = tcom.c
OUTPUT = tcom

# Por defecto dejamos la opcion de optimizacion -O2 por ser menos intrusiva
all:
	${OPTS} -o ${OUTPUT} ${INPUT} ${LM}

op3:
	${COMP} ${OPT3} -o ${OUTPUT} ${INPUT} ${LM}

op0:
	${COMP} -o ${OUTPUT} ${INPUT} ${LM}

clean: 
	rm ${OUTPUT}
