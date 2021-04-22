//
// Created by umit on 22.04.2021.
//

#ifndef HW3_GLOBALS_H
#define HW3_GLOBALS_H
#include <fcntl.h>
#include <getopt.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#define PID_INDEX 0
#define POTATO_INDEX 1
#define MAX_FIFO_C 250
typedef struct SM {
  short fifo_index;
  pid_t peer[MAX_FIFO_C][2];
} sm;

sem_t *sem;
char fifo_file_names[50][250];
int read_index;
int fifo_count=0;
caddr_t memptr;
short is_creator =0;
sm *shrm;

#endif // HW3_GLOBALS_H
