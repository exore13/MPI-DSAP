// Author:         Tanase, Dragos
// Date:           29 Jan 2020
// Title:          MPI Ring test
//
//                 DSAP P1

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char **argv)
{
    MPI_Status status;
    int ierr, rank, nThreads;
    int dato = 0;

    
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nThreads);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(nThreads < 2)
    {
        printf("Debe haber como minimo 2 hilos para probar el anillo.\n");
        return 0;
    }

    if(rank == 0)
    {
        // TODO
        // Hay que cambiar la entrada por una que compruebe que sea int
        printf("Introduce un entero para transmitir:\n");
        if(!scanf("%d", &dato))
            printf("Error en introduccion de datos, puede\nque el funcionamiento no sea el deseado...\n");
  
        printf("El entero introducido es %d\n", dato);


        // Envia el dato introducido al segundo hilo del MPI_COMM_WORLD
        MPI_Send(&dato, 1, MPI_INT, 1, 1, MPI_COMM_WORLD);

        // Pasa a esperar el dato que le enviara el ultimo hilo
        MPI_Recv(&dato, 1, MPI_INT, nThreads-1, 1, MPI_COMM_WORLD, &status);

        printf("Soy el proceso %d. El entero que he recibido es: %d\n", rank, dato);
    }

    else
    {
        // Los procesos esclavos esperan que el proceso con rango uno menor a
        // ellos les envie el dato
        MPI_Recv(&dato, 1, MPI_INT, (rank-1), 1, MPI_COMM_WORLD, &status);
        printf("Soy el proceso %d. El entero que he reciibdo es: %d\n", rank, dato);

        // Incrementa el dato recibido en uno
        dato++;

        // Los procesos esclavos envian el dato recibido al siguiente hilo
        // excepto el esclavo con rango nThreads-1, ese al ser el ultimo
        // debe enviar el dato al Master
        if(rank != (nThreads - 1))
            MPI_Send(&dato, 1, MPI_INT, (rank + 1), 1, MPI_COMM_WORLD);

        else
            MPI_Send(&dato, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}