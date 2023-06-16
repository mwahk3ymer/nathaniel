 #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_INPUT_LENGTH 1024

char **get_paths()
{
    char *path_env = getenv("PATH");
    char *token;
    char **paths = malloc(sizeof(char *) * MAX_INPUT_LENGTH);
    int i = 0;

    if (!paths)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    token = strtok(path_env, ":");
    while (token != NULL)
    {
        paths[i] = token;
        token = strtok(NULL, ":");
        i++;
    }
    paths[i] = NULL;

    return paths;
}

void execute_command(char *command, char **paths)
{
    char *args[MAX_INPUT_LENGTH];
    int i = 0;

    args[i++] = command;

    while (paths[i - 1] != NULL)
    {
        args[0] = malloc(sizeof(char) * (strlen(paths[i - 1]) + strlen(command) + 2));
        if (!args[0])
        {
            perror("malloc");
            exit(EXIT_FAILURE);
        }

        sprintf(args[0], "%s/%s", paths[i - 1], command);
        execve(args[0], args, NULL);

        free(args[0]);
        i++;
    }

    perror("execve");
    exit(EXIT_FAILURE);
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
            char **paths = get_paths();
            execute_command(input, paths);
        }
        else
        {
            int status;
            waitpid(ret, &status, 0);

            if (!WIFEXITED(status))
            {
                printf("Child process terminated abnormally\n");
            }
        }
    }

    return 0;
}
