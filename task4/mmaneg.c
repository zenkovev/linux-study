#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/mm_types.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("zenkov_ev");
MODULE_DESCRIPTION("MIPT Linux Process Mmaneg");
MODULE_VERSION("0.01");

// ----- Module handlers -----

static void listvma_handler(void) {
  VMA_ITERATOR(iter, current->mm, 0);
  // current: task_struct of current process
  // mm: mm_struct to manage all process memory
  // 0: starting virtual address

  struct vm_area_struct *vma = NULL;
  int counter = 0;

  printk(KERN_INFO "VMA of current process:\n");
  for_each_vma(iter, vma) {
    printk(
      KERN_INFO "- %d. start: 0x%lx, end: 0x%lx, flags: 0x%lx\n",
      counter, vma->vm_start, vma->vm_end, vma->vm_flags
    );
    counter += 1;
  }
}

static void findpage_handler(unsigned long long int addr) {
  printk(KERN_INFO "> Command: [findpage] [%llu]\n", addr);
}

static void writeval_handler(unsigned long long int addr, unsigned long int val) {
  printk(KERN_INFO "> Command: [writeval] [%llu] [%lu]\n", addr, val);
}

// ----- Process file functions -----

static ssize_t procfile_write(struct file *flip, const char __user *buffer, size_t buffer_size, loff_t *offset) {
  char* data = kmalloc(buffer_size+1, GFP_KERNEL);
  if (data == NULL) {
    return -EIO;
  }
  int err = copy_from_user(data, buffer, buffer_size);
  if (err != 0) {
    kfree(data);
    return -EIO;
  }
  *(data + buffer_size) = '\0';

  unsigned long long int address;
  unsigned long int value;
  if (strncmp(data, "listvma", strlen("listvma")) == 0) {
    listvma_handler();
  } else if (sscanf(data, "findpage %llu", &address) == 1) {
    findpage_handler(address);
  } else if (sscanf(data, "writeval %llu %lu", &address, &value) == 2) {
    writeval_handler(address, value);
  } else {
    return -EINVAL;
  }

  return buffer_size;
}

static void* procfile_seq_start(struct seq_file *s, loff_t *pos) {
  static unsigned long counter = 0;
  if (*pos == 0) {
    return &counter;
  }
  *pos = 0;
  return NULL;
}

static void* procfile_seq_next(struct seq_file *s, void *v, loff_t *pos) {
  (*pos)++;
  return NULL;
}

static void procfile_seq_stop(struct seq_file *s, void *v) {}

static int procfile_seq_show(struct seq_file *s, void *v) {
  VMA_ITERATOR(iter, current->mm, 0);
  // current: task_struct of current process
  // mm: mm_struct to manage all process memory
  // 0: starting virtual address

  struct vm_area_struct *vma = NULL;
  int counter = 0;

  seq_printf(s, "VMA of current process:\n");
  for_each_vma(iter, vma) {
    seq_printf(
      s, "- %d. start: 0x%lx, end: 0x%lx, flags: 0x%lx\n",
      counter, vma->vm_start, vma->vm_end, vma->vm_flags
    );
    counter += 1;
  }

  return 0;
}

static struct seq_operations procfile_seq_ops = {
  .start = procfile_seq_start,
  .next = procfile_seq_next,
  .stop = procfile_seq_stop,
  .show = procfile_seq_show,
};

static int procfile_open(struct inode *inode, struct file *file) {
  return seq_open(file, &procfile_seq_ops);
};

static const struct proc_ops proc_file_fops = {
  .proc_write = procfile_write,
  .proc_open = procfile_open,
  .proc_read = seq_read,
  .proc_lseek = seq_lseek,
  .proc_release = seq_release,
};

// ----- Module init/exit -----

static struct proc_dir_entry *mmaneg_proc_file;

static int __init mipt_mmaneg_init(void) {
  mmaneg_proc_file = proc_create("mmaneg", 0644, NULL, &proc_file_fops);
  if (mmaneg_proc_file == NULL) {
    printk(KERN_ALERT "Could not create /proc/mmaneg\n");
    return 1;
  } else {
    printk(KERN_INFO "/proc/mmaneg successfully created\n");
    return 0;
  }
}

static void __exit mipt_mmaneg_exit(void) {
  proc_remove(mmaneg_proc_file);
  printk(KERN_INFO "/proc/mmaneg removed\n");
}

module_init(mipt_mmaneg_init);
module_exit(mipt_mmaneg_exit);