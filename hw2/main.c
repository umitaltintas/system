#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

//#include <string.h>
double Pn(int n, double x[n + 1], double X);

int child_thread(int fd);

double Li(int i, int n, const double x[n + 1], double X);

int block_file(int fd);

void unblock_file(int fd);

int main(void) {

    /*
     * shared operations for child's and mother
     * */

    int fd = open("input.txt", O_RDWR);

    if (fd == -1) {
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
                child_thread(fd);
                return 0;
            } else {
//                printf("process created: %d\n",c_pid[i]);
            }
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
    while (-1 != (t_id = wait(&stat)) || errno != ECHILD) {
//        printf("%d is finished with status: %d\n", t_id, stat);
    }
    close(fd);

    return 0;
}

/*
 *child process's main function.
 */

int child_thread(int fd) {
    FILE *fp = fdopen(fd, "r");
    block_file(fd);








    /*read from file*/

    char *buffer = (char *) malloc(sizeof(char) * 250);
    fgets(buffer, 250, fp);
    char *tmp = buffer;


    double fun[16];
    char *end;
    for (int i = 0; i < 16; i++) {
        fun[i] = strtod(buffer, &end);
        buffer = end + 1;
    }

    printf("child:%d\n", getpid());
//    for (int i = 0; i <8;i++){
//        printf("x%d:%lf, y:%lf  ",i,fun[i*2],fun[i*2+1]);
//    }
    printf("\n calculate5 : %lf ", Pn(5, fun, fun[14]));
    printf("calculate6: %lf \n", Pn(6, fun, fun[14]));
    unblock_file(fd);


    free(tmp);
    return 0;
}

void unblock_file(int fd) {/* unlock file*/
    if (fcntl(fd, F_UNLCK, NULL) == -1) {
        perror("fcntl");
        exit(1);
    }
}

int block_file(int fd) {
    struct flock fl = {.l_type = F_UNLCK, .l_start =0, .l_len=0, .l_whence=SEEK_SET};
    /*  lock file for writing*/
    fl.l_type = F_WRLCK;
    if (fcntl(fd, F_SETLKW, &fl) == -1) {
        perror("fcntl");
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

/*Function to evaluate Pn(x) where Pn is the Lagrange interpolating polynomial of degree n*/

double Pn(int n, double x[], double X) {
    double sum = 0;
    int i;
    for (i = 0; i <= n; i++) {
        sum = sum + Li(i, n, x, X) * x[2 * i + 1];
    }
    return sum;
}
