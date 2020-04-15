// Author:         Tanase, Dragos
// Date:           6 Feb 2020
// Title:          MPI Communication Time 
//
//                 DSAP P2

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

const int NUM_ITERACIONES = 1000;

// Swaps two integer pointers
void swap(long double *a, long double *b)  
{  
    double aux = *a;  
    *a = *b;  
    *b = aux;  
}  

int main(int argc, char **argv)
{
    MPI_Status status;
    int ierr, rank, nThreads;
    long double *tiempos;
    char *calcByte = "%";
    
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nThreads);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(rank == 0)
    {
        //Calculo de beta
        tiempos = (long double *)malloc(sizeof(long double) * NUM_ITERACIONES);
        for(unsigned int i = 0; i < NUM_ITERACIONES; i++)
        {
            long double tiempoStart = MPI_Wtime();

            MPI_Send(&calcByte, 1, MPI_BYTE, 1, 1, MPI_COMM_WORLD);
            MPI_Recv(&calcByte, 1, MPI_BYTE, 1, 1, MPI_COMM_WORLD, &status);
            
            tiempos[i] = ((MPI_Wtime() - tiempoStart) / 2);
        }
    }

    else if(rank == 1)
    {
        //Calculo de beta
        for(unsigned int i = 0; i < NUM_ITERACIONES; i++)
        {
            MPI_Recv(&calcByte, 1, MPI_BYTE, 0, 1, MPI_COMM_WORLD, &status);
            MPI_Send(&calcByte, 1, MPI_BYTE, 0, 1, MPI_COMM_WORLD);
            
        }
    }
    MPI_Finalize();

    if(rank != 0)     
        return 0;           


    
    
    // TODO
    // Cambiar burbuja por un quicksort

    // Ordenar el array
    for(unsigned int i = 0; i < NUM_ITERACIONES-1; i++)
        for(unsigned int j = 0; j < NUM_ITERACIONES-1; j++)
            if(tiempos[j] > tiempos[j+1])
                swap(&tiempos[j], &tiempos[j+1]);
    
    //// Calculo valores atipicos
    int q1 = NUM_ITERACIONES/4;
    int q3 = (3 * NUM_ITERACIONES) / 4;
    double iqr = tiempos[q3] - tiempos[q1];
    double limInferior = tiempos[q1] - iqr * 1.5;
    double limSuperior = tiempos[q3] + iqr * 1.5;

    int validos = 0;
    long double beta = 0;
    for(unsigned int i = 0; i < NUM_ITERACIONES; i++)
        if(tiempos[i] >= limInferior && tiempos[i] <= limSuperior) 
        {
            beta += tiempos[i];
            validos++;
        }
    beta = beta * 1000000 / validos;    


    printf("Tiempo total por transmision es de %Lf microsegundos\n", beta);

    
    return 0;
}