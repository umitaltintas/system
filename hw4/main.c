//
// Created by umit on 19.05.2021.
//
#ifndef DEBUG
//#define DEBUG
#endif
#include "queue.h"
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STUDENT_COUNT 64

struct StudentForHire {
  char name[20];
  int cost;
  int quality;
  int speed;
  sem_t busy_sem;
  int busy_flag;
};

struct Cheater {
  sem_t occupied;
  sem_t empty;
  sem_t pmut;
  sem_t cmut;
};
struct Cheater reader_p;

void *studentThread(void *);
void *readerThread(void *);
struct StudentForHire *sort_students_by_speed(const struct StudentForHire arr[],
                                              int size);
struct StudentForHire *
sort_students_by_quality(const struct StudentForHire arr[], int size);
struct StudentForHire *sort_students_by_cost(const struct StudentForHire arr[],
                                             int size);
int speed_comparator(const void *p1, const void *p2) {
  return ((*(struct StudentForHire *)p2).speed -
          (*(struct StudentForHire *)p1).speed);
}
int quality_comparator(const void *p1, const void *p2) {
  return ((*(struct StudentForHire *)p2).quality -
          (*(struct StudentForHire *)p1).quality);
}
int cost_comparator(const void *p1, const void *p2) {
  return ((*(struct StudentForHire *)p1).cost -
          (*(struct StudentForHire *)p2).cost);
}
int money;
int *queue;
sem_t sem_students;
int main(int argc, char *argv[]) {

  struct StudentForHire student_arr[MAX_STUDENT_COUNT];
  queue = create_queue();
  pthread_t readerThreadID;
  if (argc != 4) {
    fprintf(stderr, "invalid arguments\n");
    exit(1);
  }
  sem_init(&reader_p.occupied, 0, 0);
  sem_init(&reader_p.empty, 0, MAX_QUEUE);
  sem_init(&reader_p.pmut, 0, 1);
  sem_init(&reader_p.cmut, 0, 1);
  pthread_create(&readerThreadID, NULL, &readerThread, argv[1]);

  FILE *fp = fopen(argv[2], "r");
  if (fp == NULL) {
    fprintf(stderr, "fopen has failed");
    exit(EXIT_FAILURE);
  }
  char s[20];
  int student_index = 0;

  while (!feof(fp)) {

    fscanf(fp, "%19s %d %d %d\n", student_arr[student_index].name,
           &student_arr[student_index].speed,
           &student_arr[student_index].quality,
           &student_arr[student_index].cost);
    sem_init(&student_arr[student_index].busy_sem, 1, 1);
    student_arr[student_index].busy_flag = 0;
#ifdef DEBUG
    printf("name: %s\nspeed:%d\nquality:%d\ncost:%d\n\n",
           student_arr[student_index].name, student_arr[student_index].speed,
           student_arr[student_index].quality, student_arr[student_index].cost);
#endif
    student_index++;
  }
#ifdef DEBUG
  printf("name: %s\nspeed:%d\nquality:%d\ncost:%d\n\n", student_arr[0].name,
         student_arr[0].speed, student_arr[0].quality, student_arr[0].cost);
#endif
  const struct StudentForHire *speed_arr =
      sort_students_by_speed(student_arr, student_index);
  printf("%d\n", speed_arr[0].speed);

  const struct StudentForHire *quality_arr =
      sort_students_by_quality(student_arr, student_index);
  printf("%d\n", quality_arr[0].quality);

  const struct StudentForHire *cost_arr =
      sort_students_by_cost(student_arr, student_index);
  printf("%d\n", cost_arr[0].cost);
#ifdef DEBUG
  printf("name: %s\nspeed:%d\nquality:%d\ncost:%d\n\n", student_arr[0].name,
         student_arr[0].speed, student_arr[0].quality, student_arr[0].cost);
#endif
  char c;
  while (1) {

    sem_wait(&reader_p.occupied);
    sem_wait(&reader_p.cmut);
    if (is_queue_empty()) {
      break;
    }

    if ((c = (char)dequeue(queue)) == '\0') {
      fprintf(stderr, "dequeue failed\n");
      exit(EXIT_FAILURE);
    }
    sem_post(&reader_p.cmut);
    sem_post(&reader_p.empty);
    printf("%c ", c);
  }

  sem_close(&reader_p.empty);
  sem_close(&reader_p.cmut);
  sem_close(&reader_p.occupied);
  sem_close(&reader_p.pmut);
}

void *studentThread(void *arg) { return NULL; }
void *readerThread(void *arg) {
  FILE *fp = fopen((char *)arg, "r");
  if (fp == NULL) {
    pthread_exit(NULL);
  }

  char c;

  while ((c = (char)fgetc(fp)) != EOF) {
    if (sem_wait(&reader_p.empty) != 0)
      perror("sem_wait(&reader_p.empty)");
    if (sem_wait(&reader_p.pmut) != 0)
      perror("sem_wait(&reader_p.pmut)");
    enqueue(queue, c);
    if (sem_post(&reader_p.pmut) != 0)
      perror("sem_post(&reader_p.pmut)");

    if (sem_post(&reader_p.occupied) != 0)
      perror("sem_post(&reader_p.occupied)");
  }

  sem_post(&reader_p.pmut);
  sem_post(&reader_p.occupied);
  sem_post(&reader_p.pmut);
  sem_post(&reader_p.occupied);
  fclose(fp);

  pthread_exit(NULL);
}

/*don't forget to free return value */
struct StudentForHire *sort_students_by_speed(const struct StudentForHire arr[],
                                              int size) {
  struct StudentForHire *new_arr =
      (struct StudentForHire *)malloc(sizeof(arr[0]) * size);
  memcpy(new_arr, arr, size * sizeof(arr[0]));

  qsort(new_arr, size, sizeof(arr[0]), speed_comparator);
#ifdef DEBUG
  printf("name: %s\nspeed:%d\nquality:%d\ncost:%d\n\n", new_arr[0].name,
         new_arr[0].speed, new_arr[0].quality, new_arr[0].cost);
#endif
  return new_arr;
}

/*don't forget to free return value */

struct StudentForHire *
sort_students_by_quality(const struct StudentForHire arr[], int size) {
  struct StudentForHire *new_arr =
      (struct StudentForHire *)malloc(sizeof(arr[0]) * size);
  memcpy(new_arr, arr, size * sizeof(arr[0]));

  qsort(new_arr, size, sizeof(arr[0]), quality_comparator);
#ifdef DEBUG
  printf("name: %s\nspeed:%d\nquality:%d\ncost:%d\n\n", new_arr[0].name,
         new_arr[0].speed, new_arr[0].quality, new_arr[0].cost);
#endif
  return new_arr;
}
/*don't forget to free return value */

struct StudentForHire *sort_students_by_cost(const struct StudentForHire arr[],
                                             int size) {
  struct StudentForHire *new_arr =
      (struct StudentForHire *)malloc(sizeof(arr[0]) * size);
  memcpy(new_arr, arr, size * sizeof(arr[0]));

  qsort(new_arr, size, sizeof(arr[0]), cost_comparator);
#ifdef DEBUG
  printf("name: %s\nspeed:%d\nquality:%d\ncost:%d\n\n", new_arr[0].name,
         new_arr[0].speed, new_arr[0].quality, new_arr[0].cost);
#endif
  return new_arr;
}