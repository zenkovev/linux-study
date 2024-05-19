#include <stdio.h>
#include <stdlib.h>

int main() {
  unsigned long variable = 0;
  unsigned long* ptr = malloc(sizeof(unsigned long));
  *ptr = 0;

  void* addr_stack = &variable;
  unsigned long addr_stack_val = (unsigned long)addr_stack;
  void* addr_heap = ptr;
  unsigned long addr_heap_val = (unsigned long)addr_heap;
  void* addr_wrong = NULL + sizeof(unsigned long);
  unsigned long addr_wrong_val = (unsigned long)addr_wrong;

  FILE* mmaneg = fopen("/proc/mmaneg", "w+");

  fprintf(mmaneg, "findpage %lx", addr_stack_val);
  fflush(mmaneg);
  fprintf(mmaneg, "findpage %lx", addr_heap_val);
  fflush(mmaneg);
  fprintf(mmaneg, "findpage %lx", addr_wrong_val);
  fflush(mmaneg);

  fprintf(mmaneg, "writeval %lx 100", addr_stack_val);
  fflush(mmaneg);
  fprintf(mmaneg, "writeval %lx 200", addr_heap_val);
  fflush(mmaneg);
  fprintf(mmaneg, "writeval %lx 300", addr_wrong_val);
  fflush(mmaneg);

  fclose(mmaneg);

  printf("Stack value: %lu\n", variable);
  printf("Heap value: %lu\n", *ptr);
  free(ptr);
}