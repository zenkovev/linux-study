#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Melges");
MODULE_DESCRIPTION("MIPT Example");
MODULE_VERSION("0.01");

static int __init mipt_example_init(void) {
  printk(KERN_INFO "Hello, World!\n");
  return 0;
}
static void __exit mipt_example_exit(void) {
  printk(KERN_INFO "Goodbye, World!\n");
}

module_init(mipt_example_init);
module_exit(mipt_example_exit);
