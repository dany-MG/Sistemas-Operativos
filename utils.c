#include "shell.h"

void Chdir(const char *path){
	if (!path)
	{
        path = getenv("HOME");
        if(!path){
            fprintf(stderr, RED"cd: path argument required\n"RST);
		    return ;
        }
	}
	if (chdir(path) == -1)
		perror(RED"cd FAILED!"RST);
}

char *Getcwd(char *buf, size_t size){
	char *result;

	result = getcwd(buf, size);
	if (!result)
		perror(RED"Getcwd FAILED!"RST);
	return (result);
}

void Getline(char **lineptr, size_t *n , FILE *stream){
    if(!lineptr || !stream){
        fprintf(stderr, RED"Getline: invalid arguments\n"RST);
        return;
    }

    ssize_t len = getline(lineptr, n, stream);
    if(len == -1){
        free(*lineptr);
        *lineptr = NULL;
        if(feof(stream))
            printf(RED"[EOF]"RST"\n");
        else
            perror(RED"Getline FAILED!"RST);
    }

    if (len > MAX_BUFF){
        fprintf(stderr, RED"Error: buffer size (200) exceeded\n"RST);
        return;
    }
}

void *Malloc(size_t buffer_size){
    void *ptr;
    if(buffer_size == 0)
        return (NULL);
    
    ptr = malloc(buffer_size);
    if(!ptr){
        perror(RED"Malloc FAILED!"RST);
        exit(EXIT_FAILURE);
    }
    return (ptr);
}

void *Realloc(void *ptr, size_t buffer_size){
    void *new_ptr;
    new_ptr = realloc(ptr, buffer_size);

    if(!new_ptr && buffer_size != 0){
        perror(RED"Realloc FAILED!"RST);
        exit(EXIT_FAILURE);
    }
    return (new_ptr);
}

pid_t Fork(){
    __pid_t pid;
    pid = fork();
    if(pid < 0){
        perror(RED"Fork FAILED!"RST);
        exit(EX_OSERR);
    }
    return (pid);
}

pid_t Wait(int *status){
    pid_t result;
    if(!status){
        fprintf(stderr, RED"Wait: status argument required\n"RST);
        return (-1);
    }
    result = wait(status);
    if(result == -1)
        perror(RED"Wait FAILED!"RST);
    if(WIFEXITED(*status))
        *status = WEXITSTATUS(*status);
    return (result);
}

void Execvp(const char *file, char *const argv[]){
    if(!file || !argv){
        fprintf(stderr, RED"Execvp: invalid arguments\n"RST);
        exit(EXIT_FAILURE);
    }
    if(execvp(file, argv) == -1){
        perror(RED"Child process FAILED!"RST);
        exit(EX_UNAVAILABLE);
    }
}

void handle_heredoc(int writeFd, const char* delimiter){
    char* line = NULL;
    size_t len = 0; 
    ssize_t read;

    if (!delimiter) {
        close(writeFd);
        return;
    }

    while(true){
        if (isatty(fileno(stdin))) {
            printf("heredoc >> ");
            fflush(stdout);
        }
        read = getline(&line, &len, stdin);
        if (read == -1){
            free(line);
            close(writeFd);
            return;
        }
        if (read > 0 && line[read - 1] == '\n') {
            line[read - 1] = '\0';
            read--;
        }
        //getting out from heredoc = delimiter found.
        if (strcmp(line, delimiter) == 0) {
            free(line);
            close(writeFd);
            return;
        }
        if (read > 0) {
            line[read] = '\n';
            read++;
        }

        if (write(writeFd, line, read) == -1) {
            perror(RED "Error: write to pipe failed" RST);
            break;
        }
    }
}

void printBanner(void){
    p(P"     ▄█  ▄█    ▄▄▄▄███▄▄▄▄   ▀█████████▄   ▄██████▄ \n" 
"    ███ ███  ▄██▀▀▀███▀▀▀██▄   ███    ███ ███    ███ \n"
"    ███ ███▌ ███   ███   ███   ███    ███ ███    ███ \n"
"    ███ ███▌ ███   ███   ███  ▄███▄▄▄██▀  ███    ███ \n"
"    ███ ███▌ ███   ███   ███ ▀▀███▀▀▀██▄  ███    ███ \n"
"    ███ ███  ███   ███   ███   ███    ██▄ ███    ███ \n"
"    ███ ███  ███   ███   ███   ███    ███ ███    ███ \n"
"█▄ ▄███ █▀    ▀█   ███   █▀  ▄█████████▀   ▀██████▀  \n"
"▀▀▀▀▀▀                                               \n"RST);
}


void spinnerLoad(){
    const char *charging[]={
        PINK"->🔥"RST,        
        PINK"-->🔥"RST,
        PINK"--->🔥"RST,
        PINK"---->🔥"RST,
        PINK"----->🔥"RST,
        PINK"------>🔥"RST,
        PINK"------->🔥"RST,
        PINK"-------->🔥"RST,
        PINK"--------->🔥"RST,
        PINK"---------->🔥"RST,
        PINK"----------->🔥"RST,
        PINK"------------>🔥"RST,
        PINK"------------->🔥"RST,
        PINK"--------------->🔥"RST
    };

    const int frames = sizeof(charging) / sizeof(charging[0]);
    p(PINK"\n[Shutting down...]\n"RST);

    for(int i = 0; i<frames; i++){
        p("\r" Y "%s" RST, charging[i]);
        fflush(stdout);
        usleep(421337); 
    }

    p(PINK"\n🃏 EXIT 🃏\n"RST);
    exit(EXIT_SUCCESS);
}

void handle_redirections(char** args, int std_in, int std_out, int std_err){
    int input = -1, output = -1;
    int heredoc[2] = {-1, -1};
    for(int i = 1; args[i] != NULL; i++){
        if(!strcmp(args[i], "<")){
            if(args[i+1] == NULL){
                fprintf(stderr, RED"Error: no input file specified\n"RST);
                dup2(std_in, 0);
                dup2(std_out, 1);
                dup2(std_err, 2);
                return;
            }
            input = open(args[i+1], O_RDONLY);
            if(input < 0 || dup2(input, 0) < 0){
                fprintf(stderr,RED"Error: can't open input file\n"RST);
                dup2(std_in, 0);
                dup2(std_out, 1);
                dup2(std_err, 2);
                return;
            }
            args[i] = NULL;
            args[i+1] = NULL;
        }
        else if(!strcmp(args[i], "<<")){
            if (!args[i+1]) {
                fprintf(stderr, RED "Error: delimiter missing\n" RST);
                return;
            }
            if(args[i+1] == NULL){
                fprintf(stderr, RED"Error: no delimiter specified for heredoc\n\n"RST);
                dup2(std_in, 0);
                dup2(std_out, 1);
                dup2(std_err, 2);
                return;
            }
            if(pipe(heredoc) == -1){
                fprintf(stderr,RED "Error: pipe failed\n" RST);
                dup2(std_in, 0);
                dup2(std_out, 1);
                dup2(std_err, 2);
                return;
            }
            handle_heredoc(heredoc[1], args[i+1]);
            input = open(args[i+1], O_RDONLY);
            if(dup2(heredoc[0], 0) == -1){
                fprintf(stderr,RED"Error: dup2 failed for heredoc\n"RST);
                dup2(std_in, 0);
                dup2(std_out, 1);
                dup2(std_err, 2);
                close(heredoc[0]);
                close(heredoc[1]);
                return;
            }
            args[i] = NULL;
            args[i+1] = NULL;
        }
        else if(!strcmp(args[i], ">") || !strcmp(args[i], ">>")){
            if(args[i+1] == NULL){
                fprintf(stderr, RED"Error: no output file specified\n"RST);
                dup2(std_in, 0);
                dup2(std_out, 1);
                dup2(std_err, 2);
                return;
            }
            output = open(args[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if(output < 0 || dup2(output, 1) < 0){
                fprint(stderr,RED"Error: can't open output file\n"RST);
                dup2(std_in, 0);
                dup2(std_out, 1);
                dup2(std_err, 2);
                return;
            }
            args[i] = NULL;
            args[i+1] = NULL;
        }
        i+=2;
    }

    if (input != -1) close(input);
    if (output != -1) close(output);
}
