//
// Created by umit on 17.04.2021.
//
#include "globals.h"
#include "usage.h"

void report_and_exit(const char *msg) {
  perror(msg);
  exit(-1);
}
int fill_fifo_files(const char *);

int select_random_index();
void finish(int sig);
void send_finish_sign();
int main(int argc, char **argv) {
  srand(time(0));
  int c;

  if (argc < 9) {
    printUsage();
    exit(EXIT_FAILURE);
  }

  while ((c = getopt(argc, argv, "b:s:f:m:")) != -1) {
    switch (c) {
    case 'f':

      fifo_names_file = optarg;
      break;
    case 'b':

      pot_sw_count = atoi(optarg);
      break;
    case 's':

      shared_memory_name = optarg;
      break;
    case 'm':
      sem_name = optarg;
      break;
    default:
      printUsage();
      exit(EXIT_FAILURE);
    }
  }
  signal(SIGINT, finish);
  errno = 0;
  int fd = shm_open(shared_memory_name, O_RDWR | O_CREAT | O_EXCL,
                    S_IRUSR | S_IWUSR);

  if (fd == -1) {
    if (errno == EEXIST) {
      is_creator = 0;
      if (1 > (fd = shm_open(shared_memory_name, O_RDWR, S_IRUSR | S_IWUSR))) {
        report_and_exit("can't open shared memory");
      }
    } else {
      report_and_exit("can't open shared memory");
    }
  } else {
    if ((ftruncate(fd, sizeof(struct SM)) == -1)) {
      report_and_exit("ftruncate failed");
    }
  }

  memory_pointer = mmap(NULL, /* let system pick where to put segment */
                        sizeof(struct SM),      /* how many bytes */
                        PROT_READ | PROT_WRITE, /* access protections */
                        MAP_SHARED, /* mapping visible to other processes */
                        fd, /* file descriptor */ 0);
  if (-1 == close(fd)) {
    report_and_exit("close");
  }
  if ((caddr_t)-1 == memory_pointer) {
    report_and_exit("Can't get segment...");
  }
  shared_memory_pointer = (sm *)memory_pointer;

  /* semaphore code to lock the shared mem */
  sem = sem_open(sem_name, /* name */
                 O_CREAT,  /* create the semaphore */
                 0666,     /* protection perms */
                 0);

  /* initial value */
  if (sem == SEM_FAILED) {
    report_and_exit("sem_open");
  }

  fill_fifo_files(fifo_names_file);

  if (is_creator) {
    for (int i = 0; i < fifo_count; i++) {
      errno = 0;
      if (0 != mkfifo(fifo_file_names[i], 0666) && errno != EEXIST) {
        report_and_exit("mkfifo");
      }
    }

    if (shared_memory_pointer)
      sem_init(&(shared_memory_pointer->sem), 1, 1);
    shared_memory_pointer->fifo_index = 0;
    shared_memory_pointer->potato_c = 0;
    if (-1 == sem_post(sem)) {
      report_and_exit("sem_post");
    }
  }

  if (-1 == sem_wait(sem)) {
    report_and_exit("sem_wait");
  }

  read_index = shared_memory_pointer->fifo_index;

  if ((shared_memory_pointer->peer[read_index][POTATO_INDEX] = pot_sw_count)) {
    shared_memory_pointer->potato_c++;
  }
  shared_memory_pointer->peer[read_index][PID_INDEX] = getpid();
  shared_memory_pointer->fifo_index++;
  if (-1 == sem_post(sem)) {
    report_and_exit("sem_post");
  }

  for (int i = 0; i < fifo_count; i++) {
    if (read_index != i) {
      fifo_fd[i] = open(fifo_file_names[i], O_WRONLY);
    } else {
      fifo_fd[i] = open(fifo_file_names[i], O_RDONLY);
    }
  }

  int potato_id = read_index;
  int random_number;
  while (true) {

    // write potato to fifo
    if (-1 == sem_wait(&shared_memory_pointer->sem)) {
      report_and_exit("sem_wait");
    }
    if (shared_memory_pointer->peer[potato_id][POTATO_INDEX]) {
      random_number = select_random_index();
      printf("pid=%d sending potato number %d to %s; %d switches left\n",
             getpid(), shared_memory_pointer->peer[potato_id][PID_INDEX],
             fifo_file_names[random_number],
             shared_memory_pointer->peer[potato_id][POTATO_INDEX] - 1);
      fflush(stdout);
      write(fifo_fd[random_number], &potato_id, sizeof(int));
    }
    if (-1 == sem_post(&shared_memory_pointer->sem)) {
      report_and_exit("sem_post");
    }

    // read potato from fifo
    read(fifo_fd[read_index], &potato_id, sizeof(int));
    if (potato_id == FINISH_SIGN) {
      break;
    } else {

      // update shared memory
      if (-1 == sem_wait(&shared_memory_pointer->sem)) {
        report_and_exit("sem_wait");
      }
      printf("pid=%d receiving potato number %d from %s\n", getpid(),
             shared_memory_pointer->peer[potato_id][PID_INDEX],
             fifo_file_names[read_index]);
      // update switch count
      shared_memory_pointer->peer[potato_id][POTATO_INDEX]--;

      // update potato count
      if (shared_memory_pointer->peer[potato_id][POTATO_INDEX] == 0) {
        printf("pid=%d; potato number %d has cooled down.\n", getpid(),
               shared_memory_pointer->peer[potato_id][PID_INDEX]);
        shared_memory_pointer->potato_c--;
      }
      // handle finish case
      if (shared_memory_pointer->potato_c == 0) {
        send_finish_sign();
        if (-1 == sem_post(&shared_memory_pointer->sem)) {
          report_and_exit("sem_post");
        }
        break;
      }
      if (-1 == sem_post(&shared_memory_pointer->sem)) {
        report_and_exit("sem_post");
      }
    }
  }
  finish(0);
  return 0;
}
void send_finish_sign() {
  int s = FINISH_SIGN;
  for (int i = 0; i < fifo_count; i++) {
    if (i != read_index) {
      if (-1 == write(fifo_fd[i], &s, sizeof(int))) {
        report_and_exit("write");
      }
    }
  }

  exit(EXIT_SUCCESS);
}
void finish(int sig) {
  sem_unlink(sem_name);
  sem_close(sem);
  sem_close(&shared_memory_pointer->sem);
  shm_unlink(shared_memory_name);
  munmap(shared_memory_pointer, sizeof(sm));
  exit(sig);
}

int fill_fifo_files(const char *file_name) {

  FILE *fp;
  if (NULL == (fp = fopen(file_name, "r"))) {
    report_and_exit("fopen");
    return -1;
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
    report_and_exit("fgets");
  }
  return -1;
}

int select_random_index() {
  int random_number = read_index;
  while (random_number == read_index) {
    random_number = (rand() % fifo_count);
  }
  return random_number;
}
