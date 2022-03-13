#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

char *arg_command_filter;
char *arg_type_filter;
char *arg_filename_filter;

static inline void check_args_exist(int argc, char **argv, int n)
{
    if (n + 1 >= argc) {
        fprintf(stderr, "[x] %s needs a argument\n", argv[n]);
        exit(1);
    }
}

static inline void check_args_null(char *arg, char *p) 
{
    if (p) {
        fprintf(stderr, "[x] %s already set\n", arg);
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
        int len;

        if (argv[n][0] != '-') {
            fprintf(stderr, "[x] Not support argument: %s\n", argv[n]);
            exit(1);
        }

        // TODO: Handle arguments
        switch (argv[n][1]) {
        case 'c':
            check_args_exist(argc, argv, n);
            printf("-c: %s\n", argv[n+1]);

            check_args_null(argv[n], arg_command_filter);

            len = strlen(argv[n+1]) + 1;
            arg_command_filter = safe_malloc(len);
            memcpy(arg_command_filter, argv[n+1], len);

            break;
        case 'f':
            check_args_exist(argc, argv, n);
            printf("-f: %s\n", argv[n+1]);

            check_args_null(argv[n], arg_filename_filter);

            len = strlen(argv[n+1]) + 1;
            arg_filename_filter = safe_malloc(len);
            memcpy(arg_filename_filter, argv[n+1], len);

            break;
        case 't':
            check_args_exist(argc, argv, n);
            printf("-t: %s\n", argv[n+1]);

            check_args_null(argv[n], arg_type_filter);

            len = strlen(argv[n+1]) + 1;
            arg_type_filter = safe_malloc(len);
            memcpy(arg_type_filter, argv[n+1], len);

            break;
        default:
            fprintf(stderr, "[x] Not support argument: %s\n", argv[n]);
            exit(1);
        }

        n += 1;
    }
}