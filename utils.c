#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>

#include "utils.h"

void*
mem_alloc(size_t size)
{
    void *ptr = malloc(size);
    if (!ptr)
    {
        perror("malloc");
        exit(1);
    }
    return ptr;
}

void
shift_str_left(char *s, size_t pos)
{
    for (size_t i = pos+1; i <= strlen(s); i++)
    {
        s[i-1] = s[i]; 
    }
}

void
trim(char *s)
{
    int trimed = 0;
    while (!trimed)
    {
        switch (s[0])
        {
        case '\n':
        case ' ':
            shift_str_left(s, 0);
            break;
        
        default:
            trimed = 1;
            break;
        }
    }
    trimed = 0;

    while (!trimed)
    {
        switch (s[strlen(s)-1])
        {
        case '\n':
        case ' ':
            shift_str_left(s, strlen(s)-1);
            break;
        
        default:
            trimed = 1;
            break;
        }
    }
}

size_t
read_until_char(int fd, 
                char *buffer, 
                size_t sizeofbuff, 
                char until)
{
    int nbReaded = -1, on_exit = 0;
    size_t cnt = 0;
    char c;
    while ((nbReaded = read(fd, &c, 1)) == 1)
    {

        if (c == until)
        {
            buffer[cnt] = '\0';
            on_exit = 1;
        }
        else
        {
            buffer[cnt] = c;
            cnt++;
        }

        if (cnt+1 == sizeofbuff && !on_exit)
        {
            // cannot read more
            buffer[cnt] = '\0';
            on_exit = 1;
        }
        if (on_exit)
            break;
    }

    return cnt;
}

void 
exec_steghide(const char *stegfile, 
                const char *pass, 
                char *buffer, 
                size_t sizeof_buffer)
{
    int tube[2];
    if (pipe(tube) < 0)
    {
        perror("pipe");
        exit(1);
    }

    pid_t id = fork();
    if (!id)
    {
        close(tube[0]);
        if (dup2(tube[1], STDERR_FILENO) < 0)
        {
            perror("dup2");
            exit(1);
        }
        execlp("steghide", "steghide", "extract", "-sf", stegfile, "-p", pass, "-xf", STEG_OUTFILE, "-f", NULL);
        perror("execlp");
        exit(1);
    }
    close(tube[1]);

    read_until_char(tube[0], buffer, sizeof_buffer, '\n');
    
    waitpid(id, NULL, 0);
    close(tube[0]);
}

char**
segment_string(const char *s, uint nb_segment)
{
    char **pointers = mem_alloc(sizeof *pointers * nb_segment);

    size_t len = strlen(s)/nb_segment;
    pointers[0] = (char*)s;
    for (size_t i = 1; i < nb_segment; i++)
    {
        pointers[i] = (char*)s+i*len;
        while (*pointers[i] != '\n') { pointers[i]++; }
        pointers[i]++;
    }
    
    return pointers;
}

bool isnumber(char *s)
{
    if (strlen(s) == 0)
        return false;
    while (*s != '\0')
    {
        if (!isdigit(*s))
            return false;
        s++;
    }
    return true;
}
