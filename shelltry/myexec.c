int execute_builtin_command(char **args) {
    if (strcmp(args[0], "exit") == 0) {
        int status = 0;
        if (args[1] != NULL) {
            status = atoi(args[1]);
        }
        exit(status);
    } else if (strcmp(args[0], "env") == 0) {
        extern char **environ;
        char **env = environ;
        while (*env) {
            write(STDOUT_FILENO, *env, strlen(*env));
            write(STDOUT_FILENO, "\n", 1);
            env++;
        }
        return 0;
    } else if (strcmp(args[0], "cd") == 0) {
        return execute_cd_command(args);
    }

    return -1;  // Not a builtin command
}

int execute_external_command(char **args) {
    char *command_path = get_command_path(args[0]);
    char filepath[MAX_COMMAND_LENGTH];

    if (command_path == NULL) {
        write(STDOUT_FILENO, "command not found.\n", 19);
        return -1;
    }

    snprintf(filepath, sizeof(filepath), "%s/%s", command_path, args[0]);
    if (access(filepath, X_OK) != 0) {
        write(STDOUT_FILENO, "insufficient permissions. \n", 26);
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        if (execve(args[0], args, NULL) < 0) {
            perror("execve");
            exit(EXIT_FAILURE);
        }
    } else {
        waitpid(pid, NULL, 0);
    }

    return 0;
}

int execute_cd_command(char **args) {
    char *new_dir = args[1];
    char cwd[MAX_COMMAND_LENGTH];

    if (new_dir == NULL || strcmp(new_dir, "~") == 0) {
        new_dir = getenv("HOME");
    } else if (strcmp(new_dir, "-") == 0) {
        new_dir = getenv("OLDPWD");
    }

    if (chdir(new_dir) == 0) {
        getcwd(cwd, sizeof(cwd));
        setenv("OLDPWD", getenv("PWD"), 1);
        setenv("PWD", cwd, 1);
    } else {
        perror("cd");
    }

    return 0;
}

int execute_command(char **args) {
    if (execute_builtin_command(args) != -1) {
        return 0;  // Builtin command executed successfully
    }

    return execute_external_command(args);
}
