#define SHELL_RL_BUFSIZE 1024
#define SHELL_TOK_BUFSIZE 64
#define SHELL_TOK_DELIM " \t\r\n\a"

// Shell Function declarations
void shell_loop();
char *shell_read_line(void);
char **shell_split_line(char *line);
int shell_execute(char **args);
int shell_launch(char **args);

// Custom ECHO command
int shell_echo(char **args);

int shell_num_builtins();

// Declarations for builtin shell commands
int shell_help(char **args);
int shell_cd(char **args);
int shell_mkdir(char **args);
int shell_exit(char **args);
int shell_exec_prev(char **args);

// Declarations for executing commands with redirect and piping
int execute_with_redirection(char **args, char *input_file, char *output_file);
int execute_with_piping(char **args1, char **args2);

// Get the length of the given array
int arr_len(char **arr);
// Save arguments from the previous command
void save_prev_args(char **src, char **dst);
