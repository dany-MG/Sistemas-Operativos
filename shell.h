#ifndef SHELL_H
#define SHELL_H
# include <unistd.h>
# include <stdbool.h>
# include <stdlib.h>
# include <stdio.h>
# include <errno.h>
# include <string.h>
# include <signal.h>
# include <sys/wait.h>
# include <sysexits.h>
#include <ctype.h>
#include <fcntl.h>		

#define MAX_BUFF 200


/*
    ** ANSI Color codes
    ** Y    -Yellow
    ** G    -Green
    ** C    -Cyan
    ** RED  -Red
    ** RST  -Reset def color
*/

#define Y   "\033[1;33m"
#define G   "\033[1;32m"
#define C   "\033[1;36m"
#define P   "\e[1;35m"
#define PINK "\033[1;38;5;218m"
#define RED   "\033[1;31m"
#define RST   "\033[0m"

#define p(...) printf(__VA_ARGS__)
#define DEL " \n\t\v\f\r"
#define e(E_S) exit(EXIT_SUCCESS)
#define ERROR(msg) fprintf(stderr, RED msg RST "\n")

typedef struct s_builtin
{
    const char *builtin_name;
    int (*foo)(char **av);
}t_builtin;

//Built-in commands

int shell_exit(char **args);
int shell_env(char **args);
int shell_echo(char **args);
int shell_cd(char **args);


void printBanner(void);
void spinnerLoad();

//Utility functions
void Chdir(const char *path);
void Getline(char **lineptr, size_t *n , FILE *stream);
char *Getcwd(char *buf, size_t size);
void *Malloc(size_t buffer_size);
void *Realloc(void *ptr, size_t buffer_size);
char* get_user();
pid_t Fork();
pid_t Wait(int *status);
void Execvp(const char *file, char *const argv[]);
void handle_heredoc(int writeFd, const char* delimiter);
void handle_redirections(char** args, int std_in, int std_out, int std_err);

//signals
void ctrlC(int signum);
void ctrlZ(int signum);
void init_shell_signals();


#endif