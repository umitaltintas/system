//
// Created by umit on 22.04.2021.
//

#ifndef HW3_GLOBALS_H
#define HW3_GLOBALS_H
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define PID_INDEX 0
#define POTATO_INDEX 1
#define MAX_FIFO_C 250
#define FINISH_SIGN (-3)

typedef struct SM {
  int potato_c;
  sem_t sem;
  short fifo_index;
  pid_t peer[MAX_FIFO_C][2];
} sm;
sem_t *sem;
char fifo_file_names[MAX_FIFO_C][250];
int fifo_fd[MAX_FIFO_C];
int read_index;
int fifo_count = 0;
caddr_t memory_pointer;
short is_creator = 1;
sm *shared_memory_pointer;
char *shared_memory_name;
char *fifo_names_file;
int pot_sw_count;
char *sem_name;

#endif // HW3_GLOBALS_H
