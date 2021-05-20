//
// Created by umit on 20.05.2021.
//

#include "sort.h"
#include "data.h"
#include "queue.h"
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int speed_comparator(void *p1, void *p2) {
  return ((*(struct StudentForHire *)p2).speed -
          (*(struct StudentForHire *)p1).speed);
}
int quality_comparator(void *p1, void *p2) {
  return ((*(struct StudentForHire *)p2).quality -
          (*(struct StudentForHire *)p1).quality);
}
int cost_comparator(void *p1, void *p2) {
  return ((*(struct StudentForHire *)p1).cost -
          (*(struct StudentForHire *)p2).cost);
}