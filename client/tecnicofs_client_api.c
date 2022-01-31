#include "common/pipe_control_functions.h"
#include "tecnicofs_client_api.h"

// global variables
int server_pipe;
int client_pipe;
int session_id;

int tfs_mount(char const *client_pipe_path, char const *server_pipe_path){
    // creates it's pipe (self)
    if (pipe_init(client_pipe_path) == -1)
        return -1;

    // open server pipe for writing
    server_pipe = pipe_open(server_pipe_path, O_WRONLY);
    if (server_pipe == -1)
        return -1;

    // requests the server to mount the client to the server using the server's pipe
    if (pipe_write(server_pipe, "1", 2) == -1)
        return -1;

    if (pipe_write(server_pipe, client_pipe_path, MAX_SIZE_PATHNAME) == -1)
        return -1;

    // open self pipe for reading
    client_pipe = pipe_open(client_pipe_path, O_RDONLY);
    if (client_pipe == -1)
        return -1;

    // reads the server's response
    session_id = pipe_read_int(client_pipe);
    if (session_id == -1)
        return -1;
    return 0;
}

int tfs_unmount(){
    //resquests the server to unmount the client
    if(pipe_write(server_pipe, "2", 2) == -1)
        return -1;

    // gives the session id to the server
    if (pipe_write_int(server_pipe, session_id) == -1)
        return -1;

    // closes the server's pipe
    if(pipe_close(server_pipe) == -1)
        return -1;

    // reads the server's response
    if (pipe_read_int(client_pipe) == -1)
        return -1;

    // closes the client's pipe
    if(pipe_close(client_pipe) == -1)
        return -1;

    return 0;
}

// TODO: o name que a funcao recebe tem que ter capacidade de 40?
int tfs_open(char const *name, int flags){
    // writes to the server the the requests
    if (pipe_write(server_pipe, "3", 2) == -1)
        return -1;
    if (pipe_write_int(server_pipe, session_id) == -1)
        return -1;
    if (pipe_write(server_pipe, name, MAX_SIZE_PATHNAME) == -1)
        return -1;
    if (pipe_write_int(server_pipe, flags) == -1)
        return -1;

    // returns the server's response
    return pipe_read_int(client_pipe);
}

int tfs_close(int fhandle){
    if (pipe_write(server_pipe, "4", 2) == -1)
        return -1;
    if (pipe_write_int(server_pipe, session_id) == -1)
        return -1;
    if (pipe_write_int(server_pipe, fhandle) == -1)
        return -1;
    
    // returns the server's response
    return pipe_read_int(client_pipe);
}

ssize_t tfs_write(int fhandle, void const *buffer, size_t len){
    if (pipe_write(server_pipe, "5", 2) == -1)
        return -1;
    if (pipe_write_int(server_pipe, session_id) == -1)
        return -1;
    if (pipe_write_int(server_pipe, fhandle) == -1)
        return -1;
    if (pipe_write_ssize_t(server_pipe, len) == -1)
        return -1;
    if (pipe_write(server_pipe, buffer, len) == -1)
        return -1;

    // returns the server's response
    return pipe_read_ssize_t(client_pipe);
}

ssize_t tfs_read(int fhandle, void *buffer, size_t len){
    if (pipe_write(server_pipe, "6", 2) == -1)
        return -1;
    if (pipe_write_int(server_pipe, session_id) == -1)
        return -1;
    if (pipe_write_int(server_pipe, fhandle) == -1)
        return -1;
    if (pipe_write_ssize_t(server_pipe, len) == -1)
        return -1;

    // returns the server's response
    int read = pipe_read_ssize_t(client_pipe);
    if (read == -1)
        return -1;
    pipe_read(client_pipe, buffer, read);
    printf("%s\n", (char *)buffer);

    return read;
}

int tfs_shutdown_after_all_closed(){
    if (pipe_write(server_pipe, "7", 2) == -1)
        return -1;
    if (pipe_write_int(server_pipe, session_id) == -1)
        return -1;

    // returns the server's response
    return pipe_read_int(client_pipe);
}
