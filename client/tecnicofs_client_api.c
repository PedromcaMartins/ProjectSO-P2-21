#include "tecnicofs_client_api.h"

// global variables
int server_pipe;
int client_pipe;
int session_id = -1;

pthread_mutex_t pipe_lock;

int tfs_mount(char const *client_pipe_path, char const *server_pipe_path){
    pthread_mutex_init(&pipe_lock, NULL);

    // creates it's pipe (self)
    if (pipe_init(client_pipe_path) == -1)
        return -1;

    signal (SIGINT, cntrlc_client);
    // open server pipe for writing
    server_pipe = pipe_open(server_pipe_path, O_WRONLY);
    if (server_pipe == -1)
        return -1;

    // creates the buffer to write to the server's pipe
    void *pipe_buffer[MAX_BUFFER_SIZE];
    size_t offset = 0;

    buffer_write_int(pipe_buffer, offset, TFS_OP_CODE_MOUNT);
    offset += sizeof(int);

    buffer_write_char(pipe_buffer, offset, client_pipe_path, MAX_SIZE_PATHNAME);
    offset += sizeof(char) * MAX_SIZE_PATHNAME;

    // FIXME #13
    // requests the server to mount the client to the server using the server's pipe
    pthread_mutex_lock(&pipe_lock);
    if (pipe_write(server_pipe, pipe_buffer, offset) == -1){
        pthread_mutex_unlock(&pipe_lock);
        return -1;
    }
    pthread_mutex_unlock(&pipe_lock);

    // open self pipe for reading
    client_pipe = pipe_open(client_pipe_path, O_RDONLY);
    if (client_pipe == -1)
        return -1;

    // reads the server's response
    session_id = pipe_read_int(client_pipe);
    if (session_id == -1)
        return -1;
    signal(SIGPIPE, SIG_IGN);
    return 0;
}

int tfs_unmount(){
    // creates the buffer to write to the server's pipe
    void *pipe_buffer[MAX_BUFFER_SIZE];
    size_t offset = 0;

    buffer_write_int(pipe_buffer, offset, TFS_OP_CODE_UNMOUNT);
    offset += sizeof(int);

    buffer_write_int(pipe_buffer, offset, session_id);
    offset += sizeof(int);

    //resquests the server to unmount the client
    pthread_mutex_lock(&pipe_lock);
    if (pipe_write(server_pipe, pipe_buffer, offset+1) == -1){
        pthread_mutex_unlock(&pipe_lock);
        return -1;
    }
    pthread_mutex_unlock(&pipe_lock);

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
    // creates the buffer to write to the server's pipe
    void *pipe_buffer[MAX_BUFFER_SIZE];
    size_t offset = 0;

    buffer_write_int(pipe_buffer, offset, TFS_OP_CODE_OPEN);
    offset += sizeof(int);

    buffer_write_int(pipe_buffer, offset, session_id);
    offset += sizeof(int);

    buffer_write_char(pipe_buffer, offset, name, MAX_SIZE_PATHNAME);
    offset += sizeof(char) * (MAX_SIZE_PATHNAME - 1);

    buffer_write_int(pipe_buffer, offset, flags);
    offset += sizeof(int);

    // writes to the server the the requests
    pthread_mutex_lock(&pipe_lock);
    if (pipe_write(server_pipe, pipe_buffer, offset+1) == -1){
        pthread_mutex_unlock(&pipe_lock);
        return -1;
    }
    pthread_mutex_unlock(&pipe_lock);

    // returns the server's response
    return pipe_read_int(client_pipe);
}

int tfs_close(int fhandle){
    // creates the buffer to write to the server's pipe
    void *pipe_buffer[MAX_BUFFER_SIZE];
    size_t offset = 0;

    buffer_write_int(pipe_buffer, offset, TFS_OP_CODE_CLOSE);
    offset += sizeof(int);

    buffer_write_int(pipe_buffer, offset, session_id);
    offset += sizeof(int);

    buffer_write_int(pipe_buffer, offset, fhandle);
    offset += sizeof(int);

    pthread_mutex_lock(&pipe_lock);
    if (pipe_write(server_pipe, pipe_buffer, offset+1) == -1){
        pthread_mutex_unlock(&pipe_lock);
        return -1;
    }
    pthread_mutex_unlock(&pipe_lock);

    // returns the server's response
    return pipe_read_int(client_pipe);
}

ssize_t tfs_write(int fhandle, void const *buffer, size_t len){
    len++;
    // creates the buffer to write to the server's pipe
    void *pipe_buffer[MAX_BUFFER_SIZE];
    size_t offset = 0;

    buffer_write_int(pipe_buffer, offset, TFS_OP_CODE_WRITE);
    offset += sizeof(int);

    buffer_write_int(pipe_buffer, offset, session_id);
    offset += sizeof(int);

    buffer_write_int(pipe_buffer, offset, fhandle);
    offset += sizeof(int);

    buffer_write_size_t(pipe_buffer, offset, len);
    offset += sizeof(size_t);

    buffer_write_char(pipe_buffer, offset, buffer, len);
    offset += sizeof(char) * len;

    pthread_mutex_lock(&pipe_lock);
    if (pipe_write(server_pipe, pipe_buffer, offset) == -1){
        pthread_mutex_unlock(&pipe_lock);
        return -1;
    }
    pthread_mutex_unlock(&pipe_lock);

    // returns the server's response
    return pipe_read_ssize_t(client_pipe);
}

ssize_t tfs_read(int fhandle, void *buffer, size_t len){
    // creates the buffer to write to the server's pipe
    void *pipe_buffer[MAX_BUFFER_SIZE];
    size_t offset = 0;

    buffer_write_int(pipe_buffer, offset, TFS_OP_CODE_READ);
    offset += sizeof(int);

    buffer_write_int(pipe_buffer, offset, session_id);
    offset += sizeof(int);

    buffer_write_int(pipe_buffer, offset, fhandle);
    offset += sizeof(int);

    buffer_write_size_t(pipe_buffer, offset, len);
    offset += sizeof(size_t);

    pthread_mutex_lock(&pipe_lock);
    if (pipe_write(server_pipe, pipe_buffer, offset+1) == -1){
        pthread_mutex_unlock(&pipe_lock);
        return -1;
    }
    pthread_mutex_unlock(&pipe_lock);

    // returns the server's response
    ssize_t read = pipe_read_ssize_t(client_pipe);
    if (read == -1)
        return -1;
    pipe_read(client_pipe, buffer, (size_t)(read));
    read--;
    return read;
}

int tfs_shutdown_after_all_closed(){
    // creates the buffer to write to the server's pipe
    void *pipe_buffer[MAX_BUFFER_SIZE];
    size_t offset = 0;

    buffer_write_int(pipe_buffer, offset, TFS_OP_CODE_SHUTDOWN_AFTER_ALL_CLOSED);
    offset += sizeof(int);

    buffer_write_int(pipe_buffer, offset, session_id);
    offset += sizeof(int);

    pthread_mutex_lock(&pipe_lock);
    if (pipe_write(server_pipe, pipe_buffer, offset+1) == -1){
        pthread_mutex_unlock(&pipe_lock);
        return -1;
    }
    pthread_mutex_unlock(&pipe_lock);

    // returns the server's response
    int res = pipe_read_int(client_pipe);

    // closes the server's pipe
    if(pipe_close(server_pipe) == -1)
        return -1;

    return res;
}

void cntrlc_client(){
    if (session_id != -1){
        tfs_unmount();
    }
    exit(0);
}
