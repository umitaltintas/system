//
// Created by umit on 19.05.2021.

#ifndef DEBUG
//#define DEBUG
#endif
#include "main.h"

#include "queue.h"
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
sem_t student_count_sem;

int main(int argc, char *argv[]) {
  int solved_hw_count = 0;
  int harcama = 0;
  struct StudentForHire student_arr[MAX_STUDENT_COUNT];
  pthread_t student_thread_arr[MAX_STUDENT_COUNT];
  queue = create_queue();
  pthread_t readerThreadID;
  if (argc != 4) {
    fprintf(stderr, "invalid arguments\n");
    finished_flag = 1;
  }
  if (!finished_flag) {
    sem_init(&reader_p.occupied, 0, 0);
    sem_init(&reader_p.empty, 0, MAX_QUEUE);
    sem_init(&reader_p.pmut, 0, 1);
    sem_init(&reader_p.cmut, 0, 1);
    pthread_create(&readerThreadID, NULL, &readerThread, argv[1]);
  }

  FILE *fp = fopen(argv[2], "r");
  if (fp == NULL) {
    fprintf(stderr, "fopen has failed");
    finished_flag = 1;
  }

  int student_count = 0;
  while (!feof(fp)) {
    if (finished_flag) {
      break;
    }
    fscanf(fp, "%19s %d %d %d\n",
           student_arr[student_count].name, // NOLINT(cert-err34-c)
           &student_arr[student_count].speed,
           &student_arr[student_count].quality,
           &student_arr[student_count].cost);
    student_arr[student_count].access_sem = (sem_t *)malloc(sizeof(sem_t));
    sem_init(student_arr[student_count].access_sem, 1, 1);
    student_arr[student_count].start_flag = (sem_t *)malloc(sizeof(sem_t));
    student_arr[student_count].busy_flag = (int *)malloc(sizeof(int));
    if (student_arr[student_count].busy_flag == NULL) {
      perror("busy_flag");
      exit(1);
    } else {
      *student_arr[student_count].busy_flag = 0;
      //      printf("flag:%d\n", *student_arr[student_count].busy_flag);
    }

    sem_init(student_arr[student_count].start_flag, 1, 0);
    student_arr[student_count].hw_count = 0;

    student_count++;
  }
  fclose(fp);
  printf("%d students-for-hire threads have been created.\n", student_count);

  print_students(student_arr, student_count);
  struct StudentForHire *speed_arr;

  struct StudentForHire *quality_arr;

  struct StudentForHire *cost_arr;

  if (!finished_flag) {
    speed_arr = sort_students_by_speed(student_arr, student_count);

    quality_arr = sort_students_by_quality(student_arr, student_count);

    cost_arr = sort_students_by_cost(student_arr, student_count);
  }

  sem_init(&student_count_sem, 1, 0);
  for (int i = 0; i < student_count; ++i) {
    if (finished_flag) {
      break;
    }
    pthread_create(&student_thread_arr[i], NULL, studentThread,
                   (void *)(&student_arr[i]));
  }

#ifdef DEBUG
  printf("name: %s\nspeed:%d\nquality:%d\ncost:%d\n\n", student_arr[0].name,
         student_arr[0].speed, student_arr[0].quality, student_arr[0].cost);
#endif
  money = atoi(argv[3]); // NOLINT(cert-err34-c)
  char c;
  int q_c = 0;
  int money_flag = 0;
  while (1) {
    if (finished_flag || money_flag) {
      break;
    }

    wait_reader();
    if (is_queue_empty()) {
      printf("No more homeworks left or coming in, closing.\n");
      break;
    }

    if ((c = (char)dequeue(queue)) == '\0') {
      fprintf(stderr, "dequeue failed\n");
      exit(EXIT_FAILURE);
    }
    q_c++;

    post_reader();
    sem_wait(&student_count_sem);
    switch (c) {
    case 'C':
      //      printf("C:%s\n", cost_arr->name);
      if (hire_student(student_count, cost_arr, &solved_hw_count, &harcama) ==
          -1)
        money_flag = 1;
      break;
    case 'S':
      //      printf("S:%s\n", speed_arr->name);

      if (hire_student(student_count, speed_arr, &solved_hw_count, &harcama) ==
          -1)
        money_flag = 1;
      break;
    case 'Q':
      //      printf("Q:%s\n", quality_arr->name);

      if (hire_student(student_count, quality_arr, &solved_hw_count,
                       &harcama) == -1)
        money_flag = 1;
      break;
    default:
      fprintf(stderr, "invalid qualifier!");
      fprintf(stderr, "%d", c);
      finished_flag = 1;
    }
  }
  for (int i = 0; i < student_count; ++i) {
    sem_wait(student_arr[i].access_sem);
    *student_arr[i].busy_flag = -1;
    sem_post(student_arr[i].access_sem);
    sem_post(student_arr[i].start_flag);
  }

  if (finished_flag != 0) {
    printf("Terminating signal received, closing.\n");
  }
  printf("q_c:%d\n", q_c);
  sem_close(&reader_p.empty);
  sem_close(&reader_p.cmut);
  sem_close(&reader_p.occupied);
  sem_close(&reader_p.pmut); // sem_closes
  for (int i = 0; i < student_count; ++i) {
    pthread_join(student_thread_arr[i], NULL);
  }
  print_homework_solving_stats(student_arr, student_count, solved_hw_count,
                               harcama);

  pthread_join(readerThreadID, NULL); // thread joins
  free(queue);
  free(speed_arr);
  free(quality_arr);
  free(cost_arr); // frees
}
void print_homework_solving_stats(const struct StudentForHire *student_arr,
                                  int student_count, int solved_hw_count,
                                  int harcama) {
  printf("Homeworks solved and money made by the students:\n");
  for (int i = 0; i < student_count; ++i) {
    printf("%s %d %d\n", student_arr[i].name, student_arr[i].hw_count,
           student_arr[i].hw_count * student_arr[i].cost);
  }
  printf("Total Cost for %d homeworks %dTL\n", solved_hw_count, harcama);
  printf("Money left at G's account: %dTL", money);
}
void post_reader() {
  sem_post(&reader_p.cmut);
  sem_post(&reader_p.empty);
}
void wait_reader() {
  sem_wait(&reader_p.occupied);
  sem_wait(&reader_p.cmut);
}
int hire_student(int student_count, struct StudentForHire *student_arr,
                 int *solved_hw_count, int *harcama) {
  //  printf("inside\n");
  for (int i = 0; i < student_count; ++i) {

    sem_wait(student_arr[i].access_sem);
    if (*student_arr[i].busy_flag == 0) {

      if (student_arr[i].cost > money) {

        printf("Money is over Closing\n");
        *student_arr[i].busy_flag = -1;
        sem_post(student_arr[i].access_sem);
        return -1;
        break;
      }
      *student_arr[i].busy_flag = 1;
      money -= student_arr[i].cost;
      *harcama += student_arr[i].cost;
      *solved_hw_count += 1;

      sem_post(student_arr[i].start_flag);
      sem_post(student_arr[i].access_sem);

      return 1;
      break;
    }
    sem_post(student_arr[i].access_sem);
  }
}

void *studentThread(void *arg) {
  struct StudentForHire *student = (struct StudentForHire *)arg;
  while (true) {
    sem_post(&student_count_sem);
    if (finished_flag)
      break;
    sem_wait(student->access_sem);
    if (student->busy_flag == -1) {
      sem_post(student->access_sem);
      break;
    }

    sem_post(student->access_sem);

    sem_wait(student->start_flag);

    sem_wait(student->access_sem);

    if (*(student->busy_flag) == -1) {
      sem_post(student->access_sem);
      break;
    }
    sem_post(student->access_sem);
    sleep(6 - student->speed);
    sem_wait(student->access_sem);
    student->hw_count++;

    if (*student->busy_flag == -1) {
      sem_post(student->access_sem);
      break;
    }
    *student->busy_flag = 0;
    sem_post(student->access_sem);
  }
  return NULL;
}
void *readerThread(void *arg) {
  FILE *fp = fopen((char *)arg, "r");
  if (fp == NULL) {
    pthread_exit(NULL);
  }

  char c;

  while (!feof(fp)) {
    if (finished_flag) {
      break;
    }
    c = (char)fgetc(fp);
    if (c != EOF) {
      safe_enqueue(c);
    } else {
      break;
    }
  }
  /*send one more time for understanding finished*/
  sem_post(&reader_p.pmut);
  sem_post(&reader_p.occupied);
  sem_post(&reader_p.pmut);
  sem_post(&reader_p.occupied);
  fclose(fp);

  pthread_exit(NULL);
}
void safe_enqueue(char c) {
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

void print_students(const struct StudentForHire *student_arr,
                    int student_count) {
  printf("%-20s %-10s %-10s %-10s\n", "Name", "Q", "S", "C");
  for (int i = 0; i < student_count; ++i) {
    if (finished_flag) {
      break;
    }

    printf("%-20s %-10d %-10d %-10d\n", student_arr[i].name,
           student_arr[i].quality, student_arr[i].speed, student_arr[i].cost);
  }
}
