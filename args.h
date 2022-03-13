#ifndef _ARGS_H
#define _ARGS_H

extern char *arg_command_filter;
extern char *arg_type_filter;
extern char *arg_filename_filter;

int parse_args(int argc, char **argv);

#endif /* _ARGS_H */