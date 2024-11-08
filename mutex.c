#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define BUF_SIZE 3 // Define o tamanho máximo do buffer

// Estrutura que define o buffer compartilhado entre os produtores e consumidores
typedef struct {
    int buf[BUF_SIZE]; // Array de inteiros que representa o buffer
    size_t len;        // Quantidade de itens atualmente no buffer
    int total_items;   // Total de itens a serem produzidos/consumidos
    int produced_items; // Contador de itens produzidos
    int consumed_items; // Contador de itens consumidos
    pthread_mutex_t mutex; // Mutex para garantir acesso exclusivo ao buffer
    pthread_cond_t can_produce; // Variável de condição para sinalizar quando pode produzir
    pthread_cond_t can_consume; // Variável de condição para sinalizar quando pode consumir
} buffer_t;

// Função executada pelas threads produtoras
void* producer(void *arg) {
    buffer_t *buffer = (buffer_t*)arg;

    while (1) { 
        pthread_mutex_lock(&buffer->mutex); // Trava o mutex para garantir exclusividade

        // Verifica se já produziu o total de itens necessário
        if (buffer->produced_items >= buffer->total_items) {
            pthread_mutex_unlock(&buffer->mutex); // Destrava o mutex
            break; // Sai do loop e encerra a thread produtora
        }

        // Se o buffer estiver cheio, aguarda até que haja espaço
        while (buffer->len == BUF_SIZE) {
            printf("Buffer está cheio!\n");
            pthread_cond_wait(&buffer->can_produce, &buffer->mutex); // Espera a condição de poder produzir
        }

        int item = rand() % 11; // Produz um número aleatório entre 0 e 10
        printf("Produzido: %d\n", item);

        buffer->buf[buffer->len] = item; // Coloca o item produzido no buffer
        ++buffer->len; // Incrementa o número de itens no buffer
        ++buffer->produced_items; // Incrementa o contador de itens produzidos

        pthread_cond_signal(&buffer->can_consume); // Sinaliza que há itens para consumir
        pthread_mutex_unlock(&buffer->mutex); // Destrava o mutex para liberar o buffer

        sleep(1); // Simula o tempo de produção com uma pausa de 1 segundo
    }

    return NULL; 
}

// Função executada pelas threads consumidoras
void* consumer(void *arg) {
    buffer_t *buffer = (buffer_t*)arg;

    while (1) { 
        pthread_mutex_lock(&buffer->mutex); // Trava o mutex para garantir exclusividade

        // Verifica se já consumiu o total de itens necessário
        if (buffer->consumed_items >= buffer->total_items) {
            pthread_mutex_unlock(&buffer->mutex); // Destrava o mutex
            break; // Sai do loop e encerra a thread consumidora
        }

        // Se o buffer estiver vazio, aguarda até que haja itens para consumir
        while (buffer->len == 0) {
            printf("Buffer está vazio!\n");
            pthread_cond_wait(&buffer->can_consume, &buffer->mutex); // Espera a condição de poder consumir
        }

        --buffer->len; // Decrementa o número de itens no buffer
        printf("Consumido: %d\n", buffer->buf[buffer->len]); // Consome o item do buffer
        ++buffer->consumed_items; // Incrementa o contador de itens consumidos

        pthread_cond_signal(&buffer->can_produce); // Sinaliza que há espaço para produzir
        pthread_mutex_unlock(&buffer->mutex); // Destrava o mutex para liberar o buffer

        sleep(1); // Simula o tempo de consumo com uma pausa de 1 segundo
    }

    return NULL; 
}

// Função principal do programa
int main(int argc, char *argv[]) {
    srand(time(NULL)); // Gera números aleatórios com base no tempo atual
    
    // Verifica se o número correto de argumentos foi passado
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <número de produtores> <número de consumidores> <total de itens>\n", argv[0]);
        return 1; // Sai com erro se o número de argumentos for incorreto
    }

    // Converte os argumentos da linha de comando para inteiros
    int num_producers = atoi(argv[1]);
    int num_consumers = atoi(argv[2]);
    //int total_items = atoi(argv[3]);
    int total_items = 1;

    // Inicializa a estrutura do buffer
    buffer_t buffer = {
        .len = 0, // O buffer começa vazio
        .total_items = total_items, // Define o número total de itens a serem produzidos/consumidos
        .produced_items = 0, // Inicializa o contador de itens produzidos
        .consumed_items = 0, // Inicializa o contador de itens consumidos
        .mutex = PTHREAD_MUTEX_INITIALIZER, // Inicializa o mutex
        .can_produce = PTHREAD_COND_INITIALIZER, // Inicializa a variável condicional para produção
        .can_consume = PTHREAD_COND_INITIALIZER  // Inicializa a variável condicional para consumo
    };

    // Vetores para armazenar as threads produtoras e consumidoras
    pthread_t producers[num_producers];
    pthread_t consumers[num_consumers];

    // Cria as threads produtoras
    for (int i = 0; i < num_producers; ++i) {
        pthread_create(&producers[i], NULL, producer, (void*)&buffer); // Inicia cada thread produtora
    }

    // Cria as threads consumidoras
    for (int i = 0; i < num_consumers; ++i) {
        pthread_create(&consumers[i], NULL, consumer, (void*)&buffer); // Inicia cada thread consumidora
    }

    // Aguarda que todas as threads produtoras terminem
    for (int i = 0; i < num_producers; ++i) {
        pthread_join(producers[i], NULL);
    }

    // Aguarda que todas as threads consumidoras terminem
    for (int i = 0; i < num_consumers; ++i) {
        pthread_join(consumers[i], NULL);
    }

    printf("O programa chegou ao fim.\n"); // Mensagem ao final do programa

    return 0; // Retorno da função principal
}