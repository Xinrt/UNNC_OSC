#include <stdlib.h>
#include "linkedlist.h"
#include "coursework.h"

struct element * pHead = NULL;
struct element * pTail = NULL;
struct element * pHeadpTails[NUMBER_OF_PROCESSES][2];
int pointersSize;
FILE *fp;

// sort processes by processes' priorities and addLast to a linked list
void sortProcess(struct element * pHead, struct element * pTail);

int main() {

    // point the pointer to the corresponding file
    fp=fopen("TASK1b.txt","w+");

    // call sortProcess fucntion
    sortProcess(pHead, pTail);

    // declaration
    struct process * getProcess = NULL;
    struct process * getProcess2 = NULL;
    struct timeval oStartTime, oEndTime;
    long int responseTime, turnaroundTime;
    double totalResponseTime=0;
    double totalTurnaroundTime=0;

    
    fprintf(fp, "\n");
    for(int i=0; i<pointersSize; i++) {
        while(pHeadpTails[i][0]) {   
            // remove from front of priority queues
            getProcess = removeFirst(&pHeadpTails[i][0], &pHeadpTails[i][1]);  

            // run processes preemtively
            runPreemptiveJob(getProcess, &oStartTime, &oEndTime);

            // while the remaining time of process is not equal to 0
            while(getProcess->iRemainingBurstTime != 0) {
                // the first running, print out the response time
                if(getProcess->iPreviousBurstTime == getProcess->iInitialBurstTime) {
                    // get each response time
                    responseTime = getDifferenceInMilliSeconds(getProcess->oTimeCreated, oStartTime);
                    fprintf(fp, "Process Id = %d, ", getProcess->iProcessId);
                    fprintf(fp, "Priority = %d, ", getProcess->iPriority);
                    fprintf(fp, "Previous Burst Time = %d, ", getProcess->iPreviousBurstTime);
                    fprintf(fp, "Remaining Burst Time = %d, ", getProcess->iRemainingBurstTime);
                    fprintf(fp, "Response Time = %d\n", responseTime);
                }

                // during running
                if(getProcess->iRemainingBurstTime != 0 && getProcess->iPreviousBurstTime != getProcess->iInitialBurstTime) {
                    fprintf(fp, "Process Id = %d, ", getProcess->iProcessId);
                    fprintf(fp, "Priority = %d, ", getProcess->iPriority);
                    fprintf(fp, "Previous Burst Time = %d, ", getProcess->iPreviousBurstTime);
                    fprintf(fp, "Remaining Burst Time = %d\n", getProcess->iRemainingBurstTime);
                }
                // add pieces of processes back and run again
                addLast(getProcess, &pHeadpTails[i][0], &pHeadpTails[i][1]);
                getProcess = removeFirst(&pHeadpTails[i][0], &pHeadpTails[i][1]);
                runPreemptiveJob(getProcess, &oStartTime, &oEndTime);
            }

            // get each turnaround time
            turnaroundTime = getDifferenceInMilliSeconds(getProcess->oTimeCreated, oEndTime);

            fprintf(fp, "Process Id = %d, ", getProcess->iProcessId);
            fprintf(fp, "Priority = %d, ", getProcess->iPriority);
            fprintf(fp, "Previous Burst Time = %d, ", getProcess->iPreviousBurstTime);
            fprintf(fp, "Remaining Burst Time = %d, ", getProcess->iRemainingBurstTime);
            fprintf(fp, "Turnaround Time = %d\n", turnaroundTime);

            // add all response time
            totalResponseTime += responseTime;
            // add all turnaround time
            totalTurnaroundTime += turnaroundTime;
            
        }
    }

    // print the average response time and turnaround time
    fprintf(fp, "Average response time = %.6f\n", totalResponseTime/NUMBER_OF_PROCESSES);
    fprintf(fp, "Average turn around time = %.6f\n", totalTurnaroundTime/NUMBER_OF_PROCESSES);

    free(getProcess);
    fclose(fp);
    return 0;
}

void sortProcess(struct element * pHead, struct element * pTail) {
    struct process * array[NUMBER_OF_PROCESSES];
    struct process * temp = NULL;
    int priorityNumber=1;

    // put generated processs into an array
    for(int i=0; i<NUMBER_OF_PROCESSES; i++) {
        array[i] = generateProcess();
    }

    // sort the array in insertion sort method by processes' priorities
    int k;
    for(int j=1; j<NUMBER_OF_PROCESSES; j++){  
        temp = array[j];
        k = j;   // range 0 to i-1 is sorted    
        while(k >= 1 && array[k-1]->iPriority > temp->iPriority){         
            array[k] = array[k-1];        
            k--;
        }      
        array[k] = temp;
    }    // end outer for loop


    //get the number of processes' priorities
    for(int i=0; i<NUMBER_OF_PROCESSES-1; i++) {
        if(array[i+1]->iPriority != array[i]->iPriority) {
            priorityNumber++;
        }
    }

    // assign the number of priorities to a global variable
    pointersSize = priorityNumber;

    // put the head and tail pointers to a two-dimentioal array
    for(int j=0; j<priorityNumber; j++) {
        pHeadpTails[j][0] = pHead;
        pHeadpTails[j][1] = pTail;
    }

    for(int j=0; j<priorityNumber; j++) {
        // printf("pHeadpTail: %d\n%d\n", pHeadpTails[j][0], pHeadpTails[j][1]);
    }

    // print put the prioirty list and add them to the several linked list from rear
    fprintf(fp, "PROCESS LIST: \n");
    int pointerIndex = 0;
    for(int i=0; i<NUMBER_OF_PROCESSES; i++) {
        addLast(array[i], &pHeadpTails[pointerIndex][0], &pHeadpTails[pointerIndex][1]);

        if(i==0){
            fprintf(fp, "Priority %d\n", array[i]->iPriority);
        } else if(i>0) {
            if(array[i]->iPriority != array[i-1]->iPriority) {
                fprintf(fp, "Priority %d\n", array[i]->iPriority);
            }
        }

        // if priority did not change, put processes into the same queue
        pointerIndex++;
        if(array[i+1]->iPriority == array[i]->iPriority) {
            pointerIndex--;
        }


        fprintf(fp, "\t Process Id = %d, ", array[i]->iProcessId);
        fprintf(fp, "Priority = %d, ", array[i]->iPriority);
        fprintf(fp, "Initial Burst Time = %d, ", array[i]->iInitialBurstTime);
        fprintf(fp, "Remaining Burst Time = %d\n", array[i]->iRemainingBurstTime);
    }
    fprintf(fp, "END\n");
}