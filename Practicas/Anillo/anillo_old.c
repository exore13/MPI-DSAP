// Author:         Tanase, Dragos
// Date:           29 Jan 2020
// Title:          MPI Ring test

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char **argv)
{
    MPI_Status status;
    int ierr, rank, nThreads;
    int *dato = (int *)malloc(sizeof(int));

    
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nThreads);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(nThreads < 2)
    {
        printf("Debe haber como minimo 2 hilos para probar el anillo.\n");
        free(dato);
        return 0;
    }

    if(rank == 0)
    {
        // TODO
        // Hay que cambiar la entrada por una que compruebe que sea int
        printf("Introduce un entero para transmitir:\n");
        scanf("%d", &dato[0]);
        printf("El entero introducido es %d\n", dato[0]);

        // Envia el dato introducido al segundo hilo del MPI_COMM_WORLD
        MPI_Send(dato, 1, MPI_INT, 1, 1, MPI_COMM_WORLD);

        // Pasa a esperar el dato que le enviara el ultimo hilo
        MPI_Recv(dato, 1, MPI_INT, nThreads-1, 1, MPI_COMM_WORLD, &status);

        printf("Soy el proceso %d. El entero que he recibido es: %d\n", rank, dato[0]);
    }

    else
    {
        // Los procesos esclavos esperan que el proceso con rango uno menor a
        // ellos les envie el dato
        MPI_Recv(dato, 1, MPI_INT, (rank-1), 1, MPI_COMM_WORLD, &status);
        printf("Soy el proceso %d. El entero que he reciibdo es: %d\n", rank, dato[0]);

        // Incrementa el dato recibido en uno
        dato[0]++;

        // Los procesos esclavos envian el dato recibido al siguiente hilo
        // excepto el esclavo con rango nThreads-1, ese al ser el ultimo
        // debe enviar el dato al Master
        if(rank != (nThreads - 1))
            MPI_Send(dato, 1, MPI_INT, (rank + 1), 1, MPI_COMM_WORLD);

        else
            MPI_Send(dato, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    free(dato);

    return 0;
}