//
// Created by umit on 20.05.2021.
//
#include "queue.h"
int FRONT = 0;
int REAR = 0;
int TAG =
    0; // tag variable is a flag to determine if queue is currently full or not.

int *create_queue(void) { // dynamically allocate memory for queue.
  return (int *)malloc(sizeof(int) * MAX_QUEUE);
}

int is_queue_full(void) { // determine if queue is full.
  return FRONT == REAR && TAG == 1;
}

int is_queue_empty(void) { // determine if queue is empty.
  return FRONT == REAR && TAG == 0;
}

void enqueue(int queue[], int item) { // insert an item into queue.

  if (is_queue_full()) {
#ifdef DEBUG
    printf("Queue is full!!\n");
#endif

    return;
  }
  REAR = (REAR + 1) % MAX_QUEUE;
  queue[REAR] = item;
  if (FRONT == REAR)
    TAG = 1; // if FRONT == REAR after inserting item, set TAG = 1.
#ifdef DEBUG
  printf("insert %d into queue[%d]\n", item, REAR);
#endif
  return;
}

int dequeue(int queue[]) { // take an item from queue.
  if (is_queue_empty()) {
    fprintf(stderr, "Queue is empty!!\n");
    exit(1);
  }
  FRONT = (FRONT + 1) % MAX_QUEUE;
  int item = queue[FRONT];
  if (FRONT == REAR)
    TAG = 0; // if FRONT == REAR after taking item, set TAG = 0.
#ifdef DEBUG
  printf("take %d from queue[%d]\n", item, FRONT);
#endif
  return item;
}