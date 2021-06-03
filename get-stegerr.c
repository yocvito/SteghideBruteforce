#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>

#include "get-stegerr.h"
#include "utils.h"

#define MAINEXEC
#define NB_EXT_ACCEPTED     4

static char *uselesPass = "Xaeéàhf35+AC§4T4§/d,aprén";
static const char *acceptedExt[NB_EXT_ACCEPTED] = {
    ".jpeg",
    ".jpg",
    ".png",
    ".tif"
};

static void 
checkExtensionFile(const char *filename)
{
    const char *ext = filename+strlen(filename)-1;
    while (ext > filename && *ext != '.' && *ext != '/') { ext--; }

    if (*ext != '.')
    {
        fprintf(stderr, RED("Error") ": no file extension detected\n");
        exit(1);
    }

    bool accepted = false;
    for(int i=0; i<NB_EXT_ACCEPTED; i++)
    {
        if (strcmp(ext, acceptedExt[i]) == 0)
            accepted = true;
    }

    if (!accepted)
    {
        fprintf(stderr, RED("Error") ": file extension not accepted\n");
        exit(1);
    }
}

char *
get_error_from_pipe(int pipe_d)
{
    char buff[256] = { 0 };
    char *finalStr = NULL;
    char *strPtr;

    int n = -1;
    if ((n = read(pipe_d, buff, sizeof buff)) <= 0)
    {
        if (n == -1)
        {
            perror("read");
            exit(1);
        }
        return NULL;
    }

    if (!(strPtr = strstr(buff, "steghide:")))
    {
        fprintf(stderr, "Error: cannot achieved to find expected output begenning\n");
        exit(1);
    }

    char *tmp = strPtr;
    while (*tmp != '\0' && *tmp != '\n') { tmp++; }


    finalStr = calloc((tmp-strPtr)+1, sizeof *finalStr);
    if (!finalStr)
    {
        perror("calloc");
        exit(1);
    }

    strncpy(finalStr, strPtr, (tmp-strPtr));
    finalStr[(tmp-strPtr)] = '\0';

    return finalStr;
}

char *
get_steghide_error(const char *filename)
{
    checkExtensionFile(filename);

    int perr[2]; // stderr pipe
    if (pipe(perr) < 0)
    {
        perror("pipe");
        exit(1);
    }

    pid_t id = fork();
    if (!id)
    {
        close(perr[0]);
        if (dup2(perr[1], STDERR_FILENO) < 0)
        {
            perror("dup2");
            exit(667);
        }
        execlp("steghide", "steghide", "extract", "-sf", filename, "-p", uselesPass, "-xf", "/dev/null", NULL);
        perror("execlp");
        exit(667);
    }
    close(perr[1]);

    sleep(1);

    char *output = get_error_from_pipe(perr[0]);
    int wexit;
    waitpid(id, &wexit, 0);
    return output;
}

#undef MAINEXEC
#ifdef MAINEXEC
static void
usage(const char *cmd)
{
    fprintf(stderr, "Usage: %s <stego-sample-file>\n", cmd);
    exit(1);
}

int 
main(int argc, char **argv)
{
    if (argc != 2)
        usage(argv[0]);
    
    char *output = get_steghide_error(argv[1]);
    if (output)
    {
        printf("Error ouput = \"%s\"\n\n", output);

        printf("C formatted buffer:\n");
        printf("char *err_out = { ");
        for (size_t i = 0; i < strlen(output)-1; i++)
        {
            printf("%d, ", output[i]);
        }
        printf("%d };\n", output[strlen(output)-1]);

        return 0;
    }
    fprintf(stderr, "Error: cannot achieved to find error string\n");
    return 1;
}
#endif