

/*
receiver.c runs in an infinite loop receiving alpha numeric strings as input from the user one line at a time.
After reading one line from the standard input, this program sends this information to the other program.
The sharing of data between the two processes should take place via shared memory.
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>

int main(int argc, char *argv[])
{
    int running = 1;
    int shid;
    int semid;
    int value = 0;
    void *sharem = NULL;
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_flg = SEM_UNDO;
    /* Because the receiver and processor may be access the shared memory at the same time, So the two processes should use signal to sync each other.At the beginning, set init signal value to 0*/
    /* create sem signal */
    if ((semid = semget((key_t)123457, 1, 0666|IPC_CREAT)) == -1)
    {
        perror("semget");
        exit(EXIT_FAILURE);
    }
    
    /* set init signal value to 0 */
    if (semctl(semid, 0, SETVAL, 0) == -1)
    {
        printf("sem init error.\n");
        
        /* if set init failed, then delete sem signal */
        if (semctl(semid, 0, IPC_RMID, 0) != 0)
        {
            perror("semctl");
            exit(EXIT_FAILURE);
        }
        
        exit(EXIT_FAILURE);
    }
    /*Create the shared memory, and attach shared memory to current process*/
 
    shid = shmget((key_t)654321, (size_t)2048, 0600|IPC_CREAT);
    
    if (shid == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    
    /* attach shared memory to current process */
    sharem = shmat(shid, NULL, 0);
    if (sharem == NULL)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    
    /*Reading one line from the standard input, this program sends this information to the other program*/
    
    while(running)
    {
        /* if sem value = 0 */
        if ((value = semctl(semid, 0, GETVAL)) == 0)
        {
            printf("please input:");
            
            /* receiving alpha numeric strings as input from the user one line at a time */
            scanf("%s", (char *)sharem);
            
            sem_b.sem_op = 1;
            
            if (semop(semid, &sem_b, 1) == -1)
            {
                fprintf(stderr, "semaphore operation failed.\n");
                exit(EXIT_FAILURE);
            }
            
            if (strcmp(sharem, "end") == 0)
            {
                running = 0;
            }
        }
    }
    /*If sem signal is 0, it represents receiver can receive user input, increase the sem signal, so the processor can get the receiver’s input after receive the user.
     If the receiver get the user input, and the input string is “end”, the receiver should return and stop waiting for the input.
     */
    
    
    shmdt(sharem);

    return 0;
}
