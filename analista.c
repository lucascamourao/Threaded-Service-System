#include <stdio.h>
#include <semaphore.h>
#include <fcntl.h>

int main()
{
    sem_t *sem_block = sem_open("/sem_block", O_RDWR);

    sem_wait(sem_block);
    FILE *lng = fopen("lista_numeros_gerados.txt", "r");

    int count = 0;
    int pid;
    while (count < 10 && fscanf(lng, "%d", &pid) == 1)
    {
        printf("PID: %d\n", pid);
        count++;
    }

    // Truncar arquivo após imprimir
    fclose(lng);
    lng = fopen("lista_numeros_gerados.txt", "w");
    fclose(lng);

    sem_post(sem_block);

    return 0;
}