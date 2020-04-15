// Author:         Tanase, Dragos
// Date:           2 Mar 2020
// Title:          Floyd Warshall MPI Implementation
//
//                 DSAP P5

// Si se le pasa por algumento algun numero, el codigo lo tomara
// como el numero de vertices a tratar. Si no se pasa ninguno
// se tomara por defecto el valor 5.

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

const int KMAXNPROCS = 8;       // Maximo numero de procesos
const int KMAXN = 1000;    // Maximo numero de Vertices en el grafo

// Codigo copiado del ejemplo del profesor.
// Crea un array bidimensional asegurandose de que las posiciones
// reservadas en memoria son contiguas. Tipo INT.
int **Crear_matriz_caminos_consecutivo(int Filas, int Columnas)
{
    // crea un array de 2 dimensiones en posiciones contiguas de memoria
    int *mem_matriz;
    int **matriz;
    int fila, col;
    if (Filas <=0) 
        {
            printf("El numero de filas debe ser mayor que cero\n");
            return 0;
        }
    if (Columnas <= 0)
        {
            printf("El numero de filas debe ser mayor que cero\n");
            return 0;
        }
    mem_matriz = malloc(Filas * Columnas * sizeof(int));
    if (mem_matriz == NULL) 
        {
            printf("Insuficiente espacio de memoria\n");
            return 0;
        }
    matriz = malloc(Filas * sizeof(int *));
    if (matriz == NULL) 
        {
            printf ("Insuficiente espacio de memoria\n");
            return 0;
        }
    for (fila=0; fila<Filas; fila++)
        matriz[fila] = mem_matriz + (fila*Columnas);

    return matriz;
}

// Codigo copiado del ejemplo del profesor.
// Crea un array bidimensional asegurandose de que las posiciones
// reservadas en memoria son contiguas. Tipo FLOAT.
float **Crear_matriz_pesos_consecutivo(int Filas, int Columnas)
{
    // crea un array de 2 dimensiones en posiciones contiguas de memoria
    float *mem_matriz;
    float **matriz;
    int fila, col;
    if (Filas <=0) 
        {
            printf("El numero de filas debe ser mayor que cero\n");
            return 0;
        }
    if (Columnas <= 0)
        {
            printf("El numero de filas debe ser mayor que cero\n");
            return 0;
        }
    mem_matriz = malloc(Filas * Columnas * sizeof(float));
    if (mem_matriz == NULL) 
        {
            printf("Insuficiente espacio de memoria\n");
            return 0;
        }
    matriz = malloc(Filas * sizeof(float *));
    if (matriz == NULL) 
        {
            printf ("Insuficiente espacio de memoria\n");
            return 0;
        }
    for (fila=0; fila<Filas; fila++)
        matriz[fila] = mem_matriz + (fila*Columnas);

    return matriz;
}

// Codigo copiado del ejemplo del profesor.
// 
void Definir_Grafo(int n,float **dist,int **caminos)
{
// Inicializamos la matriz de pesos y la de caminos para el algoritmos de Floyd-Warshall. 
// Un 0 en la matriz de pesos indica que no hay arco.
// Para la matriz de caminos se supone que los vertices se numeran de 1 a n.
  int i,j;
  for (i = 0; i < n; ++i) {
      for (j = 0; j < n; ++j) {
          if (i==j)
             dist[i][j]=0;
          else {
             dist[i][j]= (11.0 * rand() / ( RAND_MAX + 1.0 )); // aleatorios 0 <= dist < 11
             dist[i][j] = ((double)((int)(dist[i][j]*10)))/10; // truncamos a 1 decimal
             if (dist[i][j] < 2) dist[i][j]=0; // establecemos algunos a 0 
          }
          if (dist[i][j] != 0)
             caminos[i][j] = i+1;
          else
             caminos[i][j] = 0;
      }
  }
}

void printMatrizCaminos(int **a, int fila, int col) {
        int i, j;
        char buffer[10];
        printf("     ");
        for (i = 0; i < col; ++i){
                j=sprintf(buffer, "%c%d",'V',i+1 );
                printf("%5s", buffer);
       }
        printf("\n");
        for (i = 0; i < fila; ++i) {
                j=sprintf(buffer, "%c%d",'V',i+1 );
                printf("%5s", buffer);
                for (j = 0; j < col; ++j)
                        printf("%5d", a[i][j]);
                printf("\n");
        }
        printf("\n");
}

void printMatrizPesos(float **a, int fila, int col) {
        int i, j;
        char buffer[10];
        printf("     ");
        for (i = 0; i < col; ++i){
                j=sprintf(buffer, "%c%d",'V',i+1 );
                printf("%5s", buffer);
       }
        printf("\n");
        for (i = 0; i < fila; ++i) {
                j=sprintf(buffer, "%c%d",'V',i+1 );
                printf("%5s", buffer);
                for (j = 0; j < col; ++j)
                        printf("%5.1f", a[i][j]);
                printf("\n");
        }
        printf("\n");
}

/* Desasignamos un puntero a un array 2d*/
void deallocate_array(double **array, int row_dim)
{
    int i;
    for(i=1; i<row_dim; i++)
        array[i]=NULL;

    free(array[0]);
    free(array);
}


int main(int argc, char **argv)
{
    MPI_Status status;
    int ierr, rank, nThreads;
    int n = 0, **caminos;
    float **dist;
    
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
            printf("El numero de procesos no puede ser (%d).\nSe conservarán (%d) procesos.\n", nThreads, KMAXNPROCS);

        else if(rank >= KMAXNPROCS)
        {
            MPI_Finalize();
            return 0;
        }
       
        nThreads = KMAXNPROCS;
    }

    // Obtencion de N
    if(rank == 0)
    {
        if(argc == 1)
        {
            printf("Introduce numero de vertices del grafo: \n");
            if(!scanf("%d", &n))
                printf("Error en introduccion de datos, puede\nque el funcionamiento no sea el deseado...\n");

            if(n > KMAXN)
            {
                printf("El numero de vertices (%d) sobrepasa el maximo (%d)\nNos quedamos con %d\n", n, KMAXN, KMAXN);
                n = KMAXN;
            }

            else
                printf("El grafo tendra %d vertices.\n", n);
        }

        else
        {
            n = atoi(argv[1]);
            printf("Numero de vertices en el grafo: %d\n", n);
        
            if(n > KMAXN)
            {
                printf("El numero de vertices (%d) sobrepasa el maximo (%d)\nNos quedamos con %d\n", n, KMAXN, KMAXN);
                n = KMAXN;
            }
        }
    }


    // Mando el numero de vertices a los esclavos
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    printf("Soy proceso %d y ya tengo n\n", rank);
    MPI_Barrier(MPI_COMM_WORLD);

    // Reserva memoria

    // Calculo porciones
    int nSlave = (n / nThreads);
    int nMaster = (n - (nSlave * (nThreads - 1)));

    // Leyenda para el Broadcast dentro de FW
    int *reparto = (int *)malloc(sizeof(int) * nThreads);

    // Necesario para el scatterv
    int *sendCounts = (int *)malloc(sizeof(int) * nThreads);
    int *displs = (int *)malloc(sizeof(int) * nThreads);

    // Preparar variables donde se guardara la informacion del broadcast y scatter
    int **auxCaminos;
    float **auxPesos;
    int **auxGrafo;

    // Asignacion de memoria
    if(rank == 0)
    {
        auxCaminos = Crear_matriz_caminos_consecutivo(nMaster, n);
        auxPesos = Crear_matriz_pesos_consecutivo(nMaster, n);

        auxGrafo = Crear_matriz_caminos_consecutivo(nMaster, n);

        // Preparo tambien las matrices donde iran los datos finales
        dist = Crear_matriz_pesos_consecutivo(n, n);
        caminos = Crear_matriz_caminos_consecutivo(n, n);

        Definir_Grafo(n, dist, caminos);
        if (n <= 10) 
        {
            printMatrizPesos(dist,n,n);
            printMatrizCaminos(caminos,n,n);
        }
    }

    else
    {
        auxCaminos = Crear_matriz_caminos_consecutivo(nSlave, n);
        auxPesos = Crear_matriz_pesos_consecutivo(nSlave, n);

        auxGrafo = Crear_matriz_caminos_consecutivo(nSlave, n);
    }

    if(reparto == NULL || sendCounts == NULL || displs == NULL)
        printf("Error en reserva de memoria, hilo %d\n", rank);

    else
        printf("Reserva de memoria bien. (%d)\n", rank);


    // Balance de carga a los distintos procesos
    // Creo un array de n elementos, el numero que se guarde en cada
    // posicion sera el hilo del programa que tiene esa informacion,
    // esto sera usado durante el broadcast interior del algoritmo
    // para simular el salto de fila
    

    // Calculo de a que proceso se le asigna cada fila. Se usara en mitad
    // del algoritmo para los broadcast
    int rankAssignement = 1, auxRank = 1;
    for(int i = 0; i < n; i++)
    {
        if(i < nMaster)
            reparto[i] = 0;

        else
        {
            reparto[i] = rankAssignement;

            if(auxRank < nSlave)
                auxRank++;

            else
            {
                auxRank = 1;
                rankAssignement++;
            }
        }
    }

    // Clculo de sendCounts. Array nThread size, donde se almacena 
    // cuantas filas se le asigna a cada proceso. Necesario para scatterv
    // Calculo de displs. Array nThread size donde se almacena los offset
    // entre la informacion a enviar de la matriz principal.

    sendCounts[0] = nMaster;
    sendCounts[1] = nSlave;
    displs[0] = 0;
    displs[1] = nMaster;
    for(int i = 2; i < nThreads; i++)
    {
        sendCounts[i] == nSlave;            // Cuanto envias a cada proceso
        displs[i] = displs[i-1] + nSlave;   // Offset calc
    }
    
    printf("@@@ %d todo bien @@@\n", rank);
   
    
    


    // FW MPI

    // for(int k = 0; k < n; k++)
    // {




    // }
    printf("\nSoy proceso %d y acabo\n", rank);
    // TODO
    // //Liberacion de memoria
    // if(rank == 0)    
    // {
    //    deallocate_array(dist, n);
    //    deallocate_array(caminos, n);;
    // }

    //     free(auxCaminos);
    //     free(auxPesos);
    //     free(auxGrafo);

    MPI_Finalize();
    return 0;
}