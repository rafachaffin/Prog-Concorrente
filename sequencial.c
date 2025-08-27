#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "timer.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Deve informar a dimensao e o nome do arquivo\n");
        printf("Exemplo: 1000 1000_dim.bin\n");
        return 1;
    }
    
    long int dimensao = atol(argv[1]);
    const char *filename = argv[2];
    
    if (dimensao <= 0) {
        printf("Erro: dimensao deve ser um numero positivo\n");
        return 1;
    }
    
    // Aloca memória para os vetores
    double *vector1 = (double *)malloc(dimensao * sizeof(double));
    double *vector2 = (double *)malloc(dimensao * sizeof(double));
    
    if (!vector1 || !vector2) {
        fprintf(stderr, "Erro: falha na alocacao de memoria\n");
        return 1;
    }
    
    // Inicializa gerador de números aleatórios
    srand(time(NULL));
    
    // Preenche os vetores com valores aleatórios
    for (long int i = 0; i < dimensao; i++) {
        // Gera valores entre 0 e 999.99
        vector1[i] = (double)(rand() % 1000) + (double)(rand() % 100) / 100.0;
        vector2[i] = (double)(rand() % 1000) + (double)(rand() % 100) / 100.0;
    }
    
    // Mede tempo de execução do cálculo do produto interno
    double inicio, fim, tempo;
    double produto_interno = 0.0;
    
    GET_TIME(inicio);
    
    // Calcula produto interno
    for (long int i = 0; i < dimensao; i++) {
        produto_interno += vector1[i] * vector2[i];
    }
    
    GET_TIME(fim);
    tempo = fim - inicio;
    
    // Abre arquivo para escrita binária
    FILE *file = fopen(filename, "wb");
    if (!file) {
        fprintf(stderr, "Erro: nao foi possivel abrir o arquivo %s\n", filename);
        free(vector1);
        free(vector2);
        return 1;
    }
    
    // Escreve N, vetores e resultado sequencial
    fwrite(&dimensao, sizeof(long int), 1, file);
    fwrite(vector1, sizeof(double), dimensao, file);
    fwrite(vector2, sizeof(double), dimensao, file);
    fwrite(&produto_interno, sizeof(double), 1, file);
    
    fclose(file);

    printf("\n=-=-=-=-=-=-=-=-=-=-=-=-= RESULTADOS =-=-=-=-=-=-=-=-=-=-=-=-=\n");
    printf("Vetores gerados com sucesso!\n");
    printf("Dimensao: %ld\n", dimensao);
    printf("Produto interno sequencial: %.6f\n", produto_interno);
    printf("Tempo de execucao: %.6f segundos (%.3f ms)\n", tempo, tempo * 1000);
    printf("Arquivo salvo: %s\n", filename);
    printf("=-=-=-=-=-=-=-=-=-=-=-=-= SEQUENCIAL =-=-=-=-=-=-=-=-=-=-=-=-=\n");
    
    // Libera memória
    free(vector1);
    free(vector2);
    
    return 0;
}
