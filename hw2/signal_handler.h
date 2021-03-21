//
// Created by umit on 18.04.2021.
//

#ifndef HW2_SIGNAL_HANDLER_H
#define HW2_SIGNAL_HANDLER_H
#include "signal_handler.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static volatile sig_atomic_t suspended_count = 0;
static volatile sig_atomic_t sigflag=0;
static sigset_t new_mask;
static sigset_t old_mask;
static sigset_t zero_mask;
void secure_wait_for_child();
void sig_child(pid_t pid);
void suspend_child(void);
void suspend_parent(void);
void block_signals(void);
void sig_parent(pid_t pid);

#endif // HW2_SIGNAL_HANDLER_H
