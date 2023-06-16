#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_INPUT_LENGTH 1024

void execute_command(char *command)
{
    char *token;
    char *args[MAX_INPUT_LENGTH];
    int i = 0;

    token = strtok(command, " ");
    while (token != NULL)
    {
        args[i] = token;
        token = strtok(NULL, " ");
        i++;
    }
    args[i] = NULL;

    if (execve(args[0], args, NULL) == -1)
    {
        perror("execve");
        exit(EXIT_FAILURE);
    }
}

int read_command(char *input)
{
    ssize_t nchars_read;

    nchars_read = read(STDIN_FILENO, input, MAX_INPUT_LENGTH);
    if (nchars_read == -1)
    {
        perror("read");
        exit(EXIT_FAILURE);
    }
    else if (nchars_read == 0)
    {
        write(STDOUT_FILENO, "\n", 1);
        return 0;
    }

    input[nchars_read - 1] = '\0';
    return 1;
}

int main(void)
{
    char *prompt = " $ ";
    char input[MAX_INPUT_LENGTH];
    int ret;

    while (1)
    {
        write(STDOUT_FILENO, prompt, strlen(prompt));

        if (!read_command(input))
            break;

        ret = fork();
        if (ret == -1)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (ret == 0)
        {
            execute_command(input);
        }
        else
        {
            wait(NULL);
        }
    }

    return 0;
}
