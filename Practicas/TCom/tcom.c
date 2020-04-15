// Author:         Tanase, Dragos
// Date:           6 Feb 2020
// Title:          MPI Communication Time 
//
//                 DSAP P2

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

const int KNUM_ITERACIONES = 1000;
const int KNUM_CALCULOS = 12;

// Quicksort encontrado por internet y modificado para trabajar
// con long doubles en lugar de enteros
// Fuente: https://www.geeksforgeeks.org/quick-sort/

void swap(long double *a, long double *b)  
{  
    long double aux = *a;  
    *a = *b;  
    *b = aux;  
} 

int partition (long double arr[], int low, int high) 
{ 
    long double pivot = arr[high];
    int i = (low - 1);
  
    for (int j = low; j <= high- 1; j++) 
    { 
        if (arr[j] < pivot) 
        { 
            i++;
            swap(&arr[i], &arr[j]); 
        } 
    } 
    swap(&arr[i + 1], &arr[high]); 
    return (i + 1); 
} 

void quickSort(long double *arr, int low, int high) 
{ 
    if (low < high) 
    { 
        int pi = partition(arr, low, high); 
  
        quickSort(arr, low, pi - 1); 
        quickSort(arr, pi + 1, high); 
    } 
}  

// Funcion que almacena en el valor calculos el numero esperado
void createCalculos(int *calculos)
{
    int powOffset = 5;

    for(int i = 0; i < KNUM_CALCULOS; i++)
        calculos[i] = pow(2, powOffset + i);
}

int main(int argc, char **argv)
{
    MPI_Status status;
    int ierr, rank, nThreads;

    double *datos;
    char calcByte = '%';
    long double **tiempos;
    

    int *calculos = (int *)malloc(sizeof(int) * KNUM_CALCULOS);
    createCalculos(calculos);

    
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &nThreads);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(nThreads < 2)
    {
        printf("Debe haber como minimo 2 hilos para ejecutar este programa.\n");
        MPI_Finalize();
        free(calculos);
        return 0;
    }

    if(rank == 0)
    {
        tiempos = malloc(sizeof(long double *) * (KNUM_CALCULOS + 1));

        // Medicion enviando un byte
        tiempos[0] = malloc(sizeof(long double) * KNUM_ITERACIONES);
        for(int i = 0; i < KNUM_ITERACIONES; i++)
        {
            long double tiempoStart = MPI_Wtime();

            MPI_Send(&calcByte, 1, MPI_BYTE, 1, 1, MPI_COMM_WORLD);
            MPI_Recv(&calcByte, 1, MPI_BYTE, 1, 1, MPI_COMM_WORLD, &status);
            
            tiempos[0][i] = ((MPI_Wtime() - tiempoStart) / 2);
        }
        printf("Terminado envio de un Byte\n");
        

        // Mediciones enviando desde 256bytes hasta 4MB
        for(int i = 0; i < KNUM_CALCULOS; i++)
        {
            // Asignamos memoria para el array donde vayamos a guardar los tiempos
            tiempos[i+1] = malloc(sizeof(long double) * KNUM_ITERACIONES);

            // Asignamos memoria de los datos a enviar. Estos iran incrementandose
            // con el indice i
            datos = malloc(sizeof(double) * calculos[i]);

            // Bucle donde se realizaran los envios y recepciones de datos junto
            // a las mediciones de tiempo
            for(int j = 0; j < KNUM_ITERACIONES; j++)
            {
                long double tiempoStart = MPI_Wtime();

                MPI_Send(datos, calculos[i], MPI_DOUBLE, 1, 1, MPI_COMM_WORLD);
                MPI_Recv(datos, calculos[i], MPI_DOUBLE, 1, 1, MPI_COMM_WORLD, &status);

                tiempos[i+1][j] = ((MPI_Wtime() - tiempoStart) / 2);
            }
            printf("Terminado el envio %d de %d bytes\n", i, calculos[i]*8);
            
            free(datos);
        }
    }

    else if(rank == 1)
    {
        // Medicion del envio de un byte
        for(unsigned int i = 0; i < KNUM_ITERACIONES; i++)
        {
            MPI_Recv(&calcByte, 1, MPI_BYTE, 0, 1, MPI_COMM_WORLD, &status);
            MPI_Send(&calcByte, 1, MPI_BYTE, 0, 1, MPI_COMM_WORLD);
        }

         // Mediciones enviando desde 256bytes hasta 4MB
        for(int i = 0; i < KNUM_CALCULOS; i++)
        {
            // Preparamos el array para recibir los datos
            datos = malloc(sizeof(double) * calculos[i]);

            // Bucle donde se realizaran los envios y recepciones de datos
            for(int j = 0; j < KNUM_ITERACIONES; j++)
            {
                MPI_Recv(datos, calculos[i], MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &status);
                MPI_Send(datos, calculos[i], MPI_DOUBLE, 0, 1, MPI_COMM_WORLD);
            }

            free(datos);
        }
    }
    MPI_Finalize();

    // Cualquier proceso que no sea el master acaba aqui
    if(rank != 0)
    {
        free(calculos);
        return 0;
    }       

    //Procesos que no sean el master siguen
    long double *resultados =   (long double *)malloc(sizeof(long double) * (KNUM_CALCULOS + 1));
    long double *tau =          (long double *)malloc(sizeof(long double) * KNUM_CALCULOS);
    int *validos =              (int *)malloc(sizeof(int) * (KNUM_CALCULOS + 1));

    int q1 = KNUM_ITERACIONES/4;
    int q3 = (3 * KNUM_ITERACIONES) / 4;

    // Recorrera todos los tiempos
    for(int i = 0; i < (KNUM_CALCULOS + 1); i++)
    {
        // Ordeno los arrays para poder quitar los valores atipicos
        quickSort(tiempos[i], 0, KNUM_ITERACIONES-1);
            
        // Una vez ordenados los arrays, calculo los valores que haran de limites
        // superior e inferior, para poder filtrar los atipicos
        double iqr = tiempos[i][q3] - tiempos[i][q1];
        double limInferior = tiempos[i][q1] - iqr * 1.5;
        double limSuperior = tiempos[i][q3] + iqr * 1.5;

        validos[i] = 0;
        resultados[i] = 0;

        // Cuando un valor es valido, este se suma a resultados[i], y se incrementa el contador
        for(int iter = 0; iter < KNUM_ITERACIONES; iter++)
            if(tiempos[i][iter] >= limInferior && tiempos[i][iter] <= limSuperior)
            {
                resultados[i] += tiempos[i][iter];
                validos[i]++;
            }

        // Los resultados habra que expresarlos en microsegundos, por lo tanto los
        // multiplicamos por 10^6 y despues lo partimos por el numero de valores validos.
        resultados[i] = resultados[i] * pow(10, 6) / validos[i];

        // Tau es el numero necesario para enviar un byte. Este numero ira decrementando
        // cuanto suba el tamaño total del mensaje.
        if(i != 0)
            tau[i-1] = (resultados[i] - resultados[0]) / (calculos[i-1] * 8 );
    }


    for(int i = 0; i < KNUM_CALCULOS; i++)
        printf("El tiempo necesario para transmitir %d bytes es de %Lf microsegundos. Tau=%Lf\n", (calculos[i] * 8), resultados[i+1], tau[i]);

    // Liberar la memoria reservada previamente

    for(int i = 0; i < (KNUM_CALCULOS + 1); i++)
        free(tiempos[i]);

    free(calculos);
    free(tiempos);
    free(validos);
    free(tau);

    return 0;
}
