//
// Created by umit on 18.04.2021.
//
#include "signal_handler.h"
sigset_t new_mask;
sigset_t old_mask;
sigset_t zero_mask;
static void handle_sigusr1(int signo) { sig_flag = signo; }
void handle_sigurs2(int signo) {
    sig_flag = signo;
}
void block_signals(void) {
    if (signal(SIGUSR1, handle_sigusr1) == SIG_ERR)
        perror("signal(SIGUSR1) error");
    if (signal(SIGUSR2, handle_sigurs2) == SIG_ERR)
        perror("signal(SIGUSR2) error");
    sigemptyset(&zero_mask);
    sigemptyset(&new_mask);
    sigaddset(&new_mask, SIGUSR1);
    sigaddset(
            &new_mask,
            SIGUSR2); /* Block SIGUSR1 and SIGUSR2, and save current signal mask */
    if (sigprocmask(SIG_BLOCK, &new_mask, &old_mask) < 0)
        perror("SIG_BLOCK error");
}
/* tell parent we’re done */
void sig_parent(pid_t pid) { kill(pid, SIGUSR2); }

void suspend_child(void) {
    while (sig_flag == 0)
        sigsuspend(&zero_mask); /* and wait for parent */

    sig_flag = 0; /* Reset signal mask to original value */
    if (sigprocmask(SIG_SETMASK, &old_mask, NULL) < 0)
        perror("SIG_SETMASK error");
}

/* tell child we’re done */


void suspend_parent(void) {
    while (sig_flag == 0)
        sigsuspend(&zero_mask); /* and wait for child */
    sig_flag = 0;               /* Reset signal mask to original value */
    if (sigprocmask(SIG_SETMASK, &old_mask, NULL) < 0)
        perror("SIG_SETMASK error");
}
void secure_wait_for_child() {
    suspend_child();
    sig_parent(getppid());
    suspend_child();
}
