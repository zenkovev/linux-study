#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <asm/errno.h>
#include <sys/ioctl.h>

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

struct user_data {
  int age;
  char* telephone;
  char* email;
};

long add_user(char* name, struct user_data* input_data) {
  int fd = open("/dev/phone_book", O_RDWR);
  if (fd < 0) {
    return -EACCES;
  }
  
  struct user_data_helper helper = {
    .full_name = name,
    .full_name_len = strlen(name),
    .age = input_data->age,
    .telephone = input_data->telephone,
    .telephone_len = strlen(input_data->telephone),
    .email = input_data->email,
    .email_len = strlen(input_data->email)
  };

  int err = ioctl(fd, IOCTL_PHONE_BOOK_ADD, &helper);
  if (err < 0) {
    return err;
  }

  close(fd);
  return 0;
}

long get_user(char* name, struct user_data** output_data) {
  int fd = open("/dev/phone_book", O_RDWR);
  if (fd < 0) {
    return -EACCES;
  }

  *output_data = malloc(sizeof(struct user_data));
  (*output_data)->telephone = malloc(100);
  (*output_data)->email = malloc(100);

  struct user_data_helper helper = {
    .full_name = name,
    .full_name_len = strlen(name),
    .telephone = (*output_data)->telephone,
    .telephone_len = 100,
    .email = (*output_data)->email,
    .email_len = 100
  };

  int err = ioctl(fd, IOCTL_PHONE_BOOK_FIND, &helper);
  if (err < 0) {
    free((*output_data)->email);
    free((*output_data)->telephone);
    free(*output_data);
    return err;
  }

  (*output_data)->age = helper.age;

  close(fd);
  return 0;
}

long del_user(char* name) {
  int fd = open("/dev/phone_book", O_RDWR);
  if (fd < 0) {
    return -EACCES;
  }
  
  struct user_data_helper helper = {
    .full_name = name,
    .full_name_len = strlen(name),
  };

  int err = ioctl(fd, IOCTL_PHONE_BOOK_DEL, &helper);
  if (err < 0) {
    return err;
  }

  close(fd);
  return 0;
}

int main() {
  struct user_data user = {
    .age = 87,
    .telephone = "+79000091835",
    .email = "revizor@mail.ru"
  };
  int err = add_user("Nikolay_Gogol", &user);
  if (err < 0) {
    return err;
  }

  struct user_data* super_user;
  err = get_user("Kozel_Vasilyev", &super_user);
  if (err < 0) {
    return err;
  }
  printf("Name: Kozel_Vasilyev\n");
  printf("Age: %d\n", super_user->age);
  printf("Telephone: %s\n", super_user->telephone);
  printf("Email: %s\n", super_user->email);
  free(super_user->email);
  free(super_user->telephone);
  free(super_user);

  err = del_user("Ivan_Ivanov");
  if (err < 0) {
    return err;
  }

  printf("There are no errors!\n");
}