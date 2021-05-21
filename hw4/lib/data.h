//
// Created by umit on 20.05.2021.
//

#ifndef HW4_DATA_H
#define HW4_DATA_H
#include <semaphore.h>
struct StudentForHire {
  char name[20];
  int cost;
  int quality;
  int speed;
  int hw_count;
  sem_t *access_sem;
  sem_t *start_flag;
  int *busy_flag;
};
struct Cheater {
  sem_t occupied;
  sem_t empty;
  sem_t pmut;
  sem_t cmut;
};
struct StudentForHire *sort_students_by_speed(const struct StudentForHire arr[],
                                              int size);
struct StudentForHire *
sort_students_by_quality(const struct StudentForHire arr[], int size);
struct StudentForHire *sort_students_by_cost(const struct StudentForHire arr[],
                                             int size);
#endif // HW4_DATA_H
