//
// Created by umit on 18.04.2021.
//
#include "signal_handler.h"

static void handle_sigusr1(int signo) { sigflag = 1; }
void handle_sigurs2(int signo) {
  sigflag = 1;
  suspended_count++;
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
  while (sigflag == 0)
    sigsuspend(&zero_mask); /* and wait for parent */

  sigflag = 0; /* Reset signal mask to original value */
  if (sigprocmask(SIG_SETMASK, &old_mask, NULL) < 0)
    perror("SIG_SETMASK error");
}

/* tell child we’re done */

void sig_child(pid_t pid) { kill(pid, SIGUSR1); }

void suspend_parent(void) {
  while (sigflag == 0)
    sigsuspend(&zero_mask); /* and wait for child */
  sigflag = 0;              /* Reset signal mask to original value */
  if (sigprocmask(SIG_SETMASK, &old_mask, NULL) < 0)
    perror("SIG_SETMASK error");
}
void secure_wait_for_child() {
  suspend_child();
  sig_parent(getppid());
  suspend_child();
}
