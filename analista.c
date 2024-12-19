#include <stdio.h>
#include <semaphore.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

int main()
{
    // Open the semaphore for blocking file access
    sem_t *sem_block = sem_open("/sem_block", O_RDWR);
    if (sem_block == SEM_FAILED)
    {
        perror("Erro ao abrir semáforo");
        return 1;
    }

    // Wait to acquire the block semaphore
    sem_wait(sem_block);

    // Open the lista_numeros_gerados.txt file
    FILE *lng = fopen("lista_numeros_gerados.txt", "r");
    if (lng == NULL)
    {
        perror("Erro ao abrir arquivo de lista");
        sem_post(sem_block);
        return 1;
    }

    // Create a temporary file to store remaining PIDs
    FILE *temp = fopen("temp_lista.txt", "w");
    if (temp == NULL)
    {
        perror("Erro ao criar arquivo temporário");
        fclose(lng);
        sem_post(sem_block);
        return 1;
    }

    int count = 0;
    printf("Analista imprimindo PIDs:\n");
    char linha[256]; // Buffer para armazenar cada linha do arquivo

    // Print first 10 PIDs or all if less than 10
    while (count < 10 && fgets(linha, sizeof(linha), lng))
    {
        // Remove newline if present
        linha[strcspn(linha, "\n")] = '\0';

        // Check if the line already has a status
        if (strstr(linha, " - ") == NULL)
        {
            strcat(linha, " - ?");
        }

        printf("PID: %s\n", linha);
        count++;
    }

    // Copy remaining PIDs to temporary file, preserving their full format
    while (fgets(linha, sizeof(linha), lng))
    {
        fprintf(temp, "%s", linha); // Write the complete line including status
    }

    // Close both files
    fclose(lng);
    fclose(temp);

    // Replace original file with temporary file
    rename("temp_lista.txt", "lista_numeros_gerados.txt");

    // Release the block semaphore
    sem_post(sem_block);
    return 0;
}