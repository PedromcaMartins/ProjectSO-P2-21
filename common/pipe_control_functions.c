#include "pipe_control_functions.h"

int pipe_init(char const *pipe_pathname){
    // remove server pipe if it does not exist
    if (unlink(pipe_pathname) != 0 && errno != ENOENT) {
        fprintf(stderr, "[ERR]: unlink(%s) failed: %s\n", pipe_pathname,
        strerror(errno));
        exit(EXIT_FAILURE);
        return -1;
    }

    // create server pipe
    if (mkfifo(pipe_pathname, 0640) != 0) {
        fprintf(stderr, "[ERR]: mkfifo failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
        return -1;
    }

    //operation successful
    return 0;
}

int pipe_open(char const *pipe_pathname, int function){
    int phandle = open(pipe_pathname, function);
    if (phandle == -1) {
        fprintf(stderr, "[ERR]: open failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
        return -1;
    }

    return phandle;
}

size_t pipe_write(int phandle, void const *str, size_t len) {
    size_t written = 0;
    len--;

    while (written < len) {
        ssize_t ret = write(phandle, str + written, len - written);
        if (ret < 0) {
            fprintf(stderr, "[ERR]: write failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
            return (size_t)(-1);
        }

        written += (size_t)(ret);
    }

    // returns amount written
    return written;
}

ssize_t pipe_read(int phandle, void* buffer, size_t size){
    ssize_t ret = read(phandle, buffer, size-1);
    if (ret == 0) {
        // ret == 0 signals EOF
        fprintf(stderr, "[INFO]: pipe closed\n");
        return 0;
    } else if (ret == -1) {
        // ret == -1 signals error
        fprintf(stderr, "[ERR]: read failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
        return -1;
    } else if (ret > size - 1)
        return -1;

    // returns amount read
    return ret;
}

int pipe_close(int phandle){
    return close(phandle);
}

int pipe_destroy(char const *pipe_pathname){
    // remove server pipe if it does not exist
    if (unlink(pipe_pathname) != 0 && errno != ENOENT) {
        fprintf(stderr, "[ERR]: unlink(%s) failed: %s\n", pipe_pathname,
        strerror(errno));
        exit(EXIT_FAILURE);
        return -1;
    }

    //operation successful
    return 0;
}

// TODO: #8 improve the reading
int pipe_read_int(int phandle){
    void *buffer[sizeof(int)];

    // write the int to the pipe
    if (pipe_read(phandle, buffer, sizeof(int) + 1) == -1)
        return -1;

    // reads the buffer as an array of integers
    int *buffer_int = (int *)buffer;

    // returns amount written
    return buffer_int[0];
}

// TODO: #9 improve the writting
int pipe_write_int(int phandle, int msg){
    void *buffer[sizeof(int)];

    // casts the buffer as an array and adds the integer to the array
    int *buffer_int = (int *)buffer;
    buffer_int[0] = msg;

    // write the int to the pipe
    ssize_t ret = write(phandle, buffer, sizeof(int));

    // returns -1 if there is error
    if (ret != (ssize_t)sizeof(int)){
        printf("ERROR\n");
        return -1;
    }
    return 0;
}

ssize_t pipe_read_ssize_t(int phandle){
    void *buffer[sizeof(ssize_t)];

    // write the int to the pipe
    if (pipe_read(phandle, buffer, sizeof(ssize_t) + 1) == -1)
        return -1;

    // reads the buffer as an array of integers
    ssize_t *buffer_ssize_t = (ssize_t *)buffer;

    // returns amount written
    return buffer_ssize_t[0];
}

int pipe_write_ssize_t(int phandle, ssize_t msg){
    void *buffer[sizeof(ssize_t)];

    // casts the buffer as an array and adds the integer to the array
    ssize_t *buffer_ssize_t = (ssize_t *)buffer;
    buffer_ssize_t[0] = msg;

    // write the int to the pipe
    ssize_t ret = write(phandle, buffer, sizeof(ssize_t));

    // returns -1 if there is error
    if (ret != (ssize_t)sizeof(ssize_t))
        return -1;
    return 0;
}

size_t pipe_read_size_t(int phandle){
    void *buffer[sizeof(size_t)];

    // write the int to the pipe
    if (pipe_read(phandle, buffer, sizeof(size_t) + 1) == -1)
        return (size_t)(-1);

    // reads the buffer as an array of integers
    size_t *buffer_size_t = (size_t *)buffer;

    // returns amount written
    return buffer_size_t[0];
}

int pipe_write_size_t(int phandle, size_t msg){
    void *buffer[sizeof(size_t)];

    // casts the buffer as an array and adds the integer to the array
    size_t *buffer_size_t = (size_t *)buffer;
    buffer_size_t[0] = msg;

    // write the int to the pipe
    ssize_t ret = write(phandle, buffer, sizeof(size_t));

    // returns -1 if there is error
    if (ret != (size_t)sizeof(size_t))
        return -1;
    return 0;
}
