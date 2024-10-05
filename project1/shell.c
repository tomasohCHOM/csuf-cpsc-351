#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define SHELL_RL_BUFSIZE 1024
#define SHELL_TOK_BUFSIZE 64
#define SHELL_TOK_DELIM " \t\r\n\a"

char *prev_args[SHELL_RL_BUFSIZE];

/*
  Function Declarations for builtin shell commands:
 */
int shell_help(char **args);
int shell_cd(char **args);
int shell_mkdir(char **args);
int shell_exit(char **args);
// int shell_exec_prev(char **args);

/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
  "help",
  "cd",
  "mkdir",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &shell_help,
  &shell_cd,
  &shell_mkdir,
  &shell_exit,
};

int shell_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

// Get the length of the array
int arr_len(char **arr) {
  int length = 0;
  while (arr[length] != NULL) {
    ++length;
  }
  return length;
}

// Save previous arguments
void save_prev_args(char **src, char **dst) {
  int i = 0;
  while (src[i] != NULL) {
    dst[i] = strdup(src[i]);
    ++i;
  }
}

/*
  Builtin function implementations.
*/
/**
   @brief Builtin command: print help.
   @param args List of args.  Not examined.
   @return Always returns 1, to continue executing.
 */
int shell_help(char **args) {
  int i;
  printf("CPSC 351 Project 1 Shell\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < shell_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

/**
   @brief Builtin command: change directory.
   @param args List of args.  args[0] is "cd".  args[1] is the directory.
   @return Always returns 1, to continue executing.
 */
int shell_cd(char **args) {
  if (args[1] == NULL) {
    fprintf(stderr, "shell: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("shell");
    }
  }
  return 1;
}

/**
  @brief Builtin command: Recall previous command (!!)
  @param args List of args. args[0] is "!!"
  @return Always return 1, to continue executing
 */
int shell_exec_prev(char **args) {
  return 1;
}

/**
 * @brief Builtin command: mkdir.
 * @param args List of args. args[0] is "mkdir". args[1] is the directory name.
 * @return Always returns 1, to continue executing.
 */
int shell_mkdir(char **args) {
  if (args[1] == NULL) {
    fprintf(stderr, "shell: expected argument to \"mkdir\"\n");
  } else {
    if (mkdir(args[1], 0700) != 0) {
      perror("shell");
    }
  }
  return 1;
}

/**
   @brief Builtin command: exit.
   @param args List of args.  Not examined.
   @return Always returns 0, to terminate execution.
 */
int shell_exit(char **args) {
  return 0;
}

/**
  @brief Launch a program and wait for it to terminate.
  @param args Null terminated list of arguments (including program).
  @return Always returns 1, to continue execution.
 */
int shell_launch(char **args) {
  pid_t pid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("shell");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("shell");
  } else {
    // Parent process
    do {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

// uses most code from the ch5 hw for piping between two files
void pipe_func(char **args) {
  int pipearr[2];
  pid_t pid1, pid2;
  if (pipe(pipearr) < 0) {fprintf(stderr, "Pipe process failed");}
  pid1 = fork();
  if (pid1 == 0) {
    dup2(pipearr[1], STDOUT_FILENO);
    close(pipearr[0]);
    close(pipearr[1]);
  }

  pid2 = fork();
  if (pid2 == 0) {
    dup2(pipearr[0], STDIN_FILENO);
  }
}

/**
   @brief Execute shell built-in or launch program.
   @param args Null terminated list of arguments.
   @return 1 if the shell should continue running, 0 if it should terminate
 */
int shell_execute(char **args) {
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  // if (strcmp(args[0], "!!") == 0) {
  //  if (*prev_args == NULL) {
  //    fprintf(stderr, "there is no previous command");
  //  } else {
  //    shell_execute(prev_args); // can add previous command definitions in other func
  //    return 1;
  //  }
  // }

  if (i > 0 && strcmp(args[arr_len(args) - 1], "ECHO") == 0) {
    for (int j = 0; j < i - 1; j++) {
      if (strcmp(args[j], "|") != 0) {
        printf("%s\n", args[j]);
      } else {
        printf("PIPE\n");
      }
      printf("SPACE\n");
    }
    return 1;
  }

  // Checks for pipe symbol, maybe runs another function to see a | b
  // int j = 0;
  // while (args[j] != NULL) {
  //   if (strcmp(args[i], "|") == 0) {
  //     pipe_func(args);
  //   }
  // }

  for (i = 0; i < shell_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  // save_prev_args(args, prev_args);
  return shell_launch(args);
}

/**
   @brief Read a line of input from stdin.
   @return The line from stdin.
 */
char *shell_read_line(void) {
  int bufsize = SHELL_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "shell: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Read a character
    c = getchar();

    if (c == EOF) {
      exit(EXIT_SUCCESS);
    } else if (c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    // If we have exceeded the buffer, reallocate.
    if (position >= bufsize) {
      bufsize += SHELL_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "shell: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

/**
   @brief Split a line into tokens (very naively).
   @param line The line.
   @return Null-terminated array of tokens.
 */
char **shell_split_line(char *line) {
  int bufsize = SHELL_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token, **tokens_backup;

  if (!tokens) {
    fprintf(stderr, "shell: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, SHELL_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += SHELL_TOK_BUFSIZE;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        free(tokens_backup);
        fprintf(stderr, "shell: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, SHELL_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

void shell_loop(void) {
  char *line;
  char **args;
  int status;

  do {
    printf("> ");
    line = shell_read_line();
    args = shell_split_line(line);
    status = shell_execute(args);

    free(line);
    free(args);
  } while (status);
}

int main(int argc, char **argv) {
  // Run command loop.
  shell_loop();

  return EXIT_SUCCESS;
}
