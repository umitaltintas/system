//
// Created by umit on 17.04.2021.
//
#include "usage.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {

  int c;
  char *shared;
  char *fifo;
  int pat;
  if (argc < 7) {
    printUsage();
    exit(EXIT_FAILURE);
  }
  while ((c = getopt(argc, argv, "b:s:f:")) != -1) {
    switch (c) {
    case 'f':
      printf("f: %s\n", optarg);
      fifo = optarg;
      break;
    case 'b':
      printf("b: %s\n", optarg);
      pat = atoi(optarg);
      break;
    case 's':
      printf("s: %s\n", optarg);
      shared = optarg;
      break;
    default:
      printUsage();
      exit(EXIT_FAILURE);
    }
  }
  printf("shared: %s , fifo: %s, pat: %d", shared, fifo, pat);

  return 0;
}