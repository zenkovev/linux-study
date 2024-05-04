#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/hashtable.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("zenkov_ev");
MODULE_DESCRIPTION("MIPT Linux Phone Book");
MODULE_VERSION("0.01");

// ----- Declarations of read-write operations -----

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
static long device_ioctl(struct file *, unsigned int, unsigned long);

static struct file_operations file_ops = {
  .read = device_read,
  .write = device_write,
  .open = device_open,
  .release = device_release,
  .unlocked_ioctl = device_ioctl
};

// ----- Hashmap for data storage -----

struct phone_user_data {
    int key; // technical field
    char *full_name;
    int age;
    char *telephone;
    char *email;    
    struct hlist_node node; // technical field
};

static int get_key_from_full_name(char *full_name) {
  int key = 0;
  for (char *ptr = full_name; *ptr != '\0'; ++ptr) {
    key += *ptr;
  }
  return key;
}

static char* phone_user_data_to_string(struct phone_user_data *data) {
  int full_name_len = strlen(data->full_name);
  int age_len = 3;
  int telephone_len = strlen(data->telephone);
  int email_len = strlen(data->email);
  int len = full_name_len + age_len + telephone_len + email_len + 5;

  char* buffer = kmalloc(len, GFP_KERNEL);
  if (buffer == NULL) {
    return buffer;
  }

  snprintf(
    buffer, len, "%s\t%d\t%s\t%s\n", data->full_name, data->age, data->telephone, data->email
  );
  return buffer;
}

static DEFINE_HASHTABLE(phone_book_map, 10);

static void phone_book_map_init(void) {
  hash_init(phone_book_map);
}

static void phone_book_map_free(void) {
  int counter;
  struct phone_user_data *entry;
  struct hlist_node *tmp;
  hash_for_each_safe(phone_book_map, counter, tmp, entry, node) {
    hash_del(&entry->node);
    kfree(entry->full_name);
    kfree(entry->telephone);
    kfree(entry->email);
    kfree(entry);
  }
}

static void phone_book_map_add(struct phone_user_data *data) {
  data->key = get_key_from_full_name(data->full_name);
  hash_add(phone_book_map, &data->node, data->key);
}

static struct phone_user_data* phone_book_map_find(char* full_name) {
  int key = get_key_from_full_name(full_name);
  struct phone_user_data *data = NULL;

  struct phone_user_data *entry;
  hash_for_each_possible(phone_book_map, entry, node, key) {
    if (strcmp(entry->full_name, full_name) == 0) {
      data = entry;
    }
  }
  
  return data;
}

static void phone_book_map_del(char* full_name) {
  int key = get_key_from_full_name(full_name);
  struct phone_user_data *entry;
  struct hlist_node *tmp;
  hash_for_each_possible_safe(phone_book_map, entry, tmp, node, key) {
    if (strcmp(entry->full_name, full_name) == 0) {
      hash_del(&entry->node);
      kfree(entry->full_name);
      kfree(entry->telephone);
      kfree(entry->email);
      kfree(entry);
    }
  }
}

static char* phone_book_map_to_string(void) {
  char* output = kmalloc(1024, GFP_KERNEL);
  if (output == NULL) {
    return output;
  }
  int capacity = 1024;

  *output = '\0';
  int begin = 0;

  int counter;
  struct phone_user_data *entry;
  hash_for_each(phone_book_map, counter, entry, node) {
    char* text = phone_user_data_to_string(entry);
    int text_len = strlen(text);

    while (begin + text_len + 1 > capacity) {
      capacity *= 2;
      output = krealloc(output, capacity, GFP_KERNEL);
      if (output == NULL) {
        return output;
      }
    }

    memcpy(output + begin, text, text_len);
    begin += text_len;
    *(output + begin) = '\0';
  }

  return output;
}

// ----- Open/close functions -----

static int device_open_count = 0;
char* output_text = NULL;
int output_len = 0;

static int device_open(struct inode *inode, struct file *file) {
  // If device is open, return busy
  if (device_open_count >= 1) {
    return -EBUSY;
  }

  output_text = phone_book_map_to_string();
  if (output_text != NULL) {
    output_len = strlen(output_text);
  } else {
    output_len = 0;
  }
  
  device_open_count++;
  try_module_get(THIS_MODULE);
  return 0;
}

static int device_release(struct inode *inode, struct file *file) {
  if (output_text != NULL) {
    kfree(output_text);
  }
  output_text = NULL;
  output_len = 0;

  // Decrement the open counter and usage count
  // Without this, the module would not unload
  device_open_count--;
  module_put(THIS_MODULE);
  return 0;
}

// ----- Read-write functions -----

static ssize_t device_read(struct file *flip, char *buffer, size_t len, loff_t *offset) {
  int bytes_read = 0;
  for (int i = *offset; bytes_read < len && i < output_len; ++i) {
    put_user(output_text[i], buffer++);
    ++bytes_read;
  }
  
  *offset += bytes_read;
  return bytes_read;
}

static char* get_string_from_data(char** data, size_t* len) {
  if (*len <= 0) {
    return NULL;
  }
  int current_length = strlen(*data);
  if (current_length == 0) {
    return NULL;
  }

  char* value = kmalloc(current_length+1, GFP_KERNEL);
  if (value == NULL) {
    return NULL;
  }
  memcpy(value, *data, current_length+1);

  *data += current_length;
  *len -= current_length;
  if (*len > 0) {
    *data += 1;
    *len -= 1;
  }

  return value;
}

static int get_int_from_data(char** data, size_t* len) {
  if (*len == 0) {
    return -1;
  }
  int current_length = strlen(*data);
  if (current_length == 0) {
    return -1;
  }

  int value;
  int err = kstrtoint(*data, 10, &value);
  if (err != 0) {
    return -1;
  }

  *data += current_length;
  *len -= current_length;
  if (*len > 0) {
    *data += 1;
    *len -= 1;
  }

  return value;
}

int handle_user_from_data(char* data, size_t len) {
  int source_len = len;
  
  for (int i = 0; i < len; ++i) {
    if (data[i] == '\n' || data[i] == '\t' || data[i] == ' ') {
      data[i] = '\0';
    }
  }

  char* command = get_string_from_data(&data, &len);
  if (command == NULL) {
    return -EINVAL;
  }

  if (strcmp(command, "add") == 0) {
    struct phone_user_data* user = kmalloc(sizeof(struct phone_user_data), GFP_KERNEL);
    if (user == NULL) {
      kfree(command);
      return -EINVAL;
    }

    user->full_name = get_string_from_data(&data, &len);
    if (user->full_name == NULL) {
      kfree(user);
      kfree(command);
      return -EINVAL;
    }

    user->age = get_int_from_data(&data, &len);
    if (user->age == -1) {
      kfree(user->full_name);
      kfree(user);
      kfree(command);
      return -EINVAL;
    }

    user->telephone = get_string_from_data(&data, &len);
    if (user->telephone == NULL) {
      kfree(user->full_name);
      kfree(user);
      kfree(command);
      return -EINVAL;
    }

    user->email = get_string_from_data(&data, &len);
    if (user->email == NULL) {
      kfree(user->telephone);
      kfree(user->full_name);
      kfree(user);
      kfree(command);
      return -EINVAL;
    }

    kfree(command);
    phone_book_map_add(user);
  } else if (strcmp(command, "del") == 0) {
    char* name = get_string_from_data(&data, &len);
    if (name == NULL) {
      kfree(command);
      return -EINVAL;
    }

    kfree(command);
    phone_book_map_del(name);
    kfree(name);
  } else {
    kfree(command);
    return -EINVAL;
  }

  return source_len;
}

static ssize_t device_write(struct file *flip, const char *buffer, size_t len, loff_t *offset) {
  // "add Ivan_Ivanov 73 +79997779999 ahaha@mail.ru"
  // "del Ivan_Ivanov"

  char* data = kmalloc(len+1, GFP_KERNEL);
  if (data == NULL) {
    return -EIO;
  }  
  int err = copy_from_user(data, buffer, len);
  if (err != 0) {
    kfree(data);
    return -EIO;
  }
  *(data + len) = '\0';

  int result = handle_user_from_data(data, len);

  kfree(data);

  return result;  
}

// ----- IOctl functions -----

struct user_data_helper {
  char* full_name;
  int full_name_len;

  int age;

  char* telephone;
  int telephone_len;

  char* email;
  int email_len;
};

#define IOCTL_PHONE_BOOK_ADD _IOWR(248, 0, struct user_data_helper*)
#define IOCTL_PHONE_BOOK_FIND _IOWR(248, 1, struct user_data_helper*)
#define IOCTL_PHONE_BOOK_DEL _IOWR(248, 2, struct user_data_helper*)

static long device_ioctl(struct file *flip, unsigned int ioctl_num, unsigned long ioctl_param) {
  void* data_src = (void*) ioctl_param;
  struct user_data_helper* user_src = (struct user_data_helper*) data_src;

  struct user_data_helper* user = kmalloc(sizeof(struct user_data_helper), GFP_KERNEL);
  struct phone_user_data* user_obj;
  int err;

  err = copy_from_user(user, user_src, sizeof(struct user_data_helper));
  char* name = kmalloc(user->full_name_len+1, GFP_KERNEL);
  err = copy_from_user(name, user->full_name, user->full_name_len+1);

  switch (ioctl_num) {
  case IOCTL_PHONE_BOOK_ADD:
    char* telephone = kmalloc(user->telephone_len+1, GFP_KERNEL);
    err = copy_from_user(telephone, user->telephone, user->telephone_len+1);
    char* email = kmalloc(user->email_len+1, GFP_KERNEL);
    err = copy_from_user(email, user->email, user->email_len+1);

    user_obj = kmalloc(sizeof(struct phone_user_data), GFP_KERNEL);
    user_obj->full_name = name;
    user_obj->age = user->age;
    user_obj->telephone = telephone;
    user_obj->email = email;

    phone_book_map_add(user_obj);

    kfree(user);
    break;
  case IOCTL_PHONE_BOOK_FIND:
    user_obj = phone_book_map_find(name);
    if (user_obj == NULL) {
      kfree(name);
      kfree(user);
      return -ENOENT;
    }

    user->age = user_obj->age;
    err = copy_to_user(user->telephone, user_obj->telephone, strlen(user_obj->telephone)+1);
    err = copy_to_user(user->email, user_obj->email, strlen(user_obj->email)+1);
    err = copy_to_user(user_src, user, sizeof(struct user_data_helper));

    kfree(name);
    kfree(user);
    break;
  case IOCTL_PHONE_BOOK_DEL:
    phone_book_map_del(name);

    kfree(name);
    kfree(user);
    break;  
  default:
    kfree(name);
    kfree(user);
    return -EINVAL;
  }  

  return 0;
}

// ----- Init/exit functions -----

static int major_num;

static int __init mipt_phone_book_init(void) {
  phone_book_map_init();

  // Try to register character device
  major_num = register_chrdev(0, "phone_book", &file_ops);
  if (major_num < 0) {
    printk(KERN_ALERT "Could not register phone_book device: %d\n", major_num);
    return 1;
  } else {
    printk(KERN_INFO "phone_book module loaded with device major number %d\n", major_num);
    return 0;
  }
}

static void __exit mipt_phone_book_exit(void) {
  phone_book_map_free();

  // Unregister the character device
  unregister_chrdev(major_num, "phone_book");
  printk(KERN_INFO "Unregister phone_book device\n");
}

module_init(mipt_phone_book_init);
module_exit(mipt_phone_book_exit);