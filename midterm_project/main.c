// C program to demonstrate working of fork()
#include "file_locking.h"
#include "signal_handler.h"
#include <errno.h>
#include <fcntl.h> /* For O_* constants */
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h> /* For mode constants */
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

struct FRIDGE {
    int second;
    int first;
    sem_t sem_nurse;
    sem_t sem_common;
    sem_t sem_vac;
};
#define MAX_CITIZEN_COUNT 1000
struct ROOM {
    pid_t citizens[MAX_CITIZEN_COUNT];
    int citizen_index;
    int tour;
    sem_t sem_vac;
};

int n_nurses;
int n_vaccinators;
int n_citizens;
int size_of_buffer;
int n_shot;
char *path_name;
int fridge_fd;
int room_fd;
int fd;
struct FRIDGE *fridge_p;
struct ROOM *room_p;


#define MaxItems 5// Maximum items a producer can produce or a consumer can consume
#define BufferSize 5// Size of the buffer

#define FRIDGE_KEY "fridge_key"
#define ROOM_KEY "room_key"
#define MAX_NURSES_COUNT 100
int buffer[BufferSize];
int printUsage();
int vaccinator_child();
int nurse_child();
int citizen_child();
void signal_handler(int);
int main(int argc, char *argv[]) {
    int nurses[MAX_NURSES_COUNT];
    if (argc < 13) {
        printUsage();
        exit(EXIT_FAILURE);
    }
    int c;
    while ((c = getopt(argc, argv, "n:v:c:b:t:i")) != -1) {
        switch (c) {
            case 'n':

                n_nurses = atoi(optarg);
                break;
            case 'v':

                n_vaccinators = atoi(optarg);
                break;
            case 'c':

                n_citizens = atoi(optarg);
                break;
            case 'b':
                size_of_buffer = atoi(optarg);
                break;
            case 't':
                n_shot = atoi(optarg);
                break;
            case 'i':
                path_name = optarg;
                break;
            default:
                printUsage();
                exit(EXIT_FAILURE);
        }
    }
    printf("Welcome to the GTU344 clinic. Number of citizens to vaccinate c=%d with t=%d doses.\n", n_citizens, n_shot);
    fridge_fd = shm_open(FRIDGE_KEY, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (fridge_fd == -1) {
        perror("shm_open");
        return -1;
    }

    room_fd = shm_open(ROOM_KEY, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (room_fd == -1) {
        perror("shm_open");
        return -1;
    }

    int err = shm_unlink(FRIDGE_KEY);

    if (err == -1) {
        perror("shm_unlink");
        return -1;
    }


    err = shm_unlink(ROOM_KEY);

    if (err == -1) {
        perror("shm_unlink");
        return -1;
    }

    err = ftruncate(fridge_fd, sizeof(struct FRIDGE));

    if (err == -1) {
        perror("ftruncate");
        return -1;
    }

    err = ftruncate(room_fd, sizeof(struct ROOM));

    if (err == -1) {
        perror("ftruncate");
        return -1;
    }


    fridge_p = mmap(NULL, sizeof(struct FRIDGE), PROT_READ | PROT_WRITE, MAP_SHARED, fridge_fd, 0);

    if (fridge_p == MAP_FAILED) {
        perror("mmap");
        return -1;
    }

    room_p = mmap(NULL, sizeof(struct ROOM), PROT_READ | PROT_WRITE, MAP_SHARED, fridge_fd, 0);

    if (room_p == MAP_FAILED) {
        perror("mmap");
        return -1;
    }
    sem_init(&(fridge_p->sem_common), 1, 1);
    sem_init(&(fridge_p->sem_nurse), 1, size_of_buffer);
    sem_init(&(fridge_p->sem_vac), 1, 0);
    sem_init(&(room_p->sem_vac), 1, 0);
    fd = open(path_name, O_RDONLY);
    if (fd == 1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    fridge_p->first = 0;
    fridge_p->second = 0;
    room_p->citizen_index = 0;


    for (int i = 0; i < n_citizens; ++i) {
        if ((room_p->citizens[i] = (fork() == 0))) {
            signal(SIGINT, signal_handler);
            citizen_child();
            exit(0);
        }
    }
    for (int i = 0; i < n_nurses; ++i) {
        if ((nurses[i] = fork()) == 0) {
            signal(SIGINT, signal_handler);
            nurse_child();
            exit(0);
        }
    }

    for (int i = 0; i < n_vaccinators; ++i) {
        if (fork() == 0) {
            signal(SIGINT, signal_handler);
            vaccinator_child();
            exit(0);
        }
    }


    int status;
    for (int i = 0; i < n_nurses; i++) {
        int w = waitpid(nurses[i], &status, WEXITED);
        if (w == -1) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }
    }
    printf("Nurses have carried all vaccines to the buffer, terminating.\n");
    for (int i = 0; i < n_nurses; i++) {
        int w = waitpid(room_p->citizens[i], &status, WEXITED);
        if (w == -1) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }
    }
    printf("All citizens have been vaccinated .\n");

    while (wait(NULL) > 0)
        ;
    printf("The clinic i;s now closed. Stay healthy.");
    return 0;
}


int vaccinator_child(int id) {
    int index;
    pid_t pid;
    int vac_count = 0;
    do {
        sem_wait(&fridge_p->sem_vac);
        sem_wait(&fridge_p->sem_common);
        fridge_p->first--;
        fridge_p->second--;
        sem_post(&fridge_p->sem_common);
        sem_wait(&room_p->sem_vac);
        if (room_p->citizen_index >= n_shot * n_citizens)
            break;
        index = room_p->citizen_index;
        pid = room_p->citizens[index % n_citizens];
        sem_post(&room_p->sem_vac);
        sig_child(pid);
        sem_wait(&fridge_p->sem_common);
        printf("Citizen %d (pid=%d) is vaccinated for the %dth time: the clinic has %d vaccine1 and %d vaccine2\n",
               index, pid, index / n_citizens + 1, fridge_p->first, fridge_p->second);
        sem_post(&fridge_p->sem_common);
        sig_child(pid);
        vac_count++;

    } while (true);
    printf("Vaccinator %d (pid=%d) vaccinated %d doses.", id, getpid(), vac_count);
    return 0;
}
int nurse_child(int id) {
    char read_char;
    int read_int;
    size_t ret_val;
    do {
        block_file(fd);
        ret_val = read(fd, &read_char, sizeof(char));
        unblock_file(fd);
        if (ret_val < 1) {
            break;
        }
        read_int = read_char - '0';
        sem_wait(&fridge_p->sem_nurse);
        sem_wait(&fridge_p->sem_common);
        if (read_int == 1) {
            fridge_p->first++;
            if (fridge_p->second >= fridge_p->first) {
                sem_post(&fridge_p->sem_vac);
            }
        } else if (read_int == 2) {
            fridge_p->second++;
            if (fridge_p->first >= fridge_p->second) {
                sem_post(&fridge_p->sem_vac);
            }
        }
        printf("Nurse %d has brought vaccine %d: the clinic has %d vaccine1 and %d vaccine2 ",
               id, read_int, fridge_p->first, fridge_p->second);
        sem_post(&fridge_p->sem_common);
    } while (true);

    return 0;
}
int citizen_child() {
    block_signals();
    for (int i = 0; i < n_shot; i++) { suspend_child(); }
    return 0;
}