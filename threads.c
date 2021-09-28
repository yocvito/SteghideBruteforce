#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>

#include "threads.h"
#include "utils.h"

static int onexit = 0;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static char password[256] = { 0 };
static char *error_output = NULL;
static char *stegfile = NULL;
static time_t t_start = -1;
static time_t t_end = -1;
static bool verbose = false;

/********** UTILS **********/
static void*
thread_func(void *args)
{
    char **params = (char**)args;
    char *begin = params[0];
    char *end = params[1];
    uintptr_t success = 0;
    long id = syscall(__NR_gettid);

    if (verbose)
    {
        pthread_mutex_lock(&mutex);
        printf(CYAN("[THREAD %ld]")" working from %p to %p\n", id, begin, end);
        pthread_mutex_unlock(&mutex);
    }
    // TODO
    char *currentPtr = begin;
    char current[256] = { 0 };
    char buffer[256] = { 0 };
    char *tmp;
    int nb_tries = 0;
    while (currentPtr < end && !onexit)
    {
        memset(current, 0, sizeof current);
        tmp = currentPtr;
        while (*tmp != '\n' && *(tmp++) != '\0');

        assert((size_t) (tmp-currentPtr) < sizeof current);
        strncpy(current, currentPtr, tmp-currentPtr);
        current[tmp-currentPtr] = '\0';
        currentPtr = tmp+1;

        //if (verbose)
        //    printf(CYAN("[THREAD %ld] (%d)")" trying \"%s\"\n", id, nb_tries, current);

        exec_steghide(stegfile, current, buffer, sizeof buffer);
        trim(buffer);
        if (strcmp(buffer, error_output) != 0)
        {
            t_end = time(NULL);
            success = 1;
            onexit = 1;
            snprintf(password, sizeof password, "%s", current);
        }
        nb_tries++;
    }

    return (void*) success;
}

/********** API **********/

static char ***params;
pthread_t* 
sb_run_threads(uint nb_threads, char *wl, char *error, char *filename, bool verbosity)
{
    verbose = verbosity;
    pthread_t *ths = mem_alloc(sizeof *ths * nb_threads);
    error_output = strdup(error);
    stegfile = strdup(filename);
 
    char **pointers = segment_string(wl, nb_threads);
    params = mem_alloc(sizeof *params * nb_threads);
    for (uint i=0; i<nb_threads; i++)
    {
        params[i] = mem_alloc (sizeof *params[i] * 2);
        params[i][0] = pointers[i];
        if (i == nb_threads-1)
            params[i][1] = wl+strlen(wl);
        else
            params[i][1] = pointers[i+1];
    }
    if (verbose)
    {
        printf(CYAN("Info: ")"Wordlist is from %p to %p (%lu bytes)\n", wl, wl+strlen(wl), strlen(wl));
        printf(CYAN("Info: ")"Starting to bruteforce with %d threads !\n", nb_threads);
    }
    t_start = time(NULL);
    for (uint i=0; i<nb_threads ; i++)
    {
        pthread_create(ths+i, NULL, thread_func, (void*)params[i]);
    }
    free(pointers);

    return ths;
}

time_t
sb_get_exec_time()
{
    if (t_start < 0 || t_end < 0)
        return -1;
    return t_end-t_start;
}

bool 
sb_join_threads(pthread_t *threads, uint nb_threads, char *s, size_t sizeof_s)
{
    bool ret = false;
    uintptr_t success = 0;
    for (uint i=0; i<nb_threads ; i++)
    {
        pthread_join(threads[i], (void*)&success);
        if (success == 1)
        {
            ret = true;
            assert(strlen(password) <= sizeof_s);
            snprintf(s, sizeof_s, "%s", password);
        }
    }
    for (uint i=0; i<nb_threads ; i++)
    {
        free(params[i]);
    }
    free(params);
    free(threads);
    return ret;
}