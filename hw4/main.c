//
// Created by umit on 19.05.2021.

#ifndef DEBUG
//#define DEBUG
#endif
#include "main.h"

#include "queue.h"
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
sem_t student_count_sem;

void signal_handler(int sig) {
  printf("sig:%d \n", sig);
  finished_flag = 1;
}
int main(int argc, char *argv[]) {
  signal(SIGINT, signal_handler);
  int solved_hw_count = 0;
  int spent = 0;
  struct StudentForHire student_arr[MAX_STUDENT_COUNT];
  pthread_t student_thread_arr[MAX_STUDENT_COUNT];
  queue = create_queue();
  pthread_t readerThreadID;
  if (argc != 4) {
    fprintf(stderr, "invalid arguments\n");
    finished_flag = 1;
  }
  if (!finished_flag) {
    if (0 != (sem_init(&reader_p.occupied, 0, 0))) {
    };
    if (0 != (sem_init(&reader_p.empty, 0, MAX_QUEUE))) {
    };
    if (0 != (sem_init(&reader_p.pmut, 0, 1))) {
    };
    if (0 != (sem_init(&reader_p.cmut, 0, 1))) {
    };
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
    if (NULL == (student_arr[student_count].access_sem =
                     (sem_t *)malloc(sizeof(sem_t)))) {
      perror("malloc");
      finished_flag = 1;
    }
    if (0 != (sem_init(student_arr[student_count].access_sem, 1, 1))) {
      perror("sem_init");

      finished_flag = 1;
    };
    if (NULL == (student_arr[student_count].start_flag =
                     (sem_t *)malloc(sizeof(sem_t)))) {
      perror("malloc");
      finished_flag = 1;
    }
    if (NULL ==
        (student_arr[student_count].busy_flag = (int *)malloc(sizeof(int)))) {
      perror("malloc");
      finished_flag = 1;
    }
    if (student_arr[student_count].busy_flag == NULL) {
      perror("busy_flag");
      finished_flag = 1;

    } else {
      *student_arr[student_count].busy_flag = 0;
    }

    if (0 != (sem_init(student_arr[student_count].start_flag, 1, 0))) {

      perror("sem_init");
      finished_flag = 1;
    };
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

  if (0 != (sem_init(&student_count_sem, 1, 0))) {
    perror("sem_init");
    finished_flag = 1;
  };
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
    } else {
    }

    post_reader();
    if (0 != sem_wait(&student_count_sem)) {
      perror("sem_wait");
      finished_flag = 1;
    }
    switch (c) {
    case 'C':
      if (hire_student(student_count, cost_arr, &solved_hw_count, &spent) == -1)
        money_flag = 1;
      break;
    case 'S':

      if (hire_student(student_count, speed_arr, &solved_hw_count, &spent) ==
          -1)
        money_flag = 1;
      break;
    case 'Q':
      if (hire_student(student_count, quality_arr, &solved_hw_count, &spent) ==
          -1)
        money_flag = 1;
      break;
    default:
      fprintf(stderr, "invalid qualifier!");
      fprintf(stderr, "%d", c);
      finished_flag = 1;
    }
  }
  for (int i = 0; i < student_count; ++i) {
    if (0 != sem_wait(student_arr[i].access_sem)) {
      perror("sem_wait");
      finished_flag = 1;
    }
    *student_arr[i].busy_flag = -1;
    if (0 != sem_post(student_arr[i].access_sem)) {
      {
        perror("sem_post");
        finished_flag = 1;
      }
    }
    if (0 != sem_post(student_arr[i].start_flag)) {
      {
        perror("sem_post");
        finished_flag = 1;
      }
    }
  }

  if (finished_flag != 0) {
    printf("Terminating signal received, closing.\n");
  }
  sem_close(&reader_p.empty);
  sem_close(&reader_p.cmut);
  sem_close(&reader_p.occupied);
  sem_close(&reader_p.pmut); // sem_closes
  int err;
  for (int i = 0; i < student_count; ++i) {
    err = pthread_join(student_thread_arr[i], NULL);
    if (err != 0) {
      fprintf(stderr, "pthread_join\n");
      finished_flag = 1;
    }
  }
  print_homework_solving_stats(student_arr, student_count, solved_hw_count,
                               spent);

  err = pthread_join(readerThreadID, NULL);
  if (err != 0) {
    fprintf(stderr, "pthread_join\n");
    finished_flag = 1;
  }
  for (int i = 0; i < student_count; ++i) {
    free(student_arr[i].busy_flag);
    sem_close(student_arr[i].start_flag);
    free(student_arr[i].start_flag);
    sem_close(student_arr[i].access_sem);
    free(student_arr[i].access_sem);

  } // thread joins
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
  if (0 != sem_post(&reader_p.cmut)) {
    perror("sem_post");
  };
  if (0 != sem_post(&reader_p.empty)) {
    perror("sem_post");
  };
}
void wait_reader() {
  if (0 != sem_wait(&reader_p.occupied)) {
    perror("sem_wait");
    finished_flag = 1;
  }
  if (0 != sem_wait(&reader_p.cmut)) {
    perror("sem_wait");
    finished_flag = 1;
  }
}
int hire_student(int student_count, struct StudentForHire *student_arr,
                 int *solved_hw_count, int *spent) {
  for (int i = 0; i < student_count; ++i) {

    if (0 != sem_wait(student_arr[i].access_sem)) {
      {
        perror("sem_wait");
        finished_flag = 1;
      }
    }
    if (*student_arr[i].busy_flag == 0) {
      if (student_arr[i].cost > money) {

        printf("Money is over Closing\n");
        *student_arr[i].busy_flag = -1;
        if (0 != sem_post(student_arr[i].access_sem)) {
          perror("sem_post");
          finished_flag = 1;
        }
        return -1;
      }
      *student_arr[i].busy_flag = 1;
      money -= student_arr[i].cost;
      *spent += student_arr[i].cost;
      printf("%s is solving homework for %d, H has %dTL left\n",
             student_arr[i].name, student_arr[i].cost, money);
      *solved_hw_count += 1;

      if (0 != sem_post(student_arr[i].start_flag)) {
        perror("sem_post");
        finished_flag = 1;
      };
      if (0 != sem_post(student_arr[i].access_sem)) {
        perror("sem_post");

        finished_flag = 1;
      };

      return 1;
    }
    if (0 != sem_post(student_arr[i].access_sem)) {
      perror("sem_post");
      finished_flag = 1;
    };
  }
  return 1;
}

void *studentThread(void *arg) {
  struct StudentForHire *student = (struct StudentForHire *)arg;
  while (true) {
    if (0 != sem_post(&student_count_sem)) {
      perror("sem_post");
      finished_flag = 1;
    };
    if (finished_flag)
      break;
    if (0 != sem_wait(student->access_sem)) {
      perror("sem_wait");
      finished_flag = 1;
    }
    if (*student->busy_flag == -1) {
      if (0 != sem_post(student->access_sem)) {
        perror("sem_post");
        finished_flag = 1;
      };
      break;
    }

    if (0 != sem_post(student->access_sem)) {
      finished_flag = 1;
      perror("sem_post");
    }

    printf("%s is waiting for a homework\n", student->name);
    if (0 != sem_wait(student->start_flag)) {
      perror("sem_wait");
      finished_flag = 1;
    }
    if (0 != sem_wait(student->access_sem)) {
      perror("sem_wait");
      finished_flag = 1;
    }
    if (*(student->busy_flag) == -1) {
      if (0 != sem_post(student->access_sem)) {
        perror("sem_post");
        finished_flag = 1;
      };
      break;
    }
    if (0 != sem_post(student->access_sem)) {
      perror("sem_post");
      finished_flag = 1;
    };
    sleep(6 - student->speed);
    if (0 != sem_wait(student->access_sem)) {
      perror("sem_wait");
      finished_flag = 1;
    }
    student->hw_count++;

    if (*student->busy_flag == -1) {
      if (0 != sem_post(student->access_sem)) {
        finished_flag = 1;
        perror("sem_post");
      };
      break;
    }
    *student->busy_flag = 0;
    if (0 != sem_post(student->access_sem)) {
      perror("sem_post");
      finished_flag = 1;
    };
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
  if (0 != sem_post(&reader_p.pmut)) {
    finished_flag = 1;
    perror("sem_post");
  }
  if (0 != sem_post(&reader_p.occupied)) {
    finished_flag = 1;
    perror("sem_post");
  };
  if (0 != sem_post(&reader_p.pmut)) {
    finished_flag = 1;
    perror("sem_post");
  };
  if (0 != sem_post(&reader_p.occupied)) {
    finished_flag = 1;
    perror("sem_post");
  }

  fclose(fp);

  pthread_exit(NULL);
}
void safe_enqueue(char c) {
  if (sem_wait(&reader_p.empty) != 0) {
    perror("sem_wait(&reader_p.empty)");
    finished_flag = 1;
  }
  if (sem_wait(&reader_p.pmut) != 0) {
    finished_flag = 1;
    perror("sem_wait(&reader_p.pmut)");
  }
  enqueue(queue, c);
  if (sem_post(&reader_p.pmut) != 0) {
    perror("sem_post(&reader_p.pmut)");
    finished_flag = 1;
  }
  if (sem_post(&reader_p.occupied) != 0) {
    perror("sem_post(&reader_p.occupied)");
    finished_flag = 1;
  }
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
