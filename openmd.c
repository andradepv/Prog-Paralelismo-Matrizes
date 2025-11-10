# include <sys/time.h>
# include <stdio.h>
# include <stdlib.h>
# include <omp.h>

int TAM_MATRIZES[] = {10, 20, 40, 80, 160, 320, 640, 1280};
int num_tam = sizeof(TAM_MATRIZES) / sizeof(TAM_MATRIZES[0]);

void main(void) {
    struct timeval start, end;
    long mtime, seconds, useconds;

    FILE *file = fopen("resultado_OpenMP.csv", "w");
    if(file == NULL){
        printf("Erro ao abrir arquivo!");
    }

    fprintf(file, "\xEF\xBB\xBF");
    fprintf(file, "Tamanho da matriz, Tempo (milisegundos)\n");

    printf("Quantidade de processadores: %d\n", omp_get_max_threads());
    
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

        // Inicio do tempo
        gettimeofday(&start, NULL);

        #pragma omp parallel for
        for (int linha=0; linha < TAM_MATRIZ; linha++){
            for (int coluna=0; coluna < TAM_MATRIZ; coluna++){
                for ( int x=0; x < TAM_MATRIZ; x++){
                    prod[linha][coluna] += m1[linha][x] * m2[x][coluna];
                }
            }
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
    }
    fclose(file);
}