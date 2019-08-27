#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

#define MAX_STRING (100)

struct proc {
    int pid;
    char *path;
    char *command;
    struct dirent *entry;
};

void print_proc(struct proc *p) {
    printf("PATH: %s        PID: %d         CMD: %s\n", p->path, p->pid, p->command);
}

// prob a better way to do this but ehh 
// appends s2 to the end of s1
char *string_append(char *s1, char *s2) {
    s1 = realloc(s1, strnlen(s1, MAX_STRING) + strnlen(s2, MAX_STRING) + 2);
    strncat(s1, "/", 1);
    strncat(s1, s2, MAX_STRING);
    return s1;
}

int get_next_proc(DIR *proc_dirp, struct proc *p) {
    if (proc_dirp == NULL || p == NULL) {
        return -1;
    }
    struct dirent *entry;
    while ((entry = readdir(proc_dirp)) != NULL) {
        // apparently you just check if the d_name starts with a digit
        // see notes/procfs
        if (entry->d_type == DT_DIR
            && (*entry->d_name > '0' && *entry->d_name <= '9')) {
            // found a proc
            p->pid = strtoul(entry->d_name, NULL, 10);
            p->entry = entry;
            // lol am i going to segfault
            p->path = string_append(p->path, entry->d_name); 
            return 0;
        }
    }

    // no more entries
    return -1;
}

int proc_get_command(struct proc *p) {
    if (p == NULL || p->entry == NULL) {
        return -1;
    }
    char *comm_file_path = malloc(strnlen(p->path, MAX_STRING) + strlen("/comm") + 1);

    // TODO figure out why shit goes bad if this is strncpy
    strcpy(comm_file_path, p->path);
    strncat(comm_file_path, "/comm", 5);

    FILE *fp = fopen(comm_file_path, "r");
    if (fp == NULL) {
        return -1;
    }

    size_t len;
    if (p->command) {
        free(p->command);
        p->command = NULL;
    }
    if (getline(&p->command, &len, fp) == -1) {
        return -1;
    }

    fclose(fp);
    free(comm_file_path);
    return 0;
}

void proc_reset(struct proc *p) {
    p->path = realloc(p->path, strlen("/proc" + 1));
    p->command = realloc(p->command, strlen("invalid") + 1);
    strcpy(p->path, "/proc");
    strcpy(p->command, "invalid");
}

int main(void) {
    DIR *dirp = opendir("/proc");
    struct proc p;
    p.path = malloc(strlen("/proc") + 1);
    strcpy(p.path, "/proc");

    p.command = malloc(strlen("invalid")+1);
    strcpy(p.command, "invalid");

    while (get_next_proc(dirp, &p) > -1) {
        if (proc_get_command(&p) < 0) {
            printf("failed to get proc command\n");
            p.command = realloc(p.command, strlen("invalid")+1);
            strcpy(p.command, "invalid");
        }
        print_proc(&p);
        proc_reset(&p);
    }

    free(p.path);
    free(p.command);
    return 0;
}
