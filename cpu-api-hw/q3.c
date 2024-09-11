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
    printf("child (pid:%d)\n", (int)getpid());
    
    char * const argv[] = { "ls", NULL };
    char * const envp[] = { NULL };

    /* Uncomment the one being used */

    execl("/bin/ls", "ls", NULL);
    // execlp("ls", "ls", NULL);
    // execle("/bin/ls", "ls", NULL, envp);
    // execv("/bin/ls", argv);
    // execvp("ls", argv);
    // execvpe("ls", argv, envp); // Implicit declaration is invalid in C99 warning

  } else { // parent
    printf("parent of %d (pid:%d)\n\n", rc, (int)getpid());
  }
  return 0;
}

