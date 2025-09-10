/* Disciplina: Programacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Codigo: Comunicação entre threads usando variável compartilhada e exclusao mutua com bloqueio */
/* Aluno: Rafael Oliveira Chaffin */
/* DRE: 121045260 */
#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>

long int soma = 0; //variavel compartilhada entre as threads
pthread_mutex_t mutex; //variavel de lock para exclusao mutua
pthread_mutex_t sync_mutex; //mutex para sincronização
pthread_cond_t cond_print; //variável de condição para sincronização
int print_ready = 0; //flag para indicar se pode imprimir
int threads_paused = 0; //flag para indicar se threads estão pausadas
int all_threads_finished = 0; //flag para indicar se todas as threads ExecutaTarefa terminaram

//funcao executada pelas threads
void *ExecutaTarefa (void *arg) {
  long int id = (long int) arg;
  printf("Thread : %ld esta executando...\n", id);

  for (int i=0; i<100000; i++) {
     //--entrada na SC
     pthread_mutex_lock(&mutex);
     //--SC (seção critica)
     soma++; //incrementa a variavel compartilhada 
     
     // Verifica se soma é múltiplo de 1000
     if (soma % 1000 == 0) {
       // Sinaliza que precisa imprimir
       pthread_mutex_lock(&sync_mutex);
       print_ready = 1;
       threads_paused = 1;
       pthread_cond_signal(&cond_print); // Notifica a thread extra
       
       // Pausa até a thread extra terminar de imprimir
       while (print_ready) {
         pthread_cond_wait(&cond_print, &sync_mutex);
       }
       threads_paused = 0;
       pthread_mutex_unlock(&sync_mutex);
     }
     
     //--saida da SC
     pthread_mutex_unlock(&mutex);
  }
  printf("Thread : %ld terminou!\n", id);
  pthread_exit(NULL);
}

//funcao executada pela thread de log
void *extra (void *args) {
  printf("Extra : esta executando...\n");
  
  while (1) {
    pthread_mutex_lock(&sync_mutex);
    
    // Aguarda sinal para imprimir
    while (!print_ready) {
      pthread_cond_wait(&cond_print, &sync_mutex);
    }
    
    // Imprime o valor atual de soma
    printf("soma = %ld \n", soma);
    
    // Sinaliza que terminou de imprimir
    print_ready = 0;
    pthread_cond_signal(&cond_print); // Notifica as threads pausadas
    pthread_mutex_unlock(&sync_mutex);
    
    // Verifica se todas as threads ExecutaTarefa terminaram
    if (all_threads_finished) {
      break;
    }
  }
  
  printf("Extra : terminou!\n");
  pthread_exit(NULL);
}

//fluxo principal
int main(int argc, char *argv[]) {
   pthread_t *tid; //identificadores das threads no sistema
   int nthreads; //qtde de threads (passada linha de comando)

   //--le e avalia os parametros de entrada
   if(argc<2) {
      printf("Digite: %s <numero de threads>\n", argv[0]);
      return 1;
   }
   nthreads = atoi(argv[1]);

   //--aloca as estruturas
   tid = (pthread_t*) malloc(sizeof(pthread_t)*(nthreads+1));
   if(tid==NULL) {puts("ERRO--malloc"); return 2;}

   //--inicializa o mutex (lock de exclusao mutua)
   pthread_mutex_init(&mutex, NULL);
   pthread_mutex_init(&sync_mutex, NULL);
   pthread_cond_init(&cond_print, NULL);

   //--cria as threads
   for(long int t=0; t<nthreads; t++) {
     if (pthread_create(&tid[t], NULL, ExecutaTarefa, (void *)t)) {
       printf("--ERRO: pthread_create()\n"); exit(-1);
     }
   }

   //--cria thread de log
   if (pthread_create(&tid[nthreads], NULL, extra, NULL)) {
      printf("--ERRO: pthread_create()\n"); exit(-1);
   }

   //--espera as threads ExecutaTarefa terminarem
   for (int t=0; t<nthreads; t++) {
     if (pthread_join(tid[t], NULL)) {
         printf("--ERRO: pthread_join() \n"); exit(-1); 
     } 
   }
   
   //--sinaliza que todas as threads ExecutaTarefa terminaram
   pthread_mutex_lock(&sync_mutex);
   all_threads_finished = 1;
   pthread_cond_signal(&cond_print); // Notifica a thread extra para terminar
   pthread_mutex_unlock(&sync_mutex);
   
   //--espera a thread extra terminar
   if (pthread_join(tid[nthreads], NULL)) {
       printf("--ERRO: pthread_join() \n"); exit(-1); 
   } 

   //--finaliza o mutex
   pthread_mutex_destroy(&mutex);
   pthread_mutex_destroy(&sync_mutex);
   pthread_cond_destroy(&cond_print);
   
   printf("Valor de 'soma' = %ld\n", soma);

   return 0;
}
