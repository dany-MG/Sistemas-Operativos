/*
    Sistemas Opertivos 4CM1
    Proyecto: Mini Shell
    Equipo: 
           -Arroyo Garcia Jairo Andrey
           -Manrique Godinez Daniel Alejandro
           -Resendiz Garcia Renata

    Fecha de entrega: 17-Junio-2025
*/


#include "shell.h"

/*
    builtins? -> call them 
    else -> fork, execvp, wait
*/

int status = 0;

t_builtin g_builtin[]={
    {.builtin_name = "echo", .foo=shell_echo},
    {.builtin_name = "env", .foo=shell_env},
    {.builtin_name = "exit", .foo=shell_exit},
    {.builtin_name = "cd", .foo=shell_cd},
    {.builtin_name = "NULL"}
};

void shell_launch(char **args){
    int pipes[MAX_BUFF][2];
    int num_cmds = 0;
    char** curr_command = args;
    pid_t pid;
    int std_in = dup(STDIN_FILENO);
    int std_out = dup(STDOUT_FILENO);

    // counting commands and replacing pipes with NULL
    for(int i = 0; args[i] != NULL; i++){
        if(!strcmp(args[i], "|")){
            num_cmds++;
            args[i] = NULL;
        }
    }
    num_cmds++;

    // Creting pipes
    for(int i = 0; i < num_cmds - 1; i++){
        if(pipe(pipes[i]) == -1){
            perror(RED"Error: pipe failed!"RST);
            return;
        }
    }

    for(int i = 0; i < num_cmds; i++){
        pid = Fork();
        if(pid == 0){
            // input redirection
            if(i > 0) {
                dup2(pipes[i-1][0], STDIN_FILENO);
            }

            // output redirection
            if(i < num_cmds-1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            } else {
                handle_redirections(curr_command, std_in, std_out, STDERR_FILENO);
            }

            // closing unused pipes
            for(int j = 0; j < num_cmds-1; j++){
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            Execvp(curr_command[0], curr_command);
            exit(status);
        }
        while(*args != NULL) args++;
        curr_command = ++args;
    }

    for(int i = 0; i < num_cmds-1; i++){
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    for(int i = 0; i < num_cmds; i++){
        Wait(&status);
    }

    // restore original file descriptors
    dup2(std_in, STDIN_FILENO);
    dup2(std_out, STDOUT_FILENO);
    close(std_in);
    close(std_out);
}

void shell_exec(char **args){
    const char *curr_builtin;
    int std_in = dup(0), std_out = dup(1), std_err = dup(2);

    if(!args || !args[0]) {
        dup2(std_in, 0);
        dup2(std_out, 1);
        dup2(std_err, 2);
        return;
    }

    // Handle redirections
    handle_redirections(args, std_in, std_out, std_err);

    // Execute command
    int i=0;
    while ((curr_builtin = g_builtin[i].builtin_name)) {
        if (!strcmp(args[0], curr_builtin)) {
            status = g_builtin[i].foo(args);
            if (status) p(RED"%s FAILED!\n"RST, curr_builtin);
            break;
        }
        i++;
    }

    if (!curr_builtin) {
        shell_launch(args);
    }

    // Restore file descriptors
    dup2(std_in, 0);
    dup2(std_out, 1);
    dup2(std_err, 2);
    close(std_in);
    close(std_out);
    close(std_err);
}

char* shell_read_line(void){
    char *line;
    size_t buffer_size;
    char cwd[BUFSIZ];
    char* user;

    line = NULL;
    buffer_size = 0;

    user = get_user();
    if(isatty(fileno(stdin))){
        if(status)
            p(PINK"┌──♦️(%s)"P"--[%s]"Y"[%d]🃏"PINK"\n└─>"RST, user, Getcwd(cwd, BUFSIZ), status);
        else
            p(PINK"┌──♦️(%s)"P"-[%s]🃏"PINK"\n└─>"RST, user, Getcwd(cwd, BUFSIZ));
    }
    Getline(&line, &buffer_size, stdin);
    return (line);
}

char **shell_split_line(char *line) {
    char **tokens = Malloc(BUFSIZ * sizeof(char*));
    int pos = 0;
    char *token;
    char *saveptr;

    // spliting basic delimiters
    token = strtok_r(line, DEL, &saveptr);
    while (token != NULL) {
        if (strcmp(token, "<") == 0 || strcmp(token, ">") == 0 || 
            strcmp(token, "<<") == 0 || strcmp(token, ">>") == 0 ||
            strcmp(token, "|") == 0) {
            tokens[pos++] = token;
        } else {
            char *aux_token = token;
            while (*aux_token) {
                if (*aux_token == '>' || *aux_token  == '<' || *aux_token == '|') {
                    // spliting tokens
                    if (aux_token != token) {
                        char *prev_cmd = strndup(token, aux_token - token);
                        tokens[pos++] = prev_cmd;
                    }
                    char op[3];
                    // operators (>>, <<)
                    if ((*aux_token == '>' && *(aux_token+1) == '>') || 
                        (*aux_token == '<' && *(aux_token+1) == '<')) {
                        op[1] = *aux_token;
                        op[2] = '\0';
                        aux_token++;
                    }
                    tokens[pos++] = strdup(op);
                    token = aux_token + 1;
                    aux_token = token - 1;
                }
                aux_token++;
            }
            if (*token) {
                tokens[pos++] = token;
            }
        }
        token = strtok_r(NULL, DEL, &saveptr);
    }
    tokens[pos] = NULL;
    return tokens;
}

int main(){
    //signals
    signal(SIGINT, ctrlC);
    printBanner();

    char *line;
    char **args;

    //REPL
    //READ -> EVALUATE -> PRINT -> LOOP

    //1) get line
    while((line = shell_read_line())){
        //2) obtener token
        args = shell_split_line(line);

        //3) Exec
        shell_exec(args);

        //4) free
        free(line);
        free(args);
    }

    return (EXIT_SUCCESS);
}