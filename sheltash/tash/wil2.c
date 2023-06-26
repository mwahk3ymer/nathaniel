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

/*****************************Private Function declaration******************************/
char *getcwd(char *buf, size_t size);
void DisplayPrompt();
char **parseInput(char *input, int *cmdLength);
void freeCommand(char **command);

/****************************/
static int numOfCmd;
static int cmdLength;
/****************************/

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
            write(STDOUT_FILENO, "Error reading input.\n", strlen("Error reading input.\n"));
            continue;
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
                continue;
            }

            DisplayPrompt();
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
                execvp(command[RESET], command);
                freeCommand(command);
                perror("Command not found");
                exit(EXIT_FAILURE);
            }
            else
            {
                wait(&id);
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
    int i = RESET, counter = RESET;
    char inputCopy[INPUT_SIZE];
    strcpy(inputCopy, input);

    char *ptr = strtok(input, CUTTING_WORD);
    while (ptr != NULL)
    {
        ptr = strtok(NULL, CUTTING_WORD);
        counter++;
    }

    char **command = malloc((counter + 1) * sizeof(char *));
    if (command == NULL)
    {
        write(STDOUT_FILENO, "Error allocating memory.\n", strlen("Error allocating memory.\n"));
        exit(EXIT_FAILURE);
    }

    char *ptrCopy = strtok(inputCopy, CUTTING_WORD);
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
    char *buf;
    char *ptr;

    size_t size = pathconf(".", _PC_PATH_MAX);
    if (size == (size_t)-1)
    {
        perror("Error getting pathconf");
        return;
    }

    buf = malloc(size);
    if (buf == NULL)
    {
        perror("Error allocating memory");
        return;
    }

    ptr = getcwd(buf, size);
    if (ptr == NULL)
    {
        perror("Error getting current directory");
        free(buf);
        return;
    }

    struct passwd *p = getpwuid(getuid());
    if (p == NULL)
    {
        perror("Error getting username");
        free(buf);
        return;
    }

    char prompt[256];
    sprintf(prompt, "%s@%s>", p->pw_name, ptr);
    write(STDOUT_FILENO, prompt, strlen(prompt));

    free(buf);
}
