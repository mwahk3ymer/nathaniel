#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <errno.h>

#define INPUT_SIZE 510
#define CUTTING_WORD " \n"
#define ENDING_WORD "done"
#define RESET 0

void DisplayPrompt();
char **parseInput(char *input, int *cmdLength);
void freeCommand(char **command);

extern char **environ;

void printEnvironment()
{
    int i = 0;
    while (environ[i] != NULL)
    {
        write(STDOUT_FILENO, environ[i], strlen(environ[i]));
        write(STDOUT_FILENO, "\n", 1);
        i++;
    }
}

int main()
{
    int numOfCmd = RESET;
    int cmdLength = RESET;

    char input[INPUT_SIZE] = "";
    pid_t id;
    char **command;

    DisplayPrompt();

    while (strcmp(input, ENDING_WORD) != RESET)
    {
        if (fgets(input, INPUT_SIZE, stdin) == NULL)
        {
            write(STDOUT_FILENO, "EOF encountered. Exiting..\n", strlen("EOF encountered. Exiting..\n"));
            break;
        }

        command = parseInput(input, &cmdLength);

        if (strcmp("cd", command[RESET]) == RESET)
        {
            struct passwd *pwd;
            char *path = command[1];

            if (path == NULL)
            {
                pwd = getpwuid(getuid());
                path = pwd->pw_dir;
            }
            if (path[0] == '/')
                path++;

            if (chdir(path) == -1)
            {
                perror("Error changing directory");
                freeCommand(command);
		DisplayPrompt();
                continue;
            }

            DisplayPrompt();
        }
        else if (strcmp("exit", command[RESET]) == RESET)
        {
            int exitStatus = 0;
            if (cmdLength > 1)
            {
                exitStatus = atoi(command[1]);
            }
            freeCommand(command);
            exit(exitStatus);
        }
        else if (strcmp("env", command[RESET]) == RESET)
        {
            printEnvironment();
            freeCommand(command);
            DisplayPrompt();
            continue;
        }
        else
        {
            id = fork();
            if (id < RESET)
            {
                write(STDOUT_FILENO, "Fork failed.\n", strlen("Fork failed.\n"));
                freeCommand(command);
                continue;
            }
            else if (id == RESET)
            {
                numOfCmd++;
                execve(command[RESET], command, environ);
                freeCommand(command);
                perror("Command not found");
                exit(EXIT_FAILURE);
            }
            else
            {
                wait(NULL);
                if (strcmp(input, ENDING_WORD) != RESET)
                    DisplayPrompt();
                else
                {
                    char output[50];
                    sprintf(output, "Num of cmd: %d\n", numOfCmd);
                    write(STDOUT_FILENO, output, strlen(output));
                    sprintf(output, "cmd length: %d\n", cmdLength - 4);
                    write(STDOUT_FILENO, output, strlen(output));
                    write(STDOUT_FILENO, "Bye!\n", strlen("Bye!\n"));
                }
            }
        }

        freeCommand(command);
    }

    return EXIT_SUCCESS;
}

void freeCommand(char **command)
{
    int i;
    for (i = 0; command[i] != NULL; i++)
        free(command[i]);
    free(command);
}

char **parseInput(char *input, int *cmdLength)
{
int i = RESET;
    int counter = RESET;
    char *ptr;
    char *ptrCopy;
    char **command;

    char inputCopy[INPUT_SIZE];
    strcpy(inputCopy, input);

    ptr = strtok(input, CUTTING_WORD);
    while (ptr != NULL)
    {
        ptr = strtok(NULL, CUTTING_WORD);
        counter++;
    }

    command = malloc((counter + 1) * sizeof(char *));
    if (command == NULL)
    {
        write(STDOUT_FILENO, "Error allocating memory.\n", strlen("Error allocating memory.\n"));
        exit(EXIT_FAILURE);
    }

    ptrCopy = strtok(inputCopy, CUTTING_WORD);
    while (ptrCopy != NULL)
    {
        if (i == RESET)
            *cmdLength += strlen(ptrCopy);

        command[i] = malloc((strlen(ptrCopy) + 1) * sizeof(char));
        if (command[i] == NULL)
        {
            write(STDOUT_FILENO, "Error allocating memory.\n", strlen("Error allocating memory.\n"));
            freeCommand(command);
            exit(EXIT_FAILURE);
        }

        strcpy(command[i], ptrCopy);
        command[i][strlen(ptrCopy)] = '\0';
        ptrCopy = strtok(NULL, CUTTING_WORD);
        i++;
    }

    command[counter] = NULL;
    return command;	
}

void DisplayPrompt()
{
    write(STDOUT_FILENO, "$ ", 2);
}
