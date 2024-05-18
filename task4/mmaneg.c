#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("zenkov_ev");
MODULE_DESCRIPTION("MIPT Linux Process Mmaneg");
MODULE_VERSION("0.01");

static int __init mipt_mmaneg_init(void) {
  printk(KERN_INFO "Hello, Mmaneg!\n");
  return 0;
}

static void __exit mipt_mmaneg_exit(void) {
  printk(KERN_INFO "Goodbye, Mmaneg!\n");
}

module_init(mipt_mmaneg_init);
module_exit(mipt_mmaneg_exit);
