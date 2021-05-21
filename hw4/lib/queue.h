#ifndef QUEUE_H
#define QUEUE_H
#include <stdio.h>
#include <stdlib.h>
#define MAX_QUEUE 100

int *create_queue(void);
int is_queue_full(void);
int is_queue_empty(void);
void enqueue(int[], int);
int dequeue(int[]);



#endif
