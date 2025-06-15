#include "shell.h"

int shell_exit(char **args){
    (void)args;
    spinnerLoad();
    e(E_S);
}

int shell_env(char **args){
    extern char **environ;
    (void)args;
    if(!environ){
        return (1);
    }

    for(int i = 0; environ[i]; i++)
        p("%s\n", environ[i]);
    return (0);
}

int	shell_echo(char **args){
	int start = 1;
	bool newline = true;

	if (!args || !args[0])
		return (1);

	// Check for the -n option
	if (args[1] && !strcmp(args[1], "-n"))
	{
		newline = false;
		start = 2;
	}

	// Print each argument separated by a space
	for (int i = start; args[i]; i++)
	{
		p("%s", args[i]);
		if (args[i + 1])
			p(" ");
	}

	// Print newline if -n option is not present
	if (newline)
		p("\n");

	return (0);
}


int shell_cd(char **args){
    Chdir(args[1]);
    return (0); 
}

/*
    abbr
    and
    argparse
    begin
    bg
    bind
    block
    break
    breakpoint
    builtin
    case
    cd (implemented on Chdir())
    command
    commandline
    complete
    contains
    continue
    count 
    disown
    else
    emit
    end
    eval
    exec
    false
    fg
    for
    function
    functions
    history
    if
    jobs R
    math
    not
    or
    path
    printf
    pwd
    random
    read
    realpath
    return
    set
    set_color
    source
    status
    string
    switch
    test
    time 
    true
    type
    unlimit
    wait
    while
*/