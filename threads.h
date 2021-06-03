#ifndef _SB_THREADS_H_
#define _SB_THREADS_H_

#include <stdbool.h>
#include <time.h>

#include <pthread.h>

#include "utils.h"

/**
 *  @brief split wordlist and start specified amount of thread
 */
extern pthread_t* sb_run_threads(uint nb_threads, char *wl, char *error_output, char *stegfile, bool verbose);

extern time_t sb_get_exec_time();

/**
 *  @brief join any thread of the submitted thread array, return true if one thread found the pass and fill the s char pointer
 */
extern bool sb_join_threads(pthread_t *threads, uint nb_threads, char *s, size_t sizeof_s);



#endif