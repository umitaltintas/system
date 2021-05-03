//
// Created by umit on 3.05.2021.
//

#ifndef MIDTERM_PROJECT_COMMON_H
#define MIDTERM_PROJECT_COMMON_H

#include "file_locking.h"
#include "signal_handler.h"
#include <fcntl.h> /* For O_* constants */
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wait.h>


struct FRIDGE {
    int second;
    int first;
    sem_t sem_nurse;
    sem_t sem_common;
    sem_t sem_vac;
};
#define MAX_CITIZEN_COUNT 1000
struct ROOM {
    pid_t citizens[MAX_CITIZEN_COUNT];
    int citizen_index;
    sem_t sem_vac;
};

int n_nurses;
int n_vaccinators;
int n_citizens;
int size_of_buffer;
int n_shot;
char *path_name;
int fridge_fd;
int room_fd;
int fd;
struct FRIDGE *fridge_p;
struct ROOM *room_p;
int pipes[MAX_CITIZEN_COUNT][2];


#define MAX_NURSES_COUNT 100


#endif//MIDTERM_PROJECT_COMMON_H
