# include <sys/time.h>
# include <stdio.h>
# include <stdlib.h>
# include <pthread.h>
#include <unistd.h>


int TAM_MATRIZES[] = {5000, 5000, 5000};
int num_tam = sizeof(TAM_MATRIZES) / sizeof(TAM_MATRIZES[0]);

// Informações para as threads
typedef struct {
    int id;
    int **m1;
    int **m2;
    int **prod;
    int TAM_MATRIZ;
    int num_procs;
} thread_data;

// Corpo da Thread
void * T_Prod_Matriz(void * arg){
    thread_data *data = (thread_data *)arg;

    int id = data->id;
    int **m1 = data->m1;
    int **m2 = data->m2;
    int **prod = data->prod;
    int TAM_MATRIZ = data->TAM_MATRIZ;
    int num_procs = data->num_procs;

    int begin = (id * TAM_MATRIZ) / num_procs;
    int end = ((id + 1) * TAM_MATRIZ) / num_procs;

    if (id == num_procs - 1) {
        end = TAM_MATRIZ;
    }

    for (int linha = begin; linha < end; linha++){
        for (int coluna=0; coluna < TAM_MATRIZ; coluna++){
            for ( int x=0; x < TAM_MATRIZ; x++){
                prod[linha][coluna] += m1[linha][x] * m2[x][coluna];
            }
        }
    }
    pthread_exit(NULL);
}

void main(void) {
    struct timeval start, end;
    long mtime, seconds, useconds;

    long num_procs = sysconf(_SC_NPROCESSORS_ONLN);
    if (num_procs < 1) {
        // Fallback caso a chamada falhe
        num_procs = 1;
    }

    printf("Número de processadores detectado: %ld\n", num_procs);

    FILE *file = fopen("resultados_paralelo.csv", "w");
    if(file == NULL){
        printf("Erro ao abrir arquivo!");
    }

    fprintf(file, "\xEF\xBB\xBF");
    fprintf(file, "Tamanho da matriz, Tempo (milisegundos)\n");
    
    for (int i=0; i < num_tam; i++){
        int TAM_MATRIZ = TAM_MATRIZES[i];

        // Declaração das matrizes
        int **m1 = malloc(TAM_MATRIZ * sizeof(int *));
        int **m2 = malloc(TAM_MATRIZ * sizeof(int *));
        int **prod = malloc(TAM_MATRIZ * sizeof(int *));

        for (int row = 0; row < TAM_MATRIZ; row++) {
            m1[row] = malloc(TAM_MATRIZ * sizeof(int));
            m2[row] = malloc(TAM_MATRIZ * sizeof(int));
            prod[row] = malloc(TAM_MATRIZ * sizeof(int));
        }

        // Inicializando matrizes
        for (int j=0; j < TAM_MATRIZ; j++){
            for (int k=0; k < TAM_MATRIZ; k++){
                m1[j][k] = j;
                m2[j][k] = 2;
                prod[j][k] = 0;
            }
        }

        pthread_t threads[num_procs];
        thread_data thread_args[num_procs];

        // Inicio do tempo
        gettimeofday(&start, NULL);

        // Cria as threads
        for (int p=0; p<num_procs; p++){
            thread_args[p].id = p;
            thread_args[p].m1 = m1;
            thread_args[p].m2 = m2;
            thread_args[p].prod = prod;
            thread_args[p].TAM_MATRIZ = TAM_MATRIZ;
            thread_args[p].num_procs =  num_procs;
            pthread_create(&threads[p], NULL, T_Prod_Matriz, (void *)&thread_args[p]);
        }

        // Espera as threads terminarem
        for (int p=0; p<num_procs; p++){
            pthread_join(threads[p], NULL);
        }

        // Final do tempo
        gettimeofday(&end, NULL);

        // Tempo final
        seconds = end.tv_sec - start.tv_sec;
        useconds = end.tv_usec - start.tv_usec;
        mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;

        
        fprintf(file, "%d, %lu\n", TAM_MATRIZ, mtime);

        // Exibir tempo e matriz produto
        printf("Tempo para a matriz: %d\n", TAM_MATRIZ);
        printf("Tempo transcorrido: %lu milisegundos\n", mtime);

        for (int row = 0; row < TAM_MATRIZ; row++) {
            free(m1[row]);
            free(m2[row]);
            free(prod[row]);
        }
        free(m1);
        free(m2);
        free(prod);

        sleep(60);
        printf("Rodando novamente");
    }
    fclose(file);
}