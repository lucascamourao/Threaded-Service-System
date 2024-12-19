#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/time.h>
#include <string.h>
#include <sys/stat.h>

#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME 0
#endif

#define PRIORIDADE_ALTA 1
#define PRIORIDADE_BAIXA 0
#define TEMPO_ESPERA_BASE 50000 // 50ms base para espera

#define MAX_FILA 100
#define MAX_TENTATIVAS 5

// Structures and type definitions
typedef struct
{
    pid_t pid;
    int hora_chegada;
    int prioridade;
    int tempo_atendimento;
} Cliente;

typedef struct Node
{
    Cliente cliente;
    struct Node *next;
} Node;

typedef struct
{
    Node *end;
    int size;
} ListaCircular;

// Global variables
sem_t *sem_atend = SEM_FAILED;
sem_t *sem_block = SEM_FAILED;
ListaCircular fila;
int total_clientes = 0, clientes_satisfeitos = 0;
struct timeval tempo_inicial, tempo_atual;
int flag_parar = 0;
int contador_atendimentos = 0;

struct timespec start_time, end_time;
double elapsed_time;

// Mutex para proteger o acesso ao arquivo
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;
// Mutex para proteger a fila (NÃO FUNCIONA)
// pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;

// Function prototypes
void initLista(ListaCircular *lista);
int isEmpty(ListaCircular *lista);
void enqueue(ListaCircular *lista, Cliente cliente);
Cliente dequeue(ListaCircular *lista);
void safe_sem_close(sem_t **sem, const char *name);
void handle_signal(int sig);
void *thread_menu(void *arg);
int verificar_arquivo_demanda(int tentativas_maximas);
void *thread_recepcao(void *arg);
void *thread_atendente(void *arg);

// List circular implementation
void initLista(ListaCircular *lista)
{
    lista->end = NULL;
    lista->size = 0;
}

int isEmpty(ListaCircular *lista)
{
    return lista->end == NULL;
}

void enqueue(ListaCircular *lista, Cliente cliente)
{
    Node *newNode = (Node *)malloc(sizeof(Node));
    if (!newNode)
    {
        perror("Falha ao alocar memória");
        exit(1);
    }
    newNode->cliente = cliente;

    if (isEmpty(lista))
    {
        newNode->next = newNode;
        lista->end = newNode;
    }
    else
    {
        newNode->next = lista->end->next;
        lista->end->next = newNode;
        lista->end = newNode;
    }
    lista->size++;
}

Cliente dequeue(ListaCircular *lista)
{
    if (isEmpty(lista))
    {
        perror("Tentativa de remover de uma lista vazia");
        exit(1);
    }

    Node *front = lista->end->next;
    Cliente cliente = front->cliente;

    if (front == lista->end)
    {
        lista->end = NULL;
    }
    else
    {
        lista->end->next = front->next;
    }

    free(front);
    lista->size--;
    return cliente;
}

// Utility functions
void safe_sem_close(sem_t **sem, const char *name)
{
    if (*sem != SEM_FAILED)
    {
        sem_close(*sem);
        sem_unlink(name);
        *sem = SEM_FAILED;
    }
}

void handle_signal(int sig)
{
    printf("Recebido sinal de interrupção. Encerrando...\n");
    flag_parar = 1;
}

void *thread_menu(void *arg)
{
    char ch;
    printf("Pressione 's' para encerrar o programa.\n");
    while ((ch = getchar()) != 's')
        ;

    // Chamar o analista uma última vez para garantir a leitura de todos os PIDs restantes
    sem_wait(sem_block); // Adicionando semáforo para garantir acesso seguro

    FILE *lng = fopen("lista_numeros_gerados.txt", "r");
    char linha[256];

    if (lng != NULL)
    {
        printf("\nImprimindo PIDs restantes:\n");
        while (fgets(linha, sizeof(linha), lng))
        {
            printf("PID: %s", linha); // Imprime a linha
        }
        fclose(lng);
    }

    sem_post(sem_block); // Liberando o semáforo

    printf("Encerrando o programa...\n");
    flag_parar = 1;
    return NULL;
}

// Verificar arquivo de demanda
int verificar_arquivo_demanda(int tentativas_maximas)
{
    int tentativas = 0;
    while (tentativas < tentativas_maximas)
    {
        FILE *demanda = fopen("demanda.txt", "r");
        if (demanda != NULL)
        {
            fclose(demanda);
            return 1;
        }

        usleep(50000);
        tentativas++;
    }
    return 0;
}

// Thread de recepção
void *thread_recepcao(void *arg)
{
    int N = *(int *)arg;
    int clientes_gerados = 0;
    srand(time(NULL));

    // Controle de taxa de geração de clientes prioritários
    int taxa_prioritarios = 0;

    while (!flag_parar)
    {
        if (fila.size >= MAX_FILA)
        {
            usleep(TEMPO_ESPERA_BASE);
            continue;
        }

        if (N != 0 && clientes_gerados >= N)
            break;

        // Ajusta a probabilidade de clientes prioritários
        int deve_ser_prioritario = (taxa_prioritarios < (clientes_gerados * 0.4)); // 40% prioritários

        pid_t pid = fork();
        if (pid < 0)
        {
            perror("Fork failed");
            break;
        }

        if (pid == 0)
        {
            execl("./cliente", "cliente", NULL);
            perror("Exec failed");
            exit(1);
        }

        usleep(TEMPO_ESPERA_BASE);
        sem_wait(sem_block);

        // Leitura do tempo de atendimento com retry
        int tempo_atendimento = 0;
        int leitura_sucesso = 0;
        int tentativas = 0;

        while (tentativas < MAX_TENTATIVAS && !leitura_sucesso)
        {
            FILE *demanda = fopen("demanda.txt", "r");
            if (demanda != NULL)
            {
                if (fscanf(demanda, "%d", &tempo_atendimento) == 1)
                {
                    leitura_sucesso = 1;
                }
                fclose(demanda);
            }
            if (!leitura_sucesso)
            {
                usleep(TEMPO_ESPERA_BASE / 5);
                tentativas++;
            }
        }

        if (!leitura_sucesso)
        {
            kill(pid, SIGKILL);
            sem_post(sem_block);
            continue;
        }

        struct timeval current;
        gettimeofday(&current, NULL);
        int hora_chegada = (current.tv_sec - tempo_inicial.tv_sec) * 1000 +
                           (current.tv_usec - tempo_inicial.tv_usec) / 1000;

        int prioridade = deve_ser_prioritario ? PRIORIDADE_ALTA : PRIORIDADE_BAIXA;
        if (prioridade == PRIORIDADE_ALTA)
            taxa_prioritarios++;

        Cliente novo_cliente = {
            .pid = pid,
            .hora_chegada = hora_chegada,
            .prioridade = prioridade,
            .tempo_atendimento = tempo_atendimento};

        enqueue(&fila, novo_cliente);
        total_clientes++;

        printf("Cliente gerado. PID: %d, Prioridade: %s\n",
               pid, prioridade == PRIORIDADE_ALTA ? "Alta" : "Baixa");

        sem_post(sem_block);
        clientes_gerados++;

        // Ajusta o tempo de espera baseado no tamanho da fila
        usleep(TEMPO_ESPERA_BASE * (1 + (fila.size / 10)));
    }

    return NULL;
}

// Thread de atendente
void *thread_atendente(void *arg)
{
    int X = *(int *)arg;
    struct timeval current;

    while (!flag_parar || fila.size > 0)
    {
        sem_wait(sem_block);

        if (isEmpty(&fila))
        {
            sem_post(sem_block);
            usleep(TEMPO_ESPERA_BASE);
            continue;
        }

        Cliente cliente = dequeue(&fila);
        sem_post(sem_block);

        if (kill(cliente.pid, 0) != 0)
        {
            printf("Cliente PID %d já não existe.\n", cliente.pid);
            continue;
        }

        kill(cliente.pid, SIGCONT);

        struct timespec ts;
        if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
        {
            struct timeval tv;
            gettimeofday(&tv, NULL);
            ts.tv_sec = tv.tv_sec;
            ts.tv_nsec = tv.tv_usec * 1000;
        }

        ts.tv_sec += 3;

        int sem_ret = sem_timedwait(sem_atend, &ts);
        if (sem_ret == -1)
        {
            if (errno == ETIMEDOUT)
            {
                printf("[TIMEOUT] Semáforo não liberado para PID: %d\n", cliente.pid);
                continue;
            }
            perror("Erro no semáforo de atendimento");
            continue;
        }

        gettimeofday(&current, NULL);
        int tempo_espera = (current.tv_sec - tempo_inicial.tv_sec) * 1000 +
                           (current.tv_usec - tempo_inicial.tv_usec) / 1000 -
                           cliente.hora_chegada;

        // Cliente prioritário tem metade da paciência (50%)
        int paciencia = (cliente.prioridade == 1) ? (X / 2) : X;

        // Verifica satisfação baseada estritamente no tempo de espera vs paciência
        int cliente_satisfeito = (tempo_espera <= paciencia);

        // Log do atendimento com informações detalhadas
        if (cliente.prioridade == 1)
        {
            printf("Atendendo cliente prioritário. PID: %d, Tempo espera: %d ms, Paciência: %d ms\n",
                   cliente.pid, tempo_espera, paciencia);
        }
        else
        {
            printf("Atendendo cliente normal. PID: %d, Tempo espera: %d ms, Paciência: %d ms\n",
                   cliente.pid, tempo_espera, paciencia);
        }

        pthread_mutex_lock(&file_mutex);
        FILE *lng = fopen("lista_numeros_gerados.txt", "a");
        if (lng)
        {
            if (cliente_satisfeito)
            {
                fprintf(lng, "%d - Satisfeito\n", cliente.pid);
                clientes_satisfeitos++;
            }
            else
            {
                fprintf(lng, "%d - Insatisfeito\n", cliente.pid);
            }
            fclose(lng);
        }
        else
        {
            perror("Erro ao abrir o arquivo de saída");
        }
        pthread_mutex_unlock(&file_mutex);

        sem_post(sem_atend);

        // Controle de análise a cada 10 atendimentos
        struct stat st;
        if (++contador_atendimentos % 10 == 0)
        {
            system("./analista &");
        }
        else if (stat("arquivo.txt", &st) == 0)
        {
            if (st.st_size == 0)
            {
                continue;
            }
            else
            {
                system("./analista &");
            }
        }

        // Pequena pausa entre atendimentos
        usleep(TEMPO_ESPERA_BASE / 2);
    }
    return NULL;
}

// Main function
int main(int argc, char *argv[])
{
    double elapsed_time = 0;
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    sem_unlink("/sem_atend");
    sem_unlink("/sem_block");
    remove("demanda.txt");
    remove("lista_numeros_gerados.txt");

    if (argc != 3)
    {
        printf("Uso: %s <numero_clientes> <tempo_paciencia>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    int X = atoi(argv[2]);
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    while (!flag_parar && elapsed_time < 5)
    {
        sem_atend = sem_open("/sem_atend", O_CREAT | O_EXCL, 0644, 1);
        sem_block = sem_open("/sem_block", O_CREAT | O_EXCL, 0644, 1);
        if (sem_atend == SEM_FAILED || sem_block == SEM_FAILED)
        {
            perror("Falha ao criar semáforos");
            return 1;
        }

        initLista(&fila);

        pthread_t thread_rec, thread_atend, thread_menu_t;
        pthread_create(&thread_rec, NULL, thread_recepcao, &N);
        pthread_create(&thread_atend, NULL, thread_atendente, &X);
        pthread_create(&thread_menu_t, NULL, thread_menu, NULL);

        pthread_join(thread_rec, NULL);
        pthread_join(thread_atend, NULL);
        pthread_join(thread_menu_t, NULL);

        safe_sem_close(&sem_atend, "/sem_atend");
        safe_sem_close(&sem_block, "/sem_block");

        clock_gettime(CLOCK_MONOTONIC, &end_time);
        elapsed_time = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1e9;

        if ((contador_atendimentos == total_clientes) || (elapsed_time) > 5.0)
        {
            flag_parar = 1;
            break;
        }
    }

    double satisfeitos_porcentagem = (double)clientes_satisfeitos / total_clientes * 100;

    printf("Programa finalizado. Clientes satisfeitos: %d/%d\n", clientes_satisfeitos, total_clientes);
    printf("Taxa de Satisfação: %.2f%%.\n", satisfeitos_porcentagem);
    printf("Tempo de execução: %f segundos\n", elapsed_time);

    return 0;
}