// Author:         Tanase, Dragos
// Date:           24 Feb 2020
// Title:          MPI Producto Escalar
//
//                 DSAP P3

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

const int KMAXNPROCS = 8;       // Maximo numero de procesos
const int KMAXN = 100000000;    // Maxima dimension para los vectores x e y


int main(int argc, char **argv)
{
    MPI_Status status;
    int ierr, rank, nThreads;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nThreads);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(nThreads < 2)
    {
        printf("Debe haber como minimo 2 hilos para ejecutar este programa.\n");
        MPI_Finalize();
        return 0;
    }

    if(nThreads >= KMAXNPROCS)
    {
        if(rank == 0)
            printf("El numero de procesos no puede ser %d.\nSe conservarán %d procesos.\n", nThreads, KMAXNPROCS);

        else if(rank >= KMAXNPROCS)
        {
            MPI_Finalize();
            return 0;
        }
       
        nThreads = KMAXNPROCS;
    }


    int vSize;
    if(rank == 0)
    {
        printf("Introduce el tamaño de vector deseado:\n");
        if(!scanf("%d", &vSize))
            printf("Error en introduccion de datos, puede\nque el funcionamiento no sea el deseado...\n");
  
        if(vSize > KMAXN)
        {
            printf("El tamaño de los vectores no puede ser %d.\nEstos seran creados con el tamaño maximo permitido %d\n", vSize, KMAXN);
            vSize = KMAXN;
        }      
    }

    // Envia el vSize a todos los hijos
    MPI_Bcast(&vSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
    //printf("Proceso %d tiene un vSize de %d despues de Broadcast\n", rank, vSize);
    if(rank >= vSize)
    {
        MPI_Finalize();
        return 0;
    }

    double *x, *y;

    // Balance de carga a los distintos procesos
    int chunkSlave = (vSize / nThreads);
    int chunkMaster = (vSize - (chunkSlave * (nThreads - 1)));

    if(rank == 0)
    {
        x = (double *)malloc(sizeof(double) * vSize);
        y = (double *)malloc(sizeof(double) * vSize);
        

        // Rellenar x e y
        for(int i = 0; i < vSize; i++)
        {
            x[i] = 1.0 / (i + 1);
            y[i] = 1.0 + i;
        }

        // Enviar la informacion necesaria a cada proceso
        for(int i = 0; i < (nThreads - 1); i++)
        {
            MPI_Send(&x[i*chunkSlave + chunkMaster], chunkSlave, MPI_DOUBLE, i+1, 0, MPI_COMM_WORLD);
            MPI_Send(&y[i*chunkSlave + chunkMaster], chunkSlave, MPI_DOUBLE, i+1, 1, MPI_COMM_WORLD);
            printf("Master acaba de enviar dos arrays a %d\n", i+1);
        }

        // Producto escalar en chunkMaster
        x[0] = x[0]*y[0];
        for(int i = 1; i < chunkMaster; i++)
        {
            x[i] = x[i] * y[i];
            x[0] += x[i];
        }
        printf("Proceso %d ha calculado el producto escalar\n", rank);
    }

    else
    {
        x = (double *)malloc(sizeof(double) * chunkSlave);
        y = (double *)malloc(sizeof(double) * chunkSlave);

        MPI_Recv(x, chunkSlave, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(y, chunkSlave, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &status);
        printf("Proceso %d ha recibido ambos arrays\n", rank);

        // Producto escalar en chunkSlave
        x[0] = x[0]*y[0];
        for(int i = 1; i < chunkSlave; i++)
        {
            x[i] = x[i] * y[i];
            x[0] += x[i];
        }
            
        printf("Proceso %d ha calculado el producto escalar\n", rank);


        MPI_Send(x, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
        printf("Proceso %d ha enviado el resultado del producto escalar\n", rank);
    }
    
    MPI_Barrier(MPI_COMM_WORLD);

    if(rank == 0)
    {
        for(int i = 0; i < (nThreads - 1); i++)
        {
            MPI_Recv(&x[i+1], 1, MPI_DOUBLE, i+1, 0, MPI_COMM_WORLD, &status);
            printf("Master acaba de recibir los datos de %d\n", i+1);
        }

        for(int i = 1; i < nThreads; i++)
            x[0] += x[i];

        printf("El resultado del producto escalar con vSize=%d es de %f\n", vSize, x[0]);
    }
    // if(rank != 0)
    //     MPI_Reduce(&x, &x 
    // else
    //     MPI_Reduce(MPI_IN_PLACE, &x

    free(x);
    free(y);

    MPI_Finalize();
    return 0;
}