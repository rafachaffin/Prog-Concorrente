#include "timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

// Estrutura para argumentos das threads
typedef struct {
    long int n;
    double *vector1;
    double *vector2;
    int nthreads;
    int id;
} t_args;

// Função executada por cada thread para calcular produto interno concorrente
void *produto_interno_concorrente(void *args) {
    t_args *arg = (t_args *)args;
    long int inicio, fim, fatia;
    double soma_parcial = 0.0;
    double *retorno;
    
    // Calcula fatia de trabalho para esta thread
    fatia = arg->n / arg->nthreads;
    inicio = arg->id * fatia;
    fim = inicio + fatia;
    
    // Última thread pega o resto
    if (arg->id == (arg->nthreads - 1)) {
        fim = arg->n;
    }
    
    // Calcula produto interno parcial
    for (long int i = inicio; i < fim; i++) {
        soma_parcial += arg->vector1[i] * arg->vector2[i];
    }
    
    // Aloca memória para retorno
    retorno = (double *)malloc(sizeof(double));
    if (retorno != NULL) {
        *retorno = soma_parcial;
    } else {
        fprintf(stderr, "Erro: falha na alocação de memória na thread\n");
        exit(1);
    }
    
    free(args);
    pthread_exit((void *)retorno);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Deve informar o nome do arquivo e o numero de threads\n");
        printf("Exemplo: 10_dim.bin 4\n");
        return 1;
    }
    
    const char *filename = argv[1];
    int nthreads = atoi(argv[2]);
    
    if (nthreads <= 0) {
        printf("Erro: numero de threads deve ser positivo\n");
        return 1;
    }
    
    // Abre arquivo para leitura
    FILE *file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Erro: nao foi possivel abrir o arquivo %s\n", filename);
        return 1;
    }
    
    long int n;
    double *vector1, *vector2;
    double produto_sequencial;
    
    // Lê dimensão e vetores do arquivo
    fread(&n, sizeof(long int), 1, file);
    vector1 = (double *)malloc(n * sizeof(double));
    vector2 = (double *)malloc(n * sizeof(double));
    
    if (!vector1 || !vector2) {
        fprintf(stderr, "Erro: falha na alocacao de memoria\n");
        fclose(file);
        return 1;
    }
    
    fread(vector1, sizeof(double), n, file);
    fread(vector2, sizeof(double), n, file);
    fread(&produto_sequencial, sizeof(double), 1, file);
    fclose(file);
    
    printf("Dimensao dos vetores: %ld\n", n);
    printf("Numero de threads: %d\n", nthreads);
    printf("Produto interno sequencial: %.6f\n", produto_sequencial);
    
    // Ajusta número de threads se necessário
    if (nthreads > n) {
        nthreads = n;
        printf("Ajustando para %d threads\n", nthreads);
    }
    
    // Aloca arrays para threads
    pthread_t *tids = (pthread_t *)malloc(nthreads * sizeof(pthread_t));
    if (!tids) {
        fprintf(stderr, "Erro: falha na alocacao de memoria para threads\n");
        free(vector1);
        free(vector2);
        return 1;
    }
    
    // Mede tempo de execução
    double inicio, fim, tempo;
    GET_TIME(inicio);
    
    // Cria threads
    for (int i = 0; i < nthreads; i++) {
        t_args *args = (t_args *)malloc(sizeof(t_args));
        args->id = i;
        args->n = n;
        args->vector1 = vector1;
        args->vector2 = vector2;
        args->nthreads = nthreads;
        
        if (pthread_create(&tids[i], NULL, produto_interno_concorrente, (void *)args)) {
            fprintf(stderr, "Erro: falha na criacao da thread %d\n", i);
            free(vector1);
            free(vector2);
            free(tids);
            return 1;
        }
    }
    
    // Coleta resultados das threads
    double produto_concorrente = 0.0;
    double *resultado_parcial;
    
    for (int i = 0; i < nthreads; i++) {
        if (pthread_join(tids[i], (void **)&resultado_parcial)) {
            fprintf(stderr, "Erro: falha no join da thread %d\n", i);
            free(vector1);
            free(vector2);
            free(tids);
            return 1;
        }
        produto_concorrente += *resultado_parcial;
        free(resultado_parcial);
    }
    
    GET_TIME(fim);
    tempo = fim - inicio;
    
    // Calcula erro relativo
    double erro_relativo = fabs(produto_sequencial - produto_concorrente) / produto_sequencial;
    
    printf("\n=-=-=-=-=-=-=-=-=-=-=-=-= RESULTADOS =-=-=-=-=-=-=-=-=-=-=-=-=\n");
    printf("Produto interno sequencial: %.6f\n", produto_sequencial);
    printf("Produto interno concorrente: %.6f\n", produto_concorrente);
    printf("Erro relativo: %.10f\n", erro_relativo);
    printf("Tempo de execucao: %.6f segundos (%.3f ms)\n", tempo, tempo * 1000);
    printf("=-=-=-=-=-=-=-=-=-=-=-=-= CONCORRENTE =-=-=-=-=-=-=-=-=-=-=-=-=\n");
    
    // Libera memória
    free(vector1);
    free(vector2);
    free(tids);
    
    return 0;
}
