
#include "common.h"
int printUsage();
int vaccinator_child();
int nurse_child();
int citizen_child(int fd);
void signal_handler(int);
void take_vaccine_from_fridge();
void wait_until_vaccine_available();
void tell_fridge_available();
void wait_until_room_available();
bool is_vaccination_finished();
void tell_room_available();
void wait_for_empty_fridge_slot();
void wait_fridge_until_available();
void tell_vaccine_available();
void tell_slot_available();
void print_nurse_and_fridge_status(int id, int read_int);
void print_citizen_vactination_status(int index, pid_t pid);
void print_vaccinator_status(int id, int vac_count);
void call_citizen(int);
int fd_is_valid(int fd);
int main(int argc, char *argv[]) {
    int nurses[MAX_NURSES_COUNT];

    if (argc < 13) {
        printUsage();
        exit(EXIT_FAILURE);
    }
    int c;
    printf("%d\n", argc);
    while ((c = getopt(argc, argv, "n:v:c:b:t:i:")) != -1) {
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


    fridge_p = mmap(NULL, sizeof(struct FRIDGE), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (fridge_p == MAP_FAILED) {
        perror("mmap");
        return -1;
    }

    room_p = mmap(NULL, sizeof(struct ROOM), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (room_p == MAP_FAILED) {
        perror("mmap");
        return -1;
    }
    sem_init(&(fridge_p->sem_common), 1, 1);
    sem_init(&(fridge_p->sem_nurse), 1, size_of_buffer);
    sem_init(&(fridge_p->sem_vac), 1, 0);
    sem_init(&(room_p->sem_vac), 1, 1);
    fd = open(path_name, O_RDWR);
    if (fd == 1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    fridge_p->first = 0;
    fridge_p->second = 0;
    room_p->citizen_index = 0;


    int pidd;
    for (int i = 0; i < n_citizens; ++i) {
        if (-1 == pipe(pipes[i])) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
        if ((pidd = fork()) == 0) {
            close(pipes[i][1]);
            signal(SIGINT, signal_handler);
            citizen_child(pipes[i][0]);
            exit(0);
        } else {
            close(pipes[i][0]);
            room_p->citizens[i] = pidd;
        }
    }
    for (int i = 0; i < n_nurses; ++i) {
        if ((nurses[i] = fork()) == 0) {
            signal(SIGINT, signal_handler);
            nurse_child(i);
            exit(0);
        }
    }

    for (int i = 0; i < n_vaccinators; ++i) {
        if (fork() == 0) {
            signal(SIGINT, signal_handler);
            vaccinator_child(i);
            exit(0);
        }
    }


    int status;
    for (int i = 0; i < n_nurses; i++) {
        int w = waitpid(nurses[i], &status, 0);
        if (w == -1) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }
    }
    printf("Nurses have carried all vaccines to the buffer, terminating.\n");
    for (int i = 0; i < n_citizens; i++) {
        int w = waitpid(room_p->citizens[i], &status, 0);
        if (w == -1) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }
    }
    printf("All citizens have been vaccinated .\n");
    errno = 0;
    while (-1 != wait(NULL) || errno != ECHILD)
        ;


    printf("The clinic is now closed. Stay healthy.");


    sem_destroy(&room_p->sem_vac);
    sem_destroy(&fridge_p->sem_common);
    sem_destroy(&fridge_p->sem_vac);
    sem_destroy(&fridge_p->sem_nurse);

    int err = munmap(fridge_p, sizeof(struct FRIDGE));
    if (err == -1) {
        perror("munmap");
        return -1;
    }
    err = munmap(room_p, sizeof(struct ROOM));
    if (err == -1) {
        perror("munmap");
        return -1;
    }
    return 0;
}


int vaccinator_child(int id) {
    int index;
    pid_t pid;
    int vac_count = 0;
    for (int i = 0; i < n_citizens; ++i) {
        close(pipes[i][0]);
    }
    do {
        if(sig_flag !=0)break;
        wait_until_room_available();
        if (is_vaccination_finished()) {
            for (int i = 0; i < n_vaccinators; ++i) {
                tell_room_available();
            }
            break;
        }
        wait_until_vaccine_available();


        take_vaccine_from_fridge();


        tell_slot_available();
        tell_fridge_available();


        index = room_p->citizen_index++;
        pid = room_p->citizens[index % n_citizens];
        tell_room_available();


        printf("Vaccinator %d (pid=%d) is inviting citizen pid=%d to the clinic\n", id, getpid(), pid);
        call_citizen(index % n_citizens);
        fflush(stdout);
        print_citizen_vactination_status(index, pid);

        vac_count++;
    } while (true);
    for (int i = 0; i < n_citizens; ++i) {
        close(pipes[i][1]);
    }
    print_vaccinator_status(id, vac_count);
    return 0;
}
void call_citizen(int index) {
    char c = 'c';
    if (!fd_is_valid(pipes[index][1])) {

        fprintf(stderr, "invalid to write\n");
    }
    ssize_t r = write(pipes[index][1], &c, sizeof(char));
    if (r <= 0) {
        fprintf(stderr, "could not write\n");
        fflush(stderr);
    }
}
int nurse_child(int id) {

    char read_char;
    int read_int;
    size_t ret_val;
    do {
        if(sig_flag !=0)break;
        block_file(fd);
        ret_val = read(fd, &read_char, sizeof(char));
        unblock_file(fd);
        if (ret_val < 1) {
            break;
        }
        read_int = read_char - '0';

        wait_for_empty_fridge_slot();
        wait_fridge_until_available();


        if (read_int == 1) {
            fridge_p->first++;
            if (fridge_p->second >= fridge_p->first) {
                tell_vaccine_available();
            }
        } else if (read_int == 2) {
            fridge_p->second++;
            if (fridge_p->first >= fridge_p->second) {
                tell_vaccine_available();
            }
        }
        print_nurse_and_fridge_status(id, read_int);
        tell_fridge_available();
    } while (true);

    return 0;
}

int citizen_child(int fdr) {
    if (!fd_is_valid(fd)) {

        printf("invalid\n");
        return -1;
    }
    char r;
    for (int i = 0;( i < n_shot)&&(sig_flag ==0); i++) {


        if (-1 == read(fdr, &r, sizeof(char))) {
            printf("error read\n");
        }

        fflush(stdout);
    }

    return 0;
}


void tell_vaccine_available() { sem_post(&fridge_p->sem_vac); }
void wait_fridge_until_available() { sem_wait(&fridge_p->sem_common); }
void wait_for_empty_fridge_slot() { sem_wait(&fridge_p->sem_nurse); }
void tell_slot_available() { sem_post(&fridge_p->sem_nurse); }


void tell_room_available() { sem_post(&room_p->sem_vac); }
void wait_until_room_available() { sem_wait(&room_p->sem_vac); }
void tell_fridge_available() {
    sem_post(&fridge_p->sem_common);
}
void wait_until_vaccine_available() {
    sem_wait(&fridge_p->sem_vac);
    sem_wait(&fridge_p->sem_common);
}
void take_vaccine_from_fridge() {
    fridge_p->first--;
    fridge_p->second--;
}
bool is_vaccination_finished() {
    bool r = room_p->citizen_index >= n_shot * n_citizens;
    return r;
}

void signal_handler(int sig) {
    sig_flag = sig;
}
int printUsage() {
    printf("usage\n");
    return 0;
}
void print_nurse_and_fridge_status(int id, int read_int) {
    printf("Nurse %d has brought vaccine %d: the clinic has %d vaccine1 and %d vaccine2 \n",
           id, read_int, fridge_p->first, fridge_p->second);
    fflush(stdout);
}
void print_vaccinator_status(int id, int vac_count) {
    printf("Vaccinator %d (pid=%d) vaccinated %d doses.", id, getpid(), vac_count);
    fflush(stdout);
}
void print_citizen_vactination_status(int index, pid_t pid) {
    printf("Citizen %d (pid=%d) is vaccinated for the %dth time: the clinic has %d vaccine1 and %d vaccine2\n",
           index % n_citizens, pid, index / n_citizens + 1, fridge_p->first, fridge_p->second);
    fflush(stdout);
}
int fd_is_valid(int fdr) {
    return fcntl(fdr, F_GETFD) != -1 || errno != EBADF;
}