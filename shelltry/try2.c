#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_COMMAND_LENGTH 100
#define MAX_ARGS 10

void read_command(char **command)
{
	size_t bufsize = MAX_COMMAND_LENGTH;
    write(STDOUT_FILENO, "$ ", 2);
    getline(command, &bufsize, stdin);
    (*command)[strcspn(*command, "\n")] = '\0';
}

void parse_command(char *command, char **args)
{
    int i = 0;
    char *token = strtok(command, " ");
    while (token != NULL)
    {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
}

int check_command_path(char *command, char *path)
{
	char filepath[MAX_COMMAND_LENGTH];
	snprintf(filepath, sizeof(filepath), "%s/%s", path, command);
	if (access(filepath, F_OK) == 0)
		return (1);
	return (0);
}
char *get_command_path(char *command)
{
	char *path = getenv("PATH");
	char *dir = strtok(path, ":");
	while (dir != NULL)
	{
		if (check_command_path(command, dir))
			return (dir);
		dir = strtok(NULL, ":");
	}
	return (NULL);
}

int execute_command(char **args)
{
	pid_t pid;
	char *command_path = get_command_path(args[0]);
	char filepath[MAX_COMMAND_LENGTH];
	int status = 0;
	
	if (strcmp(args[0], "exit") == 0)
	{
		if (args[1] != NULL)
		{
			status = atoi(args[1]);
		}
		exit(status);
	}
	else if (strcmp(args[0], "env") == 0)
	{
		extern char **environ;
		char **env = environ;
		while (*env)
		{
			write(STDOUT_FILENO, *env, strlen(*env));
			write(STDOUT_FILENO, "\n", 1);
			env++;
		}
		return (0);
	}
	else if (strcmp(args[0], "cd") == 0)
	{
		char *new_dir = args[1];
		char cwd[MAX_COMMAND_LENGTH];
		
		if (new_dir == NULL || strcmp(new_dir, "-") == 0)
		{
			new_dir = getenv("HOME");
		}
		else if (strcmp(new_dir, "-") == 0)
		{
			new_dir = getenv("OLDPWD");
		}
		
		if (chdir(new_dir) == 0)
		{
			getcwd(cwd, sizeof(cwd));
			setenv("OLDPWD", getenv("PWD"), 1);
			setenv("PWD", cwd, 1);
		}
		else
		{
			perror("cd");
		}

		return (0);
	}

    /**handling path***/
    /**char *command_path = get_command_path(args[0]);**/
    if (command_path == NULL)
    {
	    write(STDOUT_FILENO, "command not found.\n", 19);
	    return (-1);
    }
    /**char filepath[MAX_COMMAND_LENGTH];**/
    snprintf(filepath, sizeof(filepath), "%s/%s", command_path, args[0]);
    if (access(filepath, X_OK) != 0)
    {
	    write(STDOUT_FILENO, "insufficient permissions. \n", 26);
	    return (-1);
    }

    pid = fork();
    if (pid < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        if (execve(args[0], args, NULL) < 0)
        {
            perror("execve");
            exit(EXIT_FAILURE);
        }
    }
    else
    {

        waitpid(pid, NULL, 0);
    }
return 0;
}

int main(void)
{
    char *command = NULL;
    char *args[MAX_ARGS];
    
    while (1)
    {
        read_command(&command);

        if (strcmp(command, "exit") == 0)
        {
		parse_command(command, args);
		execute_command(args);
		break;
        }
        else if (strcmp(command, "help") == 0)
        {
            write(STDOUT_FILENO, "Shell Help:\n", 13);
            write(STDOUT_FILENO, "  - Enter a command and press Enter to execute it.\n", 51);
            write(STDOUT_FILENO, "  - Enter 'exit' to exit the shell.\n", 36);
            write(STDOUT_FILENO, "  - Enter 'help' to display this help message.\n", 47);
        }
        else
        {
            parse_command(command, args);
            execute_command(args);
            
            /**char status_str[10];
            sprintf(status_str, "%d\n", status);
            write(STDOUT_FILENO, status_str, strlen(status_str));**/
        }
    }

    return 0;
}
