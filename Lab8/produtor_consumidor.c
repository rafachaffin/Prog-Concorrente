/* Disciplina: Programacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Aluno: Rafael Oliveira Chaffin */
/* DRE: 121045260 */
/* Codigo: Padrão Produtor/Consumidor utilizando semáforos */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <stdbool.h>

// Estrutura para informações das threads consumidoras
typedef struct {
    int id;
    int primos_encontrados;
} ConsumidorInfo;

// Variáveis globais do buffer
int *buffer;
int tamanho_buffer;
int proximo_inserir = 0;
int proximo_remover = 0;
int N;
int num_consumidores;

// Semaforos
sem_t mutex_buffer;
sem_t slots_vazios;
sem_t slots_ocupados;
sem_t mutex_contadores; 

// Variáveis globais para resultados
int total_primos = 0;
int thread_vencedora = -1;
int max_primos = 0;
int numeros_processados = 0;
bool produtor_terminou = false;

// Função para verificar se um número é primo
int ehPrimo(long long int n) {
    if (n <= 1) return 0;
    if (n == 2) return 1;
    if (n % 2 == 0) return 0;
    for (int i = 3; i * i <= n; i += 2) {
        if (n % i == 0) return 0;
    }
    return 1;
}

// Thread produtora
void* produtor(void* arg) {
    int numero_atual = 1;
    while (numero_atual <= N) {
        for (int i = 0; i < tamanho_buffer; i++) {
            sem_wait(&slots_vazios);
        }
        sem_wait(&mutex_buffer);
        int elementos_inseridos = 0;
        for (int i = 0; i < tamanho_buffer && numero_atual <= N; i++) {
            buffer[proximo_inserir] = numero_atual;
            proximo_inserir = (proximo_inserir + 1) % tamanho_buffer;
            elementos_inseridos++;
            numero_atual++;
        }
        sem_post(&mutex_buffer);
        for (int i = 0; i < elementos_inseridos; i++) {
            sem_post(&slots_ocupados);
        }
    }
    sem_wait(&mutex_contadores);
    produtor_terminou = true;
    sem_post(&mutex_contadores);
    for (int i = 0; i < num_consumidores; i++) {
        sem_post(&slots_ocupados);
    }
    return NULL;
}

// Thread consumidora
void* consumidor(void* arg) {
    ConsumidorInfo *info = (ConsumidorInfo*) arg;
    int primos_locais = 0;
    int numero;
    
    while (1) {
        sem_wait(&slots_ocupados);
        sem_wait(&mutex_contadores);
        if (produtor_terminou && numeros_processados == N) {
            sem_post(&mutex_contadores);
            break;
        }
        sem_post(&mutex_contadores);

        sem_wait(&mutex_buffer);
        numero = buffer[proximo_remover];
        proximo_remover = (proximo_remover + 1) % tamanho_buffer;
        sem_post(&mutex_buffer);
        
        sem_wait(&mutex_contadores);
        numeros_processados++;
        sem_post(&mutex_contadores);
        
        sem_post(&slots_vazios);
        
        if (ehPrimo(numero)) {
            primos_locais++;
            printf("Consumidor %d encontrou %d, é primo\n", info->id, numero);
            
        } else {
            printf("Consumidor %d encontrou %d, não é primo\n", info->id, numero);
            
        }
    }
    
    // Atualiza contadores globais no final
    sem_wait(&mutex_contadores);
    total_primos += primos_locais;
    if (primos_locais > max_primos) {
        max_primos = primos_locais;
        thread_vencedora = info->id;
    }
    sem_post(&mutex_contadores);
    
    return NULL;
}

// Função main 
int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Uso: %s <N> <M> <C>\n", argv[0]);
        printf("N: quantidade de números a gerar\n");
        printf("M: tamanho do buffer\n");
        printf("C: número de threads consumidoras\n");
        return 1;
    }
    
    N = atoi(argv[1]);
    int M = atoi(argv[2]);
    num_consumidores = atoi(argv[3]);
    
    if (N <= 0 || M <= 0 || num_consumidores <= 0) {
        printf("Erro: todos os parâmetros devem ser positivos\n");
        return 1;
    }
    
    buffer = malloc(M * sizeof(int));
    tamanho_buffer = M;
    
    sem_init(&mutex_buffer, 0, 1);
    sem_init(&mutex_contadores, 0, 1);
    sem_init(&slots_vazios, 0, M);
    sem_init(&slots_ocupados, 0, 0);
    
    pthread_t thread_produtor;
    pthread_create(&thread_produtor, NULL, produtor, NULL);
    
    pthread_t threads_consumidores[num_consumidores];
    ConsumidorInfo info_consumidores[num_consumidores];
    
    for (int i = 0; i < num_consumidores; i++) {
        info_consumidores[i].id = i + 1;
        info_consumidores[i].primos_encontrados = 0;
        pthread_create(&threads_consumidores[i], NULL, consumidor, &info_consumidores[i]);
    }
    
    pthread_join(thread_produtor, NULL);
    for (int i = 0; i < num_consumidores; i++) {
        pthread_join(threads_consumidores[i], NULL);
    }
    
    printf("\n=== RESULTADOS FINAIS ===\n");
    printf("Total de números primos encontrados: %d\n", total_primos);
    printf("Thread consumidora vencedora: %d (encontrou %d primos)\n", 
           thread_vencedora, max_primos);
    
    free(buffer);
    sem_destroy(&mutex_buffer);
    sem_destroy(&mutex_contadores);
    sem_destroy(&slots_vazios);
    sem_destroy(&slots_ocupados);
    
    return 0;
}
