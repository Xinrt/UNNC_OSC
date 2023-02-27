#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include "linkedlist.h"
#include "coursework.h"

int elementInBuffer = 0;
int totalProcesses = 0;
int totalCosumeProesses = 0;
int consumerFlag = 0;

// semaphore
sem_t mutex;        
sem_t full_sem, empty_sem;
struct element * pHead = NULL;
struct element * pTail = NULL;

// create arrays to store head and tail pointers of each possible priority
struct element * headPointers[MAX_PRIORITY];
struct element * tailPointers[MAX_PRIORITY];
FILE *fp;


long int responseTime, turnaroundTime;
double totalResponseTime=0;
double totalTurnaroundTime=0;

void *producer(void *a);
void *consumer(void *a);


int main() {

    // point the pointer to the corresponding file
    fp=fopen("TASK2b.txt","w+");

    int i = 0;
    pthread_t tid_producer[NUMBER_OF_PRODUCERS]; 
    pthread_t tid_consumer[NUMBER_OF_CONSUMERS]; 

    // used to transfer producer and consumer ID
    int producerId[NUMBER_OF_PRODUCERS];
    int consumerId[NUMBER_OF_CONSUMERS];
    
    // initialize
    sem_init(&mutex, 0, 1);
    sem_init(&full_sem, 0, 0);
    sem_init(&empty_sem, 0, MAX_BUFFER_SIZE);

    // create producer thread
    for (i = 0; i < NUMBER_OF_PRODUCERS; ++i) {
        producerId[i] = i;
        pthread_create(&tid_producer[i], NULL, producer, &producerId[i]);
    }
   // create consumer thread
    for (i = 0; i < NUMBER_OF_CONSUMERS; ++i) {
        consumerId[i] = i;
        pthread_create(&tid_consumer[i], NULL, consumer, &consumerId[i]);
    }
    // wait for ending of threads
    for (i = 0; i < NUMBER_OF_PRODUCERS; ++i) {
        pthread_join(tid_producer[i], NULL);
    }
    for (i = 0; i < NUMBER_OF_CONSUMERS; ++i) {
        pthread_join(tid_consumer[i], NULL);
    }

    fprintf(fp, "Average response time = %.6f\n", totalResponseTime/(double)MAX_NUMBER_OF_JOBS);
    fprintf(fp, "Average turn around time = %.6f\n", totalTurnaroundTime/(double)MAX_NUMBER_OF_JOBS);

    sem_destroy(&mutex);
    sem_destroy(&full_sem);
    sem_destroy(&empty_sem);
    exit(0);

    fclose(fp);
    return 0;
}

void *producer(void *a) {
    struct process * generatedProcess = NULL;
    int producerId = *(int *)a;

    while (1) {
        sem_wait(&empty_sem);
        sem_wait(&mutex);

        // generate at most MAX_NUMBER_OF_JOBS processes
        if(totalProcesses<MAX_NUMBER_OF_JOBS) {
            generatedProcess = generateProcess();

            // add the processes to their corresponding pirority position
            addLast(generatedProcess, &headPointers[generatedProcess->iPriority], &tailPointers[generatedProcess->iPriority]);
            totalProcesses++;

            fprintf(fp, "Producer = %d, ", producerId);          
            fprintf(fp, "Items Produced = %d, ", totalProcesses);
            fprintf(fp, "New Process Id = %d, ", generatedProcess->iProcessId);
            fprintf(fp, "Burst Time = %d\n", generatedProcess->iInitialBurstTime);
            

            if(totalProcesses>=MAX_NUMBER_OF_JOBS) {
                for(int i=0; i<NUMBER_OF_PRODUCERS; i++) {
                    sem_post(&empty_sem);
                }
            }

            sem_post(&mutex);
            sem_post(&full_sem);
        } else {
            sem_post(&empty_sem);
            sem_post(&mutex);            
            return NULL;
        }
    }

}

void *consumer(void *a) {
    struct process * getProcess = NULL;
    struct timeval oStartTime, oEndTime;
    int consumerId = *(int *)a;

    while (1) {
        sem_wait(&full_sem);
        sem_wait(&mutex);

        // if consumer finished, consumer flag will be 1 and then exit thread
        if(consumerFlag) {
            sem_post(&full_sem);
            sem_post(&mutex);
            free(getProcess);
            return NULL;
        } else {

            // get the higher priority processes first
            for(int i=0; i<MAX_PRIORITY; i++) {
                if(headPointers[i]) {
                    getProcess = removeFirst(&headPointers[i], &tailPointers[i]);
                    break;
                }
            }
            sem_post(&mutex);
    
            // run processes preemptively
            runPreemptiveJob(getProcess, &oStartTime, &oEndTime);
            totalCosumeProesses++;
            
            // while the remaining time of process is not equal to 0
            while(getProcess->iRemainingBurstTime != 0) {
                // the first running, print out the response time
                if(getProcess->iPreviousBurstTime == getProcess->iInitialBurstTime) {
                    // get each response time
                    responseTime = getDifferenceInMilliSeconds(getProcess->oTimeCreated, oStartTime);
                    fprintf(fp, "Consumer = %d, ", consumerId); 
                    fprintf(fp, "Process Id = %d, ", getProcess->iProcessId);
                    fprintf(fp, "Priority = %d, ", getProcess->iPriority);
                    fprintf(fp, "Previous Burst Time = %d, ", getProcess->iPreviousBurstTime);
                    fprintf(fp, "Remaining Burst Time = %d, ", getProcess->iRemainingBurstTime);
                    fprintf(fp, "Response Time = %d\n", responseTime);
                }

                // during running
                if(getProcess->iRemainingBurstTime != 0 && getProcess->iPreviousBurstTime != getProcess->iInitialBurstTime) {
                    fprintf(fp, "Consumer = %d, ", consumerId); 
                    fprintf(fp, "Process Id = %d, ", getProcess->iProcessId);
                    fprintf(fp, "Priority = %d, ", getProcess->iPriority);
                    fprintf(fp, "Previous Burst Time = %d, ", getProcess->iPreviousBurstTime);
                    fprintf(fp, "New Burst Time = %d\n", getProcess->iRemainingBurstTime);
                }  

                // add pieces of processes back and run again
                addLast(getProcess, &headPointers[getProcess->iPriority], &tailPointers[getProcess->iPriority]);
                sem_post(&mutex);
                 for(int i=0; i<MAX_PRIORITY; i++) {
                    if(headPointers[i]) {
                        getProcess = removeFirst(&headPointers[i], &tailPointers[i]);
                        break;
                    }
                }
                sem_wait(&mutex);
                runPreemptiveJob(getProcess, &oStartTime, &oEndTime);
            }

            // get each turnaround time
            turnaroundTime = getDifferenceInMilliSeconds(getProcess->oTimeCreated, oEndTime);

            fprintf(fp, "Consumer = %d, ", consumerId); 
            fprintf(fp, "Process Id = %d, ", getProcess->iProcessId);
            fprintf(fp, "Priority = %d, ", getProcess->iPriority);
            fprintf(fp, "Previous Burst Time = %d, ", getProcess->iPreviousBurstTime);
            fprintf(fp, "New Burst Time = %d, ", getProcess->iRemainingBurstTime);
            fprintf(fp, "Turnaround Time = %d\n", turnaroundTime);

            // add all response time
            totalResponseTime += responseTime;
            // add all turnaround time
            totalTurnaroundTime += turnaroundTime;  
        }
        sem_wait(&mutex);
            if((totalCosumeProesses==totalProcesses) && (totalProcesses == MAX_NUMBER_OF_JOBS)) {
                consumerFlag = 1;
                for(int i=0; i<NUMBER_OF_CONSUMERS; i++) {
                    sem_post(&full_sem);
                }
            }

            sem_post(&empty_sem);
            sem_post(&mutex);
    }
}