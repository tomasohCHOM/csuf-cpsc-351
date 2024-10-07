#include "shell.h"
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

// Save previous arguments here
char *prev_args[SHELL_RL_BUFSIZE];

/*
 * List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
  "help",
  "cd",
  "mkdir",
  "exit",
  "!!"
};

int (*builtin_func[]) (char **) = {
  &shell_help,
  &shell_cd,
  &shell_mkdir,
  &shell_exit,
  &shell_exec_prev
};

int shell_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

int arr_len(char **arr) {
  int length = 0;
  while (arr[length] != NULL) {
    ++length;
  }
  return length;
}

void save_prev_args(char **src, char **dst) {
  int i = 0;
  while (src[i] != NULL) {
    dst[i] = strdup(src[i]);
    ++i;
  }
  // Remove trailing args from previous command
  while (dst[i] != NULL) {
    free(dst[i]);
    dst[i] = NULL;
    ++i;
  }
}

/**
   @brief Builtin command: print help.
   @param args List of args.  Not examined.
   @return Always returns 1, to continue executing.
 */
int shell_help(char **args) {
  int i;
  printf("CPSC 351 UNIX Shell, made by Tomas Oh and Max Rivas\n");
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
  @brief Builtin command: Recall previous command (!!)
  @param args List of args. args[0] is "!!"
  @return Always return 1, to continue executing
 */
int shell_exec_prev(char **args) {
  if (*prev_args == NULL) {
    fprintf(stderr, "shell: no previous command\n");
  } else {
    shell_execute(prev_args);
  }
  return 1;
}


/**
  @brief (1) Echo command (Prints SPACE between elements and PIPE for each '|')
  @param args List of args. Last element in args ends with ECHO
  @return Always return 1, to continue executing
 */
int shell_echo(char **args) {
  for (int j = 0; j < arr_len(args) - 1; j++) {
    if (strcmp(args[j], "|") != 0) {
      printf("%s\n", args[j]);
    } else {
      // (3) Print PIPE for each pipe character in the input
      printf("PIPE\n");
    }
    // (2) Print SPACE for each character in the input
    printf("SPACE\n");
  }
  return 1;
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

/**
   @brief Execute shell built-in or launch program.
   @param args Null terminated list of arguments.
   @return 1 if the shell should continue running, 0 if it should terminate
 */
int shell_execute(char **args) {
  int i;

  // Check for empty command
  if (args[0] == NULL) {
    return 1;
  }

  // Check for ECHO at the end of the input
  if (i > 0 && strcmp(args[arr_len(args) - 1], "ECHO") == 0) {
    return shell_echo(args);
  }

  // (4) Run built-in commands
  for (i = 0; i < shell_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  // After this point, we execute non-built in commands
  return shell_launch(args);
}

/**
  @brief Launch a program and wait for it to terminate.
  @param args Null terminated list of arguments (including program).
  @return Always returns 1, to continue execution.
 */
int shell_launch(char **args) {
  // Variables for pipes and redirection
  char *input_file = NULL;
  char *output_file = NULL;
  char **pipe_pos = args;
  int pipe_index = 0;

  // Check for any pipes in the input
  while (*pipe_pos != NULL && strcmp(*pipe_pos, "|") != 0) {
    pipe_pos++;
    pipe_index++;
  }

  // Check for redirect commands '>' and '<'
  for (int i = 0; args[i] != NULL; i++) {
    if (strcmp(args[i], ">") == 0) {
      output_file = args[i + 1];
      args[i] = NULL;  // Remove the redirection part from args
    } else if (strcmp(args[i], "<") == 0) {
      input_file = args[i + 1];
      args[i] = NULL;  // Remove the redirection part from args
    }
  }

  // Decide which execution path to take based on the existence of pipes or redirection
  if (*pipe_pos != NULL) {
    char *args1[SHELL_RL_BUFSIZE];
    for (int i = 0; i < pipe_index; i++) {
      args1[i] = args[i];
    }
    args1[pipe_index] = NULL;
    char **args2 = pipe_pos + 1;

    return execute_with_piping(args1, args2);
  } else if (input_file || output_file) {
    return execute_with_redirection(args, input_file, output_file);
  }

  // (5) Fork/exec command WITHOUT command line redirection
  pid_t pid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process: execute the command
    if (execvp(args[0], args) == -1) {
      perror("shell");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("fork");
    return 1;
  }

  // Parent process: wait for the child to finish
  do {
    waitpid(pid, &status, WUNTRACED);
  } while (!WIFEXITED(status) && !WIFSIGNALED(status));

  return 1;
}

// (6) Fork/exec command WITH command line redirection
int execute_with_redirection(char **args, char *input_file, char *output_file) {
  pid_t pid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process: handle redirection
    if (input_file) {
      int in_fd = open(input_file, O_RDONLY);
      if (in_fd < 0) {
        perror("shell");
        exit(EXIT_FAILURE);
      }
      dup2(in_fd, STDIN_FILENO);  // Redirect stdin to input file
      close(in_fd);
    }
    if (output_file) {
      int out_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if (out_fd < 0) {
        perror("shell");
        exit(EXIT_FAILURE);
      }
      dup2(out_fd, STDOUT_FILENO);  // Redirect stdout to output file
      close(out_fd);
    }

    if (execvp(args[0], args) == -1) {
      perror("shell");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    perror("fork");
    return 1;
  }

  // Parent process waits for the child
  do {
    waitpid(pid, &status, WUNTRACED);
  } while (!WIFEXITED(status) && !WIFSIGNALED(status));

  return 1;
}

// (7) Fork/exec two programs with pipes between them
int execute_with_piping(char **args1, char **args2) {
  int pipefd[2];  // Array to hold the pipe file descriptors
  pid_t pid1, pid2;

  // Create the pipe
  if (pipe(pipefd) == -1) {
    perror("pipe");
    return -1;  // Error creating pipe
  }

  // First child process
  if ((pid1 = fork()) == -1) {
    perror("fork");
    return -1;  // Error forking
  }

  if (pid1 == 0) {  // Child process for the first command
    // Redirect stdout to the write end of the pipe
    dup2(pipefd[1], STDOUT_FILENO);
    close(pipefd[0]);  // Close read end of the pipe
    close(pipefd[1]);  // Close write end of the pipe (not needed in child)

    // Execute the first command
    if (execvp(args1[0], args1) == -1) {
      perror("execvp");
      exit(EXIT_FAILURE);  // Exit child process on failure
    }
  }

  // Second child process
  if ((pid2 = fork()) == -1) {
    perror("fork");
    return -1;  // Error forking
  }

  if (pid2 == 0) {  // Child process for the second command
    // Redirect stdin to the read end of the pipe
    dup2(pipefd[0], STDIN_FILENO);
    close(pipefd[1]);  // Close write end of the pipe
    close(pipefd[0]);  // Close read end of the pipe (not needed in child)

    // Execute the second command
    if (execvp(args2[0], args2) == -1) {
      perror("execvp");
      exit(EXIT_FAILURE);  // Exit child process on failure
    }
  }

  // Parent process
  close(pipefd[0]);  // Close read end of the pipe in parent
  close(pipefd[1]);  // Close write end of the pipe in parent

  // Wait for both child processes to finish
  waitpid(pid1, NULL, 0);
  waitpid(pid2, NULL, 0);

  return 1;  // Success
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

    save_prev_args(args, prev_args);

    free(line);
    free(args);
  } while (status);
}

int main(int argc, char **argv) {
  // Run command loop.
  shell_loop();

  return EXIT_SUCCESS;
}

