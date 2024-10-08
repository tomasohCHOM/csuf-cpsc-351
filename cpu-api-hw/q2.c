#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, const char* argv[]) {
  printf("before fork (pid:%d)\n", (int)getpid());
  int rc = fork();
  if (rc < 0) {
    fprintf(stderr, "Fork failed\n");
    exit(1);
  } else if (rc == 0) { // child
    printf("hello\n");
    printf("child (pid:%d)\n", (int)getpid());
  } else { // parent
    sleep(1);
    printf("goodbye\n");
    printf("parent of %d (pid:%d)\n\n", rc, (int)getpid());
  }
  return 0;
}
