#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "args.h"

#define BUF_SIZE 0x180

typedef struct {
    int valid;
    int inode;
    char path[BUF_SIZE];
} cwd_info;

typedef struct {
    int valid;
    int inode;
    char path[BUF_SIZE];
} root_info;

typedef struct {
    int valid;
    int inode;
    char path[BUF_SIZE];
} exe_info;

static inline int is_numerical_string(char *str)
{
    while (*str) {
        if ('0' > *str || *str > '9')
            return 0;
        str++;
    }

    return 1;
}

static void get_command(char *pid, char *command)
{
    int fd, ret;
    char path[BUF_SIZE];

    snprintf(path, BUF_SIZE, "/proc/%s/comm", pid);

    if ((fd = open(path, O_RDONLY)) == -1) {
        return;
    }

    while ((ret = read(fd, command, BUF_SIZE - 1))) {
        if (command[ret - 1] == '\n')
            command[ret - 1] = 0;
        else
            command[ret] = 0;
    }

    close(fd);
}

static void get_user(char *pid, char *user)
{
    struct stat info;
    struct passwd *owner_info;
    char path[BUF_SIZE];

    snprintf(path, BUF_SIZE, "/proc/%s", pid);

    if (stat(path, &info)) {
        return;
    }

    if (!(owner_info = getpwuid(info.st_uid))) {
        return;
    }

    snprintf(user, BUF_SIZE, "%s", owner_info->pw_name);
}

static void get_cwd(char *pid, cwd_info *ci)
{
    struct stat info;
    int len;
    char path[BUF_SIZE];

    ci->valid = 0;

    snprintf(path, BUF_SIZE, "/proc/%s/cwd", pid);

    if ((len = readlink(path, ci->path, BUF_SIZE - 1)) == -1) {
        if (errno == EACCES) {
            snprintf(ci->path, BUF_SIZE, "/proc/%s/cwd (Permission denied)", pid);
        }
        return;
    }

    ci->path[len] = 0;

    if (stat(path, &info)) {
        return;
    }

    ci->inode = info.st_ino;
    ci->valid = 1;
}

static void get_root(char *pid, root_info *ri)
{
    struct stat info;
    int len;
    char path[BUF_SIZE];

    ri->valid = 0;

    snprintf(path, BUF_SIZE, "/proc/%s/root", pid);

    if ((len = readlink(path, ri->path, BUF_SIZE - 1)) == -1) {
        if (errno == EACCES) {
            snprintf(ri->path, BUF_SIZE, "/proc/%s/root (Permission denied)", pid);
        }
        return;
    }

    ri->path[len] = 0;

    if (stat(path, &info)) {
        return;
    }

    ri->inode = info.st_ino;
    ri->valid = 1;
}

static void get_exe(char *pid, exe_info *ei)
{
    struct stat info;
    int len;
    char path[BUF_SIZE];

    ei->valid = 0;

    snprintf(path, BUF_SIZE, "/proc/%s/exe", pid);

    if ((len = readlink(path, ei->path, BUF_SIZE - 1)) == -1) {
        if (errno == EACCES) {
            snprintf(ei->path, BUF_SIZE, "/proc/%s/exe (Permission denied)", pid);
        } else if (errno == ENOENT) {
            snprintf(ei->path, BUF_SIZE, "/proc/%s/exe", pid);
        }
        return;
    }

    ei->path[len] = 0;

    if (stat(path, &info)) {
        return;
    }

    ei->inode = info.st_ino;
    ei->valid = 1;
}

static void print_basic_info(char *command, char *pid, char *user)
{
    printf("%s\t\t"
           "%s\t\t"
           "%s\t\t",
           command,
           pid,
           user);
}

static void print_cwd(char *command, char *pid, char *user, cwd_info *ci)
{
    print_basic_info(command, pid, user);

    if (ci->valid) {
        printf("cwd\t\t"        // FD
               "DIR\t\t"        // TYPE
               "%d\t\t"         // NODE
               "%s",            // NAME
               ci->inode,
               ci->path);
    } else {
        printf("cwd\t\t"        // FD
               "unknown\t\t"    // TYPE
               " \t\t"          // NODE
               "%s",            // NAME
               ci->path);
    }

    printf("\n");
}

static void print_root(char *command, char *pid, char *user, root_info *ri)
{
    print_basic_info(command, pid, user);

    if (ri->valid) {
        printf("rtd\t\t"        // FD
               "DIR\t\t"        // TYPE
               "%d\t\t"         // NODE
               "%s",            // NAME
               ri->inode,
               ri->path);
    } else {
        printf("rtd\t\t"        // FD
               "unknown\t\t"    // TYPE
               " \t\t"          // NODE
               "%s",            // NAME
               ri->path);
    }

    printf("\n");
}

static void print_exe(char *command, char *pid, char *user, exe_info *ei)
{
    print_basic_info(command, pid, user);

    if (ei->valid) {
        printf("txt\t\t"        // FD
               "REG\t\t"        // TYPE
               "%d\t\t"         // NODE
               "%s",            // NAME
               ei->inode,
               ei->path);
    } else {
        printf("txt\t\t"        // FD
               "unknown\t\t"    // TYPE
               " \t\t"          // NODE
               "%s",            // NAME
               ei->path);
    }

    printf("\n");
}

static void print_path_info(char *path)
{
    struct stat info;

    if (stat(path, &info)) {
        return;
    }

    // TYPE
    if (S_ISREG(info.st_mode)) {
        printf("REG\t\t");
    } else if (S_ISDIR(info.st_mode)) {
        printf("DIR\t\t");
    } else if (S_ISCHR(info.st_mode)) {
        printf("CHR\t\t");
    } else if (S_ISFIFO(info.st_mode)) {
        printf("FIFO\t\t");
    } else if (S_ISSOCK(info.st_mode)) {
        printf("SOCK\t\t");
    } else {
        printf("unknown\t\t");
    }

    // NODE
    printf("%lu\t\t", info.st_ino);

    // NAME
    printf("%s", path);
}

static void print_delmem(char *command, char *pid, char *user, char *filepath, int inode)
{
    print_basic_info(command, pid, user);

    printf("DEL\t\t"        // FD
           "REG\t\t"        // TYPE
           "%d\t\t"         // NODE
           "%s",            // NAME
           inode,
           filepath);

    printf("\n");
}

static void print_mem(char *command, char *pid, char *user, char *filepath)
{
    print_basic_info(command, pid, user);

    // FD
    printf("mem\t\t");

    print_path_info(filepath);

    printf("\n");
}

static void handle_mem_maps(char *command, char *user, struct dirent *procent)
{
    char path[BUF_SIZE];
    char buf[BUF_SIZE];
    char filepath_buf[BUF_SIZE];
    char *cur, *filepath = NULL, *pinode;
    FILE *mapfile;

    snprintf(path, BUF_SIZE, "/proc/%s/maps", procent->d_name);

    if (!(mapfile = fopen(path, "r"))) {
        return;
    }

    filepath_buf[0] = 0;

    // Find files
    while (fgets(buf, BUF_SIZE, mapfile)) {
        cur = buf;

        /*
         * maps format:
         *  start-from rwxp offset-- dev-- inode path
         */

        // skip start-from
        while (*cur != ' ') {
            cur++;
        }

        // jump to inode info
        cur += 21;

        pinode = cur;

        while (*cur != ' ') {
            cur++;
        }

        *cur++ = 0;

        // find path
        while (*cur == ' ') {
            cur++;
        }

        filepath = cur;

        // find new line
        while (*cur != '\n') {
            cur++;
        }

        *cur = 0;

        if (filepath[0] == '/' && strcmp(filepath_buf, filepath)) {
            strcpy(filepath_buf, filepath);

            if (!strcmp(&filepath[strlen(filepath) - 10], " (deleted)")) {
                filepath[strlen(filepath) - 10] = 0;
                print_delmem(command, procent->d_name, user, filepath, atoi(pinode));
            } else {
                print_mem(command, procent->d_name, user, filepath);
            }
        }
    }

    fclose(mapfile);
}

static void hadnle_fd_ent(char *command, char *user, struct dirent *procent, struct dirent *fdent)
{
    int len;
    char fdpath[BUF_SIZE];
    char path[BUF_SIZE];
    struct stat info, link_info;

    if (fdent->d_type != DT_LNK) {
        return;
    }

    if (!is_numerical_string(fdent->d_name)) {
        return;
    }
    
    print_basic_info(command, procent->d_name, user);

    snprintf(fdpath, BUF_SIZE, "/proc/%s/fd/%s", procent->d_name, fdent->d_name);

    if ((len = readlink(fdpath, path, BUF_SIZE - 1)) == -1) {
        // TODO: Handle error
        printf("\n");
        return;
    }

    path[len] = 0;
    
    if (stat(fdpath, &info)) {
        return;
    }

    if (lstat(fdpath, &link_info)) {
        return;
    }

    // FD
    printf("%s", fdent->d_name);

    switch (link_info.st_mode & 0600) {
    case 0600:
        printf("u");
        break;
    case 0400:
        printf("r");
        break;
    case 0200:
        printf("w");
        break;
    default:
        break;
    }

    printf("\t\t");

    // TYPE
    if (S_ISREG(info.st_mode)) {
        printf("REG\t\t");
    } else if (S_ISDIR(info.st_mode)) {
        printf("DIR\t\t");
    } else if (S_ISCHR(info.st_mode)) {
        printf("CHR\t\t");
    } else if (S_ISFIFO(info.st_mode)) {
        printf("FIFO\t\t");
    } else if (S_ISSOCK(info.st_mode)) {
        printf("SOCK\t\t");
    } else {
        printf("unknown\t\t");
    }

    // NODE
    printf("%lu\t\t", info.st_ino);

    // NAME
    printf("%s", path);

    printf("\n");
}

static void handle_fd_dir(char *command, char *user, struct dirent *procent)
{
    DIR *fddir;
    struct dirent *fdent;
    char path[BUF_SIZE];

    snprintf(path, BUF_SIZE, "/proc/%s/fd", procent->d_name);

    if (!(fddir = opendir(path))) {
        print_basic_info(command, procent->d_name, user);
        printf("NOFD\t\t"        // FD
               " \t\t"           // TYPE
               " \t\t"           // NODE
               "%s"              // NAME
               " (Permission denied)\n",
               path);
        return;
    }

    errno = 0;

    while ((fdent = readdir(fddir))) {
        hadnle_fd_ent(command, user, procent, fdent);
    }

    if (errno) {
        fprintf(stderr, "[x] handle_fd_dir: readdir error\n");
        exit(1);
    }

    closedir(fddir);
}

static void handle_proc_ent(struct dirent *procent)
{
    char command[BUF_SIZE];
    char user[BUF_SIZE];
    cwd_info ci;
    root_info ri;
    exe_info ei;

    if (procent->d_type != DT_DIR) {
        return;
    }

    if (!is_numerical_string(procent->d_name)) {
        return;
    }

    // get command
    get_command(procent->d_name, command);

    // get user
    get_user(procent->d_name, user);

    // get cwd
    get_cwd(procent->d_name, &ci);

    // print cwd
    print_cwd(command, procent->d_name, user, &ci);

    // get root
    get_root(procent->d_name, &ri);

    // print root
    print_root(command, procent->d_name, user, &ri);

    // get exe
    get_exe(procent->d_name, &ei);

    // print exe
    print_exe(command, procent->d_name, user, &ei);

    // handle maps
    handle_mem_maps(command, user, procent);

    // handle fd
    handle_fd_dir(command, user, procent);
}

static void print_banner()
{
    printf("COMMAND\t\t"
           "PID\t\t"
           "USER\t\t"
           "FD\t\t"
           "TYPE\t\t"
           "NODE\t\t"
           "NAME\n");
}

int main(int argc, char **argv)
{
    DIR *procdir;
    struct dirent *procent;

    parse_args(argc, argv);

    if (!(procdir = opendir("/proc"))) {
        fprintf(stderr, "[x] Cannot open /proc\n");
        exit(1);
    }

    errno = 0;
    
    print_banner();
    while ((procent = readdir(procdir))) {
        handle_proc_ent(procent);
    }
    
    if (errno) {
        fprintf(stderr, "[x] main: readdir error\n");
        exit(1);
    }

    return 0;
}