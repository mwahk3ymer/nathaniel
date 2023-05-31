#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main(void)
{
	char *delim = " \n";
	char *cmd = NULL, *cmd_cpy, *token = NULL;
	size_t n = 0;
	int argc = 0, i = 0;
	char **argv = NULL;

	printf("$ ");
	if (getline(&cmd, &n, stdin) == -1)
	return (-1);

	cmd_cpy = strdup(cmd);

	token = strtok(cmd, delim);

	while (token)
	{
		token = strtok(NULL, delim);
		argc++;
	}

	printf("%d\n", argc);

	argv = malloc(sizeof(char *) * argc);

	token = strtok(cmd_cpy, delim);

	while (token)
	{
		argv[i] = token;
		token = strtok(NULL, delim);
		i++;
	}
	argv[i] = NULL;

	i = 0;
	while (argv[i])
		printf("%s\n", argv[i++]);
	free(cmd), free(cmd_cpy), free(argv);

	return (0);
}
