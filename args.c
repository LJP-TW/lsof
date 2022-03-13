#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <regex.h>

#include "args.h"

int filter_enable_c;
int filter_enable_f;
int filter_enable_t;
regex_t regex_command_filter;
regex_t regex_type_filter;
regex_t regex_filename_filter;

char *valid_type_filter[] = {"REG", "CHR", "DIR", "FIFO", "SOCK", "unknown"};

int match_command_filter(char *command)
{
    int nonmatch;

    if (filter_enable_c) {
        nonmatch = regexec(&regex_command_filter, command, (size_t) 0, NULL, 0);

        if (nonmatch) {
            return 0;
        }
    }
    
    return 1;
}

int match_type_filter(char *type)
{
    int nonmatch;

    if (filter_enable_t) {
        nonmatch = regexec(&regex_type_filter, type, (size_t) 0, NULL, 0);

        if (nonmatch) {
            return 0;
        }
    }
    
    return 1;
}

int match_filename_filter(char *filename)
{
    int nonmatch;

    if (filter_enable_f) {
        nonmatch = regexec(&regex_filename_filter, filename, (size_t) 0, NULL, 0);

        if (nonmatch) {
            return 0;
        }
    }
    
    return 1;
}

static inline void check_args_exist(int argc, char **argv, int n)
{
    if (n + 1 >= argc) {
        fprintf(stderr, "[x] %s needs a argument\n", argv[n]);
        exit(1);
    }
}

static inline void *safe_malloc(int size)
{
    void *p = malloc(size);
    if (!p) {
        fprintf(stderr, "[x] Cannot malloc\n");
        exit(1);
    }
    return p;
}



void parse_args(int argc, char **argv)
{
    if (argc == 1)
        return;

    for (int n = 1; n < argc; n++) {
        int len, valid = 0;

        if (argv[n][0] != '-') {
            fprintf(stderr, "[x] Not support argument: %s\n", argv[n]);
            exit(1);
        }

        // TODO: Handle arguments
        switch (argv[n][1]) {
        case 'c':
            check_args_exist(argc, argv, n);

            if (filter_enable_c) {
                fprintf(stderr, "[x] %s already set\n", argv[n]);
                exit(1);
            }
        
            if (regcomp(&regex_command_filter, argv[n+1], REG_EXTENDED|REG_NOSUB) != 0) {
                fprintf(stderr, "[x] regcomp failed\n");
                exit(1);
            }

            filter_enable_c = 1;

            break;
        case 'f':
            check_args_exist(argc, argv, n);

            if (filter_enable_f) {
                fprintf(stderr, "[x] %s already set\n", argv[n]);
                exit(1);
            }

            if (regcomp(&regex_filename_filter, argv[n+1], REG_EXTENDED|REG_NOSUB) != 0) {
                fprintf(stderr, "[x] regcomp failed\n");
                exit(1);
            }

            filter_enable_f = 1;

            break;
        case 't':
            check_args_exist(argc, argv, n);

            if (filter_enable_t) {
                fprintf(stderr, "[x] %s already set\n", argv[n]);
                exit(1);
            }

            for (int i = 0; i < sizeof(valid_type_filter) / sizeof(char *); ++i) {
                if (!strcmp(valid_type_filter[i], argv[n+1])) {
                    valid = 1;
                    break;
                }
            }

            if (valid) {
                if (regcomp(&regex_type_filter, argv[n+1], REG_EXTENDED|REG_NOSUB) != 0) {
                    fprintf(stderr, "[x] regcomp failed\n");
                    exit(1);
                }
            } else {
                fprintf(stderr, "Invalid TYPE option.\n");
                exit(1);
            }

            filter_enable_t = 1;

            break;
        default:
            fprintf(stderr, "[x] Not support argument: %s\n", argv[n]);
            exit(1);
        }

        n += 1;
    }
}