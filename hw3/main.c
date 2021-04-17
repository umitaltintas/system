//
// Created by umit on 17.04.2021.
//
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include "usage.h"

int main(int argc, char **argv) {

    int opt;

    if (argc < 2) {
        printUsage();
        exit(EXIT_FAILURE);
    }
    int c;
    while ((c = getopt(argc, argv, "b:s:f:")) != -1) {
        switch (c) {
            case 'f':
                printf("f: %s\n", optarg);
                break;
            case 'b':
                printf("b: %s\n", optarg);
                break;
            case 's':
                printf("s: %s\n", optarg);
                break;

            default:
                printUsage();
                exit(EXIT_FAILURE);
        }


    }
}