#ifndef _ARGS_H
#define _ARGS_H

int match_command_filter(char *command);
int match_type_filter(char *type);
int match_filename_filter(char *filename);
void parse_args(int argc, char **argv);

#endif /* _ARGS_H */