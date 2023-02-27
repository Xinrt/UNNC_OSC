#include <stdlib.h>
#include "linkedlist.h"
#include "coursework.h"

struct element * pHead = NULL;
struct element * pTail = NULL;
FILE *fp;

// sort processes by remaining time and addLast to a linked list
void sortProcess(struct element ** pHead, struct element ** pTail);

int main() {

    // point the pointer to the corresponding file
    fp=fopen("TASK1a.txt","w+");

    // call sortProcess fucntion
    sortProcess(&pHead, &pTail);


    // declaration
    struct process * getProcess = NULL;
    struct timeval oStartTime, oEndTime;
    long int responseTime, turnaroundTime;
    double totalResponseTime=0;
    double totalTurnaroundTime=0;

    while(pHead) {

        // get the process from front of linked list
        getProcess = removeFirst(&pHead, &pTail);
        
        // run processes non-preemptively
        runNonPreemptiveJob(getProcess, &oStartTime, &oEndTime);

        fprintf(fp, "Process Id = %d, ", getProcess->iProcessId);
        fprintf(fp, "Previous Burst Time = %d, ", getProcess->iPreviousBurstTime);
        fprintf(fp, "Remaining Burst Time = %d, ", getProcess->iRemainingBurstTime);

        // get the reponse time 
        responseTime = getDifferenceInMilliSeconds(getProcess->oTimeCreated, oStartTime);
        // get the turnaround time
        turnaroundTime = getDifferenceInMilliSeconds(getProcess->oTimeCreated, oEndTime);

        fprintf(fp, "Response Time = %d, ", responseTime);
        fprintf(fp, "Turnaround Time = %d\n", turnaroundTime);

        // add all response time
        totalResponseTime += responseTime;
        // add all response time
        totalTurnaroundTime += turnaroundTime;
    }

    // print out the average response time and average turnaround time
    fprintf(fp, "Average response time = %.6f\n", totalResponseTime/NUMBER_OF_PROCESSES);
    fprintf(fp, "Average turn around time = %.6f\n", totalTurnaroundTime/NUMBER_OF_PROCESSES);

    free(getProcess);
    fclose(fp);

    return 0;
}

void sortProcess(struct element ** pHead, struct element ** pTail) {
    struct process * array[NUMBER_OF_PROCESSES];
    struct process * temp = NULL;

    // put generated processs into an array
    for(int i=0; i<NUMBER_OF_PROCESSES; i++) {
        array[i] = generateProcess();
    }

    // sort the array in insertion sort method by processes' initial burst time
    int k;
    for(int j=1; j<NUMBER_OF_PROCESSES; j++){  
        temp = array[j];
        k = j;   // range 0 to i-1 is sorted    
        while(k >= 1 && array[k-1]->iInitialBurstTime > temp->iInitialBurstTime){         
            array[k] = array[k-1];        
            k--;
        }      
        array[k] = temp;
    }    // end outer for loop

    // add whole sorted array into a linked list from rear
    for(int m=0; m<NUMBER_OF_PROCESSES; m++) {
        addLast(array[m], pHead, pTail);
    }
    
}