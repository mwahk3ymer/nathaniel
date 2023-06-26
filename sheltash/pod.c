#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "shell.h"
#include "node.h"
#include "pod.h"
#include "parser.h"
#include "source.h"


char *search_path(char *file)
{
    char *PATH = getenv("PATH");
    char *p    = PATH;
    char *p2;
    
    while(p && *p)
    {
        p2 = p;

        while(*p2 && *p2 != ':')
        {
            p2++;
        }
        
	int  plen = p2-p;
        if(!plen)
        {
            plen = 1;
        }
        
        int  alen = strlen(file);
        char path[plen+1+alen+1];
        
	strncpy(path, p, p2-p);
        path[p2-p] = '\0';
        
	if(p2[-1] != '/')
        {
            strcat(path, "/");
        }

        strcat(path, file);
        
	struct stat st;
        if(stat(path, &st) == 0)
        {
            if(!S_ISREG(st.st_mode))
            {
                errno = ENOENT;
                p = p2;
                if(*p2 == ':')
                {
                    p++;
                }
                continue;
            }

            p = malloc(strlen(path)+1);
            if(!p)
            {
                return NULL;
            }
            
	    strcpy(p, path);
            return p;
        }
        else    /* file not found */
        {
            p = p2;
            if(*p2 == ':')
            {
                p++;
            }
        }
    }

    errno = ENOENT;
    return NULL;
}


int do_exec_cmd(int argc, char **argv)
{
    if(strchr(argv[0], '/'))
    {
        execv(argv[0], argv);
    }
    else
    {
        char *path = search_path(argv[0]);
        if(!path)
        {
            return 0;
        }
        execv(path, argv);
        free(path);
    }
    return 0;
}


static inline void free_argv(int argc, char **argv)
{
    if(!argc)
    {
        return;
    }

    while(argc--)
    {
        free(argv[argc]);
    }
}


int do_simple_command(struct node_s *node)
{
    if(!node)
    {
        return 0;
    }

    struct node_s *child = node->first_child;
    if(!child)
    {
        return 0;
    }
    
    int argc = 0;
    long max_args = 255;
    char *argv[max_args+1];     /* keep 1 for the terminating NULL arg */
    char *str;
    
    while(child)
    {
        str = child->val.str;
        argv[argc] = malloc(strlen(str)+1);
        
	if(!argv[argc])
        {
            free_argv(argc, argv);
            return 0;
        }
        
	strcpy(argv[argc], str);
        if(++argc >= max_args)
        {
            break;
        }
        child = child->next_sibling;
    }
    argv[argc] = NULL;

    pid_t child_pid = 0;
    if((child_pid = fork()) == 0)
    {
        do_exec_cmd(argc, argv);
        fprintf(stderr, "error: failed to execute command: %s\n", strerror(errno));
        if(errno == ENOEXEC)
        {
            exit(126);
        }
        else if(errno == ENOENT)
        {
            exit(127);
        }
        else
        {
            exit(EXIT_FAILURE);
        }
    }
    else if(child_pid < 0)
    {
        fprintf(stderr, "error: failed to fork command: %s\n", strerror(errno));
        return 0;
    }

    int status = 0;
    waitpid(child_pid, &status, 0);
    free_argv(argc, argv);
    
    return 1;
}



int main(int argc, char **argv)
{
    char *cmd;
    do
    {
        print_prompt1();
        cmd = read_cmd();
        if(!cmd)
        {
            exit(EXIT_SUCCESS);
        }
        if(cmd[0] == '\0' || strcmp(cmd, "\n") == 0)
        {
            free(cmd);
            continue;
        }
        if(strcmp(cmd, "exit\n") == 0)
        {
            free(cmd);
            break;
        }
	struct source_s src;
        src.buffer   = cmd;
        src.bufsize  = strlen(cmd);
        src.curpos   = INIT_SRC_POS;
        parse_and_execute(&src);
        free(cmd);
    } while(1);
    exit(EXIT_SUCCESS);
}


char *read_cmd(void)
{
    char buf[1024];
    char *ptr = NULL;
    char ptrlen = 0;
    while(fgets(buf, 1024, stdin))
    {
        int buflen = strlen(buf);
        if(!ptr)
        {
            ptr = malloc(buflen+1);
        }
        else
        {
            char *ptr2 = realloc(ptr, ptrlen+buflen+1);
            if(ptr2)
            {
                ptr = ptr2;
            }
            else
            {
                free(ptr);
                ptr = NULL;
            }
        }
        if(!ptr)
        {
            fprintf(stderr, "error: failed to alloc buffer: %s\n", strerror(errno));
            return NULL;
        }
        strcpy(ptr+ptrlen, buf);
        if(buf[buflen-1] == '\n')
        {
            if(buflen == 1 || buf[buflen-2] != '\\')
            {
                return ptr;
            }
            ptr[ptrlen+buflen-2] = '\0';
            buflen -= 2;
            print_prompt2();
        }
        ptrlen += buflen;
    }
    return ptr;
}


int parse_and_execute(struct source_s *src)
{
    skip_white_spaces(src);

    struct token_s *tok = tokenize(src);

    if(tok == &eof_token)
    {
        return 0;
    }

    while(tok && tok != &eof_token)
    {
        struct node_s *cmd = parse_simple_command(tok);

        if(!cmd)
        {
            break;
        }

        do_simple_command(cmd);
        free_node_tree(cmd);
        tok = tokenize(src);
    }
    return 1;
} 
