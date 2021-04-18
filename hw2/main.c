#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

//#include <string.h>
double Pn(int n, double x[n + 1], double X);

int child_thread(FILE *fp);

double Li(int i, int n, const double x[n + 1], double X);

int block_file(int fd);

int unblock_file(int fd);

volatile sig_atomic_t suspended_count = 0;

static volatile sig_atomic_t sigflag; /* set nonzero by sig handler */
static sigset_t newmask, oldmask, zeromask;

static void handle_sigusr1(int signo) { sigflag = 1; }
void handle_sigurs2(int sig_num) {
  sigflag = 1;
  suspended_count++;
}

void TELL_WAIT(void) {
  if (signal(SIGUSR1, handle_sigusr1) == SIG_ERR)
    perror("signal(SIGUSR1) error");
  if (signal(SIGUSR2, handle_sigurs2) == SIG_ERR)
    perror("signal(SIGUSR2) error");
  sigemptyset(&zeromask);
  sigemptyset(&newmask);
  sigaddset(&newmask, SIGUSR1);
  sigaddset(
      &newmask,
      SIGUSR2); /* Block SIGUSR1 and SIGUSR2, and save current signal mask */
  if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0)
    perror("SIG_BLOCK error");
}

void TELL_PARENT(pid_t pid) { kill(pid, SIGUSR2); /* tell parent we’re done */ }

void WAIT_PARENT(void) {
  while (sigflag == 0)
    sigsuspend(&zeromask); /* and wait for parent */

  sigflag = 0; /* Reset signal mask to original value */
  if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
    perror("SIG_SETMASK error");
}

void TELL_CHILD(pid_t pid) { kill(pid, SIGUSR1); /* tell child we’re done */ }

void WAIT_CHILD(void) {
  while (sigflag == 0)
    sigsuspend(&zeromask); /* and wait for child */
  sigflag = 0;             /* Reset signal mask to original value */
  if (sigprocmask(SIG_SETMASK, &zeromask, NULL) < 0)
    perror("SIG_SETMASK error");
}

void handler(int sig_num) { printf("%d", sig_num); }

int main(void) {
  TELL_WAIT();
  FILE *fp = fopen("input.txt", "r+");

  if (fp == NULL) {
    perror("open");
    return 1;
  }

  /*
   * create 8 child process and call child_thread from them.
   *
   * */
  int c_pid[8];
  for (int i = 0; i < 8; ++i) {
    if ((c_pid[i] = fork()) != -1) {
      if (c_pid[i] == 0) {
        child_thread(fp);
        return 0;
      }
      printf("offset in parent :%ld\n", ftell(fp));
    } else {
      perror("fork");
      return 1;
    }
  }

  /*
   * wait until all child is finished.
   *
   * */
  int stat;
  int t_id;
  int i = 0;
  while (suspended_count < 8) {
    TELL_CHILD(c_pid[i]);
    i++;
    WAIT_CHILD();
  }

  for (int i = 0; i < 8; ++i) {
    TELL_CHILD(c_pid[i]);
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

  TELL_WAIT();

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
  WAIT_PARENT();
  TELL_PARENT(getppid());
  WAIT_PARENT();

  double fun[16];
  char *end;
  for (int i = 0; i < 16; i++) {
    fun[i] = strtod(buffer, &end);
    buffer = end + 1;
  }
  printf("\n calculate5 : %.2lf ", Pn(5, fun, fun[14]));
  printf("calculate6: %.2lf \n", Pn(6, fun, fun[14]));
  printf("offset6 :%ld\n", ftell(fp));

  free(tmp);
  return 0;
}

int unblock_file(int fd) { /* unlock file*/
  struct flock fl = {
      .l_whence = SEEK_SET, .l_pid = 0, .l_start = 0, .l_len = 0};
  fl.l_type = F_UNLCK;
  errno = 0;
  if (fcntl(fd, F_SETLKW, &fl) == -1) {
    perror("unlock|fcntl");
    return -1;
  } else

    return 0;
}

int block_file(int fd) {
  struct flock fl = {.l_whence = SEEK_SET, .l_start = 0, .l_len = 0};
  /*  lock file for writing*/
  fl.l_type = F_WRLCK;
  fl.l_pid = getpid();
  errno = 0;
  if (fcntl(fd, F_SETLKW, &fl) == -1) {
    perror("block_file|fcntl");
    return -1;
  }
  return 0;
}

double Li(int i, int n, const double x[], double X) {
  int j;
  double prod = 1;
  for (j = 0; j <= n; j++) {
    if (j != i)
      prod = prod * (X - x[j * 2]) / (x[i * 2] - x[j * 2]);
  }
  return prod;
}

/*Function to evaluate Pn(x) where Pn is the Lagrange interpolating polynomial
 * of degree n*/

double Pn(int n, double x[], double X) {
  double sum = 0;
  int i;
  for (i = 0; i <= n; i++) {
    sum = sum + Li(i, n, x, X) * x[2 * i + 1];
  }
  return sum;
}