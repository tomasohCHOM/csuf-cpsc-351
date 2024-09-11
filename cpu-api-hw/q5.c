#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, const char* argv[]) {
  printf("before fork (pid:%d)\n", (int)getpid());
  int rc = fork();
  if (rc < 0) {
    fprintf(stderr, "Fork failed\n");
    exit(1);
  } else if (rc == 0) { // child
    printf("child (pid:%d)\n", (int)getpid());
  } else { // parent
    printf("parent of %d (pid:%d)\n\n", rc, (int)getpid());
    int status;
    waitpid(rc, &status, 0);
    printf("%d", status);
  }
  return 0;
}

