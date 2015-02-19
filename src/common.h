#ifndef __common_h
#define __common_h

#include <stdlib.h>
#include <stdbool.h>


/* prototypes */
unsigned int read_exact(int fd, char *buf, size_t count, 
		size_t max_per_read, bool dummy_buf);
unsigned int write_exact(int fd, const char *buf, size_t count, 
		 size_t max_per_write, bool dummy_buf);
void error(const char *msg);

#endif
