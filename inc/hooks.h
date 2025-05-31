#ifndef PSY_HOOKS_H
#define PSY_HOOKS_H

#include <linux/types.h>
#include <linux/ptrace.h>
#include <linux/sched.h>
#include <linux/cred.h>
#include <linux/kprobes.h>

bool hInstall(void);
void hUninstall(void);

#endif
