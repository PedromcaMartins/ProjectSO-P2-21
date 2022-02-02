#ifndef PIPE_CONTROL_FUNCTIONS_H
#define PIPE_CONTROL_FUNCTIONS_H

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// unlinks and creates the given pipe
int pipe_init(char const *pipe_pathname);

// opens the pipe to the given function and returns the phandle or -1
int pipe_open(char const *pipe_pathname, int function);

// sends the string throught the pipe's phandle
size_t pipe_write(int phandle, void const *str, size_t len);

// reads the string throught the pipe's phandle
ssize_t pipe_read(int phandle, void* buffer, size_t size);

// closes the given pipe
int pipe_close(int phandle);

// destroys the pipe
int pipe_destroy(char const *pipe_pathname);

// added functionality to improve the coding experience
int pipe_read_int(int phandle);
int pipe_write_int(int phandle, int msg);

size_t pipe_read_size_t(int phandle);
int pipe_write_size_t(int phandle, size_t msg);

ssize_t pipe_read_ssize_t(int phandle);
int pipe_write_ssize_t(int phandle, ssize_t msg);


// writes to the buffer
void pipe_write_buffer(void *buffer, size_t offset, void const *str, size_t len);
void pipe_write_int_buffer(void *buffer, size_t offset, int msg);
void pipe_write_size_t_buffer(void *buffer, size_t offset, size_t msg);
void pipe_write_ssize_t_buffer(void *buffer, size_t offset, ssize_t msg);

#endif // PIPE_CONTROL_FUNCTIONS_H
