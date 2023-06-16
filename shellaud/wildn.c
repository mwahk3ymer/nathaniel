#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_INPUT_LENGTH 1024

int main(void)
{
    char *prompt = " $ ";
    char input[MAX_INPUT_LENGTH];
    char *token = strtok(input, " ");
    ssize_t nchars_read;
    int ret;
    int i = 0;

    while (1)
    {
	    write(STDOUT_FILENO, prompt, strlen(prompt));

        nchars_read = read(STDIN_FILENO, input, MAX_INPUT_LENGTH);
        if (nchars_read == -1)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }
        else if (nchars_read == 0)
        {
            write(STDOUT_FILENO, "\n", 1);
            break;
        }

        input[nchars_read - 1] = '\0';

        ret = fork();
        if (ret == -1)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (ret == 0)
        {
            /* Tokenize input into arguments */
            char **args = malloc(sizeof(char *) * (MAX_INPUT_LENGTH / 2 + 2));
            if (args == NULL)
            {
                perror("malloc");
                exit(EXIT_FAILURE);
            }

            /*char *token = strtok(input, " ");
            int i = 0;*/
            while (token != NULL)
            {
                args[i] = token;
                token = strtok(NULL, " ");
                i++;
            }
            args[i] = NULL;

            if (execve(args[0], args, NULL) == -1)
            {
                perror("404");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            wait(NULL);
        }
    }

    return 0;
}
