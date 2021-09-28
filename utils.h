#ifndef _SB_UTILS_H_
#define _SB_UTILS_H_

#include <stdlib.h>
#include <ctype.h>

#define RED(x)     "\x1b[31m" x "\x1b[0m"  
#define YELLOW(x)  "\x1b[33m" x "\x1b[0m"  
#define MAGENTA(x) "\x1b[35m" x "\x1b[0m"  
#define CYAN(x)   "\x1b[36m" x "\x1b[0m" 
#define BOLD(x)     "\x1b[1m" x "\x1b[0m"

#define STEG_OUTFILE        "data.txt"

typedef unsigned int uint;

extern void* mem_alloc(size_t size);

/**
 *  @brief Shift a from left to pos, the char at pos is deleting.
 */
extern void shift_str_left(char *, size_t pos);

/**
 *  @brief Remove tabulation & line feeds at begenning and end of a string 
 */
extern void trim(char *s);

/**
 *  @brief read from a file descriptor until a specific character. If the character is never met, returns the chars readed though
 */
extern size_t read_until_char(int fd, char *buffer, size_t sizeofbuff, char until);

/**
 *  @brief exec steghide with params stegfile and pass and returns output in buffer
 */
extern void exec_steghide(const char *stegfile, const char *pass, char *buffer, size_t sizeof_buffer);

/**
 *  @brief virtually split a string into nb_segment by returning nb_segment pointers to the begenning of each segment
 *  you might free the returned pointer but DO NOT FREE the pointers contained in
 */
extern char** segment_string(const char *s, uint nb_segment);

/**
 *  @brief check if the submitted string is a positive integer
 */
extern bool isnumber(char *s);

#endif