#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, const char* argv[]) {
  printf("before fork (pid:%d)\n", (int)getpid());
  int x = 100;
  int rc = fork();
  if (rc < 0) {
    fprintf(stderr, "Fork failed\n");
    exit(1);
  } else if (rc == 0) { // child
    printf("Child, x: %d\n", x);
    x = 200;
    printf("Child, now x: %d\n", x);
    printf("child (pid:%d)\n", (int)getpid());
  } else {
    printf("parent, x: %d\n", x);
    x = 200;
    printf("Parent, now x: %d\n", x);
    printf("parent of %d (pid:%d)\n\n", rc, (int)getpid());
  }
  return 0;
}
