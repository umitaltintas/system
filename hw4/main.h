//
// Created by umit on 20.05.2021.
//
#include "data.h"

#ifndef HW4_MAIN_H
#define HW4_MAIN_H

#define MAX_STUDENT_COUNT 64
struct Cheater reader_p;
void *studentThread(void *);
void *readerThread(void *);
void safe_enqueue(char c);
void print_students(const struct StudentForHire *student_arr,
                    int student_count);
int hire_student(int student_count, struct StudentForHire *student_arr,int* ,int* );
void wait_reader();
void post_reader();
void print_homework_solving_stats(const struct StudentForHire *student_arr,
                                  int student_count,int , int);
int money;
int *queue;
sem_t sem_students;
volatile int finished_flag = 0;
#endif // HW4_MAIN_H
