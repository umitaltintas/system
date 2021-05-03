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


static volatile sig_atomic_t sig_flag = 0;

void secure_wait_for_child();

void suspend_child(void);
void suspend_parent(void);
void block_signals(void);
void sig_parent(pid_t pid);

#endif// HW2_SIGNAL_HANDLER_H
