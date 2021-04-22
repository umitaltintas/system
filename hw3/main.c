//
// Created by umit on 17.04.2021.
//
#include "globals.h"
#include "usage.h"
#include <errno.h>
#include <time.h>

void report_and_exit(const char *msg) {
  perror(msg);
  exit(-1);
}
int fill_fifo_files(const char *);

int select_random_index();
int main(int argc, char **argv) {
  srand(time(0));

  int c;
  char *shared_mem_name;
  char *fifo_names_file;
  int pot_sw_count;
  char *sem_name;

  if (argc < 9) {
    printUsage();
    exit(EXIT_FAILURE);
  }

  while ((c = getopt(argc, argv, "b:s:f:m:")) != -1) {
    switch (c) {
    case 'f':
      printf("f: %s\n", optarg);
      fifo_names_file = optarg;
      break;
    case 'b':
      printf("b: %s\n", optarg);
      pot_sw_count = atoi(optarg);
      break;
    case 's':
      printf("s: %s\n", optarg);
      shared_mem_name = optarg;
      break;
    case 'm':
      printf("m: %s\n", optarg);
      sem_name = optarg;
      break;
    default:
      printUsage();
      exit(EXIT_FAILURE);
    }
  }
  printf("shared_mem_name: %s , fifo_names_file: %s, pot_sw_count: %d",
         shared_mem_name, fifo_names_file, pot_sw_count);

  errno = 0;
  int fd = shm_open(shared_mem_name,  /* name from smem.h */
                    O_RDWR | O_CREAT, /* read/write, create if needed */
                    0x0666);
  if (errno != 0) {
    if (errno == EEXIST) {
      is_creator = 1;
    } else {
      report_and_exit("can't open shared memory");
    }
  }

  if ((ftruncate(fd, sizeof(struct SM)) == -1)) {
    report_and_exit("ftruncate failed");
  }

  memptr = mmap(NULL,              /* let system pick where to put segment */
                sizeof(struct SM), /* how many bytes */
                PROT_READ | PROT_WRITE, /* access protections */
                MAP_SHARED,             /* mapping visible to other processes */
                fd, /* file descriptor */ 0);

  if ((caddr_t)-1 == memptr) {
    report_and_exit("Can't get segment...");
  }
  /* semaphore code to lock the shared mem */
  sem = sem_open(sem_name, /* name */
                 O_CREAT,  /* create the semaphore */
                 0x0666,   /* protection perms */
                 0);       /* initial value */
  if (sem == (void *)-1) {
    report_and_exit("sem_open");
  }
  fill_fifo_files(fifo_names_file);
  shrm = (sm *)memptr;

  if (is_creator) {
    for (int i = 0; i < fifo_count; i++) {
      if (0 != mkfifo(fifo_file_names[i], 0666)) {
        report_and_exit("mkfifo");
      }
    }
    sem_init(&(shrm->sem), 1, 1);
    shrm->fifo_index = 0;
    shrm->potato_c = 0;
    sem_post(sem);
  }
  sem_wait(sem);
  read_index = shrm->fifo_index;
  if ((shrm->peer[read_index][POTATO_INDEX] = pot_sw_count)) {
    shrm->potato_c++;
  }
  shrm->peer[read_index][PID_INDEX] = getpid();
  shrm->fifo_index++;
  sem_post(sem);

  for (int i = 0; i < fifo_count; i++) {
    if (read_index != i) {
      fifo_fd[i] = open(fifo_file_names[i], O_WRONLY);
    } else {
      fifo_fd[i] = open(fifo_file_names[i], O_RDONLY);
    }
  }
  int potato_id = read_index;
  sem_wait(&shrm->sem);
  if (!shrm->peer[read_index][POTATO_INDEX]) {
    write(fifo_fd[select_random_index()], potato_id, sizeof(potato_id));
  }

  return 0;
}

int fill_fifo_files(const char *file_name) {

  FILE *fp;
  if (NULL == (fp = fopen(file_name, "r"))) {
    report_and_exit("fopen failed");
  }
  while (fgets(fifo_file_names[fifo_count], 250, fp) != NULL) {
    fifo_count++;
  }
  if (feof(fp)) {
    for (int i = 0; i < fifo_count; ++i) {
      fifo_file_names[i][strcspn(fifo_file_names[i], "\n")] = '\0';
    }
    return 1;
  }
  if (ferror(fp)) {
    report_and_exit(fgets);
  }
  return -1;
}

int select_random_index() {
  int randn = read_index;
  while (randn == read_index) {
    randn = (rand() % fifo_count);
  }
  return randn;
}
