#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <getopt.h>

#include <pthread.h>

#include "threads.h"
#include "utils.h"
#include "get-stegerr.h"

static char * error_output = NULL;
static int nb_threads = 4;

static void
usage(const char *cmd)
{
    fprintf(stderr, BOLD("Usage")": %s <wordlist> <stegfile> [<nb-threads>] [--verbose] (default number of threads: %d)\n", cmd, nb_threads);
    exit(1);
}

static void
print_help(const char *cmd)
{
    fprintf(stderr, BOLD("Brief")": Binary allowing you to bruteforce a steghide file by optionnaly using threads\n");
    fprintf(stderr, BOLD("Usage")": %s -w <wordlist> -f <stegfile> [-t <nb-threads>] [-v]\n", cmd);
    fprintf(stderr, "   -h, --help          Display this help\n");
    fprintf(stderr, "   -v, --verbose       Activate verbosity\n");
    fprintf(stderr, "   -w, --wordlist      Wordlist filename using to bruteforce\n");
    fprintf(stderr, "   -f, --stegfile      Steghide file to bruteforce\n");
    fprintf(stderr, "   -t, --nb-threads    Number of threads to start (default: 4, max: 30)\n");
} 

int
main(int argc, char **argv)
{
    bool verbose = false;
    static struct option long_options[] =
    {
        {"wordlist", required_argument, 0, 'w'},
        {"stegfile", required_argument, 0, 'f'},
        {"nb-threads", required_argument, 0, 't'},
        {"verbose",  no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    char optc = 0;
    char *filename = NULL;
    char *wl_filename = NULL;
    int args_err = 0;
    short required = 0;
    while ((optc = getopt_long(argc, argv, "w:f:t:vh", long_options, NULL)) != -1 && args_err == 0)
    {
        switch (optc)
        {
            case 'v':
                verbose = true;
                break;
            case 'h':
                print_help(argv[0]);
                exit(1);
                break;
            case 'f':
                if (optarg)
                {
                    filename = strdup(optarg);
                    required++;
                }
                else args_err = 1;
                break;
            case 'w':
                if (optarg)
                {
                    wl_filename = strdup(optarg);
                    required++;
                }
                else args_err = 1;
                break;
            case 't':
                if (optarg)
                {
                    if (isnumber(optarg))
                    {
                        nb_threads = atoi(optarg);
                        if (nb_threads > 30)
                        {
                            print_help(argv[0]);
                            exit(1);
                        }
                    }
                    else args_err = 1;
                }
                else args_err = 1;
                break;
            default:
                usage(argv[0]);
                break;
        }
    }
    if (required != 2 || args_err == 1)
    {
        print_help(argv[0]);
        exit(1);
    }
/*
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
            verbose = true;
            nb_threads = atoi(argv[4]);
        } 
        else 
        {
            if (strcmp(argv[4], "--verbose") != 0)
            {
                fprintf(stderr, "Unrecognized option: %s\n", argv[4]);
                usage(argv[0]);
            }
            verbose = true;
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
            verbose = true;
        }
        else
            usage(argv[0]);
    }
    else if (argc != 3)
        usage(argv[0]);

    char *filename = strdup(argv[2]);*/
    error_output = get_steghide_error(filename);
    if (verbose)
        printf(CYAN("Steghide error output: ") "\"%s\"\n", error_output);

    int fd = open(wl_filename, O_RDONLY);
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

    pthread_t *threads = sb_run_threads(nb_threads, wl, error_output, filename, verbose);

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
    free(wl_filename);

    return 0;
}
