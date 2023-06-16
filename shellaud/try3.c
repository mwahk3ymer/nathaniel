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
    if ((*command)[0] == '\0')
    {
        /* End-of-file condition (Ctrl+D)*/
        exit(EXIT_SUCCESS);
    }
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

int execute_command(char **args)
{
	char *path = getenv("PATH");
	char *token = strtok(path, ":");
	char command_path[MAX_COMMAND_LENGTH];
	int command_found = 0;
    pid_t pid;

    if (strcmp(args[0], "exit") == 0)
    {
        exit(EXIT_SUCCESS);
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
        return 0;
    }

    /*char *path = getenv("PATH");
    char *token = strtok(path, ":");
    char command_path[MAX_COMMAND_LENGTH];
    int command_found = 0;*/

    while (token != NULL)
    {
        snprintf(command_path, sizeof(command_path), "%s/%s", token, args[0]);
        if (access(command_path, X_OK) == 0)
        {
            command_found = 1;
            break;
        }
        token = strtok(NULL, ":");
    }

    if (!command_found)
    {
        write(STDOUT_FILENO, "Command not found.\n", 19);
        return 0;
    }

    pid = fork();
    if (pid < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        if (execve(command_path, args, NULL) < 0)
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
        }

        free(command);
        command = NULL;
    }

    return 0;
}
