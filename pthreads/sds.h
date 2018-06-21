/******************************************************
 * FILE: sds.h
 * DATE: 5/5/2018
 * UNIT: OS, 2018 S1
 * AUTHOR: Kai Li Shi 19157364
 * PURPOSE: header file for sds.c
 *          contains method declarations and global variables. 
 ******************************************************/          

#ifndef ASSIGNMENT_SDS_H
#define ASSIGNMENT_SDS_H

#define FALSE 0
#define TRUE 1
#define d 100
#define b 20 

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int r, w, t1, t2;
int numDWritten = 0;
int emptycount= b;
int fullcount = 0;
int readcount = 0;
pthread_mutex_t readmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t count = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t full = PTHREAD_COND_INITIALIZER;
int data_buffer[b];
int countarray[b];
int sharedarray[d];

void * writer ();
void * reader();
void writeMessage(char *message);

#endif //ASSIGNMENT_SDS_H
