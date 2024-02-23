/*
Integrantes
- Jorge Vinícius Lourenço Santos
- Matheus de Farias Santos
- Adailton Moura da Silva
*/

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define MAX_SIZE 10

pthread_mutex_t mutex;
pthread_cond_t full, empty;

typedef struct
{
    int vector_clock[3];
} ClockVector;

ClockVector buffer[MAX_SIZE];
int in = 0, out = 0;

void print_clock(char *action, char *thread_name, ClockVector clock)
{
    printf("%s %s o relógio: [", thread_name, action);
    for (int i = 0; i < 3; ++i)
    {
        printf("%d", clock.vector_clock[i]);
        if (i < 2)
            printf(", ");
    }
    printf("]\n");
}

void generate_clock(ClockVector *clock, int thread_id)
{
    srand(time(NULL) +
          thread_id); // Adiciona o ID da thread ao seed para diferentes relógios
    for (int i = 0; i < 3; ++i)
    {
        clock->vector_clock[i] = rand() % 100; // Ajuste conforme necessário
    }
}

void *produtor(void *arg)
{
    int thread_id = *((int *)arg);
    free(arg);

    while (1)
    {
        // produzir relógio vetorial aleatório
        ClockVector new_clock;
        generate_clock(&new_clock, thread_id);

        pthread_mutex_lock(&mutex);

        while (((in + 1) % MAX_SIZE) == out)
        {
            // Fila cheia, aguarda espaço
            printf("Fila cheia. Thread produtora %c%d aguardando...\n", 'P',
                   thread_id);
            pthread_cond_wait(&empty, &mutex);
        }

        // inserir relógio vetorial na fila
        buffer[in] = new_clock;
        in = (in + 1) % MAX_SIZE;

        pthread_mutex_unlock(&mutex);

        pthread_cond_signal(&full); // Sinaliza que há um item na fila

        char thread_name[3];
        snprintf(thread_name, sizeof(thread_name), "P%d", thread_id);
        print_clock("produziu", thread_name, new_clock);

        sleep(1); // produz a cada 2 segundos
    }
}

void *consumidor(void *arg)
{
    int thread_id = *((int *)arg);
    free(arg);

    while (1)
    {
        pthread_mutex_lock(&mutex);

        while (in == out)
        {
            // Fila vazia, aguarda item
            printf("Fila vazia. Thread consumidora %c%d aguardando...\n", 'C',
                   thread_id);
            pthread_cond_wait(&full, &mutex);
        }

        // consumir relógio vetorial da fila
        ClockVector consumed_clock = buffer[out];
        out = (out + 1) % MAX_SIZE;

        pthread_mutex_unlock(&mutex);

        pthread_cond_signal(&empty); // Sinaliza que há um espaço na fila

        char thread_name[3];
        snprintf(thread_name, sizeof(thread_name), "C%d", thread_id);
        print_clock("consumirá", thread_name, consumed_clock);

        sleep(2); // consome a cada 1 segundo
    }
}

int main()
{
    // Inicialização dos mutex e variáveis de condição
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&full, NULL);
    pthread_cond_init(&empty, NULL);

    // Criação de threads
    pthread_t produtor_threads[3], consumidor_threads[3];
    int *thread_ids;

    for (int i = 0; i < 3; ++i)
    {
        thread_ids = malloc(sizeof(int));
        *thread_ids = i + 1;

        pthread_create(&produtor_threads[i], NULL, produtor, (void *)thread_ids);
        pthread_create(&consumidor_threads[i], NULL, consumidor,
                       (void *)thread_ids);
    }

    // Espera as threads terminarem (o que nunca acontecerá)
    for (int i = 0; i < 3; ++i)
    {
        pthread_join(produtor_threads[i], NULL);
        pthread_join(consumidor_threads[i], NULL);
    }

    // Liberação dos recursos
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&full);
    pthread_cond_destroy(&empty);

    return 0;
}