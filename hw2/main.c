#include "file_locking.h"
#include "lagrange.h"
#include "signal_handler.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int child_thread(FILE *fp);



int main(void) {
  printf("test");

  block_signals();
  FILE *fp = fopen("input.txt", "r+");
  if (fp == NULL) {
    perror("open");
    return 1;
  }

  /*create 8 child process and call child_thread from them.*/
  int c_pid[8];
  for (int i = 0; i < 8; ++i) {
    if ((c_pid[i] = fork()) != -1) {
      if (c_pid[i] == 0) {
        child_thread(fp);
        return 0;
      }
    } else {
      perror("fork");
      return 1;
    }
  }

  /*wait until all child is finished.*/
  int stat;
  int t_id;
  int i = 0;
  while (suspended_count < 8) {
    sig_child(c_pid[i]);
    i++;
    suspend_parent();
  }

  for (int i = 0; i < 8; ++i) {
    sig_child(c_pid[i]);
  }

  while (-1 != (t_id = wait(&stat)) || errno != ECHILD) {
    printf("%d is finished with status: %d\n", t_id, stat);
  }
  fclose(fp);

  return 0;
}

/*
 *child process's main function.
 */

int child_thread(FILE *fp) {

  block_signals();

  // block file
  block_file(fileno(fp));

  /*read from file*/

  char *buffer = (char *)malloc(sizeof(char) * 250);
  if (NULL == fgets(buffer, 250, fp)) {
    if (feof(fp))
      fprintf(stderr, "End-of-file occurred\n");
    if (ferror(fp))
      fprintf(stderr, "Input error occurred\n");
  }
  fflush(fp);
  char *tmp = buffer;
  unblock_file(fileno(fp));
  secure_wait_for_child();

  double fun[16];
  char *end;
  for (int i = 0; i < 16; i++) {
    fun[i] = strtod(buffer, &end);
    buffer = end + 1;
  }
  printf("\n calculate5 : %.2lf ", Pn(5, fun, fun[14]));
  printf("calculate6: %.2lf \n", Pn(6, fun, fun[14]));

  free(tmp);
  return 0;
}
