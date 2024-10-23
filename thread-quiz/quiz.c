#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include "common_threads.h"

void* func(void* args) {
  int n = (int)(long)args;
  for (int i = 0; i < 10; ++i) { 
    printf("%d %d\n", n, i);
  }
  return NULL;
}

int main(int argc, char *argv[]) {
  pthread_t run[3];
  printf("main: begin\n");
  for (int i = 0; i < 3; ++i) {
    Pthread_create(&run[i], NULL, func, (void*)(long)10);
  }
  for (int i = 0; i < 3; ++i) {
    Pthread_join(run[i], NULL);
  }
  printf("main: end\n");
  return 0;
}
