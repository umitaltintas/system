//
// Created by umit on 20.05.2021.
//

#include "data.h"
#include "sort.h"
#include <stdio.h>
#include <stdlib.h>


/*don't forget to free return value */
struct StudentForHire *sort_students_by_speed(const struct StudentForHire arr[],
                                              int size) {
  struct StudentForHire *new_arr =
      (struct StudentForHire *)malloc(sizeof(arr[0]) * size);
  for (int i = 0; i < size; i++) {
    new_arr[i] = arr[i];
  }
  qsort(new_arr, size, sizeof(arr[0]), speed_comparator);
#ifdef DEBUG
  printf("name: %s\nspeed:%d\nquality:%d\ncost:%d\n\n", new_arr[0].name,
         new_arr[0].speed, new_arr[0].quality, new_arr[0].cost);
#endif
  return new_arr;
}

struct StudentForHire *
sort_students_by_quality(const struct StudentForHire arr[], int size) {
  struct StudentForHire *new_arr =
      (struct StudentForHire *)malloc(sizeof(arr[0]) * size);
  for (int i = 0; i < size; i++) {
    new_arr[i] = arr[i];
  }


  qsort(new_arr, size, sizeof(arr[0]), quality_comparator);
#ifdef DEBUG
  printf("name: %s\nspeed:%d\nquality:%d\ncost:%d\n\n", new_arr[0].name,
         new_arr[0].speed, new_arr[0].quality, new_arr[0].cost);
#endif
  return new_arr;
}

struct StudentForHire *sort_students_by_cost(const struct StudentForHire arr[],
                                             int size) {
  struct StudentForHire *new_arr =
      (struct StudentForHire *)malloc(sizeof(arr[0]) * size);
  for (int i = 0; i < size; i++) {
    new_arr[i] = arr[i];
  }


  qsort(new_arr, size, sizeof(arr[0]), cost_comparator);
#ifdef DEBUG
  printf("name: %s\nspeed:%d\nquality:%d\ncost:%d\n\n", new_arr[0].name,
         new_arr[0].speed, new_arr[0].quality, new_arr[0].cost);
#endif
  return new_arr;
}
