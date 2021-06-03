#include <stdlib.h>

/**
 *  @brief get the steghide error output for bad pass on the local machine by trying reading data in a stegfile (pointed by filename) 
 */
extern char* get_steghide_error(const char *filename);


extern char* get_error_from_pipe(int pipe_d);