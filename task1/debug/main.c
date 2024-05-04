// gcc main.c -o main && ./main
// gcc -fsanitize=address -g main.c -o main && ./main

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GFP_KERNEL 0
#define EINVAL 1

void* kmalloc(size_t size, int flags) {
    return malloc(size);
}

void kfree(void *objp) {
    free(objp);
}

struct hlist_node {};

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

static void phone_book_map_add(struct phone_user_data *data) {
  data->key = get_key_from_full_name(data->full_name);
  printf("<> phone_user_data <>\n");
  printf("Key: %d\n", data->key);
  printf("Full name: %s\n", data->full_name);
  printf("Age: %d\n", data->age);
  printf("Telephone: %s\n", data->telephone);
  printf("Email: %s\n", data->email);

  free(data->full_name);
  free(data->telephone);
  free(data->email);
  free(data);
}

static void phone_book_map_del(char* full_name) {
  printf("<> phone_delete <>\n");
  printf("Name: %s\n", full_name);
}

int kstrtoint(char* data, int radix, int* value) {
  *value = atoi(data);
  if (*value == 0) {
    return -1;
  }
  return 0;
}

// Code

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

// Code

void test(char* data) {
  size_t len = strlen(data);
  char* buffer = malloc(len+1);
  memcpy(buffer, data, len);
  *(buffer + len) = '\0';

  int code = handle_user_from_data(buffer, len);
  if (code != len) {
    printf("<> error <>\n");
  }

  free(buffer);
}

int main() {
    test("add Ivan_Ivanov 73 +79997779999 ahaha@mail.ru\n");
    test("add Ivan_Ivanov 73 +79997779999 ahaha@mail.ru\n");
    test("ahahah\n");
    test("addvk fhfgb 54664 fhkfkakj\n");
    test("del Ivan_Ivanov\n");
    test("adsfhjfewwh erwhkgrewhj\n");
    test("add Petr_Petrov 57 +79997779999 ura@mail.ru\n");
    test("fehjewhjrhhk\n");
    test("\n");
}