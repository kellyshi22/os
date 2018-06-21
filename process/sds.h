/*********************************************************
 * FILE: sds.h
 * DATE: 5/5/2018
 * UNIT: OS, 2018 S1
 * AUTHOR: Kai Li Shi 19157364
 * PURPOSE: header file for sds.c
 *          contains method declarations and global variables
 * *******************************************************/

#ifndef PROCESSES_SDS_H
#define PROCESSES_SDS_H

#define FALSE 0
#define TRUE 1
#define d 100
#define b 20

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>

typedef struct {
    sem_t read;
    sem_t count;
    sem_t full;
    sem_t empty;
} Locks;

int r, w, t1, t2, error;

void writeMessage(char * message);
void writer(Locks * locks, int * data_buffer, int * f, int * numDWritten); 
void reader(Locks * locks, int * data_buffer, int * readcount, int * countarray, int * numDWritten, int * fastestreader);

#endif //PROCESSES_SDS_H
