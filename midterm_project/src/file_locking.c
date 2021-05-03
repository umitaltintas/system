//
// Created by umit on 18.04.2021.
//

#include "file_locking.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>

#include <unistd.h>
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