#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <cuda_runtime.h>
#include <unistd.h>
#include <cuda.h>

int TAM_MATRIZES[] = {100, 500, 1000, 5000, 10000, 17000};
int num_tam = sizeof(TAM_MATRIZES) / sizeof(TAM_MATRIZES[0]);

__global__ void multMatrizKernel(int *A, int *B, int *C, int N) {
    int row = blockIdx.y * blockDim.y + threadIdx.y;
    int col = blockIdx.x * blockDim.x + threadIdx.x;

    if (row < N && col < N) {
        int soma = 0;
        for (int k = 0; k < N; k++) {
            soma += A[row * N + k] * B[k * N + col];
        }
        C[row * N + col] = soma;
    }
}

int main(void) {
    FILE *file = fopen("resultados_preusogpu.csv", "w");
    if (file == NULL) {
        printf("Erro ao abrir arquivo de saída!\n");
        return 1;
    }

    fprintf(file, "\xEF\xBB\xBF");
    fprintf(file, "Tamanho da matriz,Tempo (ms)\n");

    for (int i = 0; i < num_tam; i++) {
        int N = TAM_MATRIZES[i];
        size_t size = N * N * sizeof(int);


        int *h_A = (int *)malloc(size);
        int *h_B = (int *)malloc(size);
        int *h_C = (int *)malloc(size);

        for (int r = 0; r < N; r++) {
            for (int c = 0; c < N; c++) {
                h_A[r * N + c] = 1;
                h_B[r * N + c] = 2;
                h_C[r * N + c] = 0;
            }
        }
        cudaEvent_t start, stop;
        cudaEventCreate(&start);
        cudaEventRecord(start);


        int *d_A, *d_B, *d_C;
        cudaMalloc((void **)&d_A, size);
        cudaMalloc((void **)&d_B, size);
        cudaMalloc((void **)&d_C, size);

        cudaMemcpy(d_A, h_A, size, cudaMemcpyHostToDevice);
        cudaMemcpy(d_B, h_B, size, cudaMemcpyHostToDevice);


        int BLOCK_SIZE = 32;
        dim3 threads(BLOCK_SIZE, BLOCK_SIZE);
        dim3 grid((N + BLOCK_SIZE - 1) / BLOCK_SIZE, (N + BLOCK_SIZE - 1) / BLOCK_SIZE);


        cudaEventCreate(&stop);



        multMatrizKernel<<<grid, threads>>>(d_A, d_B, d_C, N);
        cudaDeviceSynchronize();

        cudaEventRecord(stop);
        cudaEventSynchronize(stop);
        float milliseconds = 0;
        cudaEventElapsedTime(&milliseconds, start, stop);

        cudaMemcpy(h_C, d_C, size, cudaMemcpyDeviceToHost);

        printf("Matriz de tamanho: %d \n", N);
        printf("Tempo GPU: %f ms\n", milliseconds);
        fprintf(file, "%d, %f\n", N, milliseconds);

        free(h_A);
        free(h_B);
        free(h_C);
        cudaFree(d_A);
        cudaFree(d_B);
        cudaFree(d_C);

        cudaEventDestroy(start);
        cudaEventDestroy(stop);

        sleep(10);
        printf("Rodando novamente \n");
    }

    fclose(file);
    printf("\nTodos os testes concluídos!\n");
    return 0;
}
