/*
 The second program processor.c creates an output file digits.out and waits for user input to be sent by the receiver program.
 As soon as one line is received from the receiver, it counts the number of digits in that line and dumps the digit count along with the original line in the digits.out file.This program also runs in an infinite loop.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    int running = 1;
    char *shm_p = NULL;
    int shmid;
    int semid;
    int value;
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_flg = SEM_UNDO;
    FILE *file = NULL;
    char buffer[128];
    /*open a file for dumping the information received*/
    /* create the digits.out */
    if ((file = fopen("./digits.out", "w+")) == NULL)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }
    /*create sem signal and shared memory with the same key id*/
    /* create sem signal */
    if ((semid = semget((key_t)123457, 1, 0666|IPC_CREAT)) == -1)
    {
        perror("semget");
        exit(EXIT_FAILURE);
    }
    
    /* create shared memory */
    shmid = shmget((key_t)654321, (size_t)2048, 0600|IPC_CREAT);
    if (shmid == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    
    shm_p = shmat(shmid, NULL, 0);
    if (shm_p == NULL)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    /* As soon as one line is received from the receiver, it counts the number of digits in that line and dumps the digit count along with the original line in the digits.out file.This program runs in an infinite loop.
     */
    while(running)
    {
        if ((value = semctl(semid, 0, GETVAL)) == 1)
        {
            memset(&buffer[0], 0, 128);
            printf("read data:");

            printf("%s\n", shm_p);
            sprintf(&buffer[0], "%d:", strlen(shm_p));
            fwrite(&buffer[0], 1, strlen(buffer), file);
            fwrite(shm_p, 1, strlen(shm_p), file);
            fwrite("\n", 1, 1, file);

            sem_b.sem_op = -1;
            if (semop(semid, &sem_b, 1) == -1)
            {
                fprintf(stderr, "semaphore operation failed.\n");
                exit(EXIT_FAILURE);
            }
        }
        /*When receive the information “end”, the processor will quit.*/
        if (strcmp(shm_p, "end") == 0)
        {
            running = 0;
        }
       
    }
    
    shmdt(shm_p);
    
    
    if (shmctl(shmid, IPC_RMID, 0) != 0)
    {
        perror("shmctl");
        exit(EXIT_FAILURE);
    }
    
    if (semctl(semid, 0, IPC_RMID, 0) != 0)
    {
        perror("shmctl");
        exit(EXIT_FAILURE);
    }
    
    fclose(file);
    
    return 0;
}