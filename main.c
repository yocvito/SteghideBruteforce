#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

#include <pthread.h>

#include "threads.h"
#include "utils.h"
#include "get-stegerr.h"

static char * error_output = NULL;
static int nb_threads = 4;

static void
usage(const char *cmd)
{
    fprintf(stderr, YELLOW("Usage")": %s <wordlist> <stegfile> [<nb-threads>] [--verbose] (default number of threads: %d)\n", cmd, nb_threads);
    exit(1);
}

int
main(int argc, char **argv)
{
    bool verbosity = false;
    if (argc == 5)
    {
        if (!isnumber(argv[3]))
        {
            if (!isnumber(argv[4]))
                usage(argv[0]);
            else if (strcmp(argv[3], "--verbose") != 0)
            {
                fprintf(stderr, "Unrecognized option: %s\n", argv[3]);
                usage(argv[0]);
            }
            verbosity = true;
            nb_threads = atoi(argv[4]);
        } 
        else 
        {
            if (strcmp(argv[4], "--verbose") != 0)
            {
                fprintf(stderr, "Unrecognized option: %s\n", argv[4]);
                usage(argv[0]);
            }
            verbosity = true;
            nb_threads = atoi(argv[3]);
        }
    }
    else if (argc == 4)
    {
        if (isnumber(argv[3]))
        {
            nb_threads = atoi(argv[3]);
        }
        else if (strcmp(argv[3], "--verbose") == 0)
        {
            verbosity = true;
        }
        else
            usage(argv[0]);
    }
    else if (argc != 3)
        usage(argv[0]);

    char *filename = strdup(argv[2]);
    error_output = get_steghide_error(filename);
    if (verbosity)
        printf(CYAN("Steghide error output: ") "\"%s\"\n", error_output);

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0)
    {
        perror("open");
        exit(1);
    }
    size_t wl_len = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    char *wl = mmap(NULL, wl_len, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (fd < 0)
    {
        perror("mmap");
        exit(1);
    }

    pthread_t *threads = sb_run_threads(nb_threads, wl, error_output, filename, verbosity);

    char pass[100] = { 0 };
    if (sb_join_threads(threads, nb_threads, pass, sizeof pass))
    {
        printf(MAGENTA("\nPassword found => %s\n"), pass);
	    printf(MAGENTA("Extracted data => %s\n"), STEG_OUTFILE);
    }
    else
    {
        printf(RED("\nPassword not found !\n"));
    }

    printf(MAGENTA("Exec time => %ld s\n"), sb_get_exec_time());

    if (munmap(wl, wl_len) < 0)
    {
        perror("munmap");
        exit(1);
    }
    close(fd);
    free(error_output);
    free(filename);

    return 0;
}
