#include "tfs_server.h"

int request_thread_mount(){
    // creates the buffer to write to the thread's buffer
    void *thread_buffer[MAX_BUFFER_SIZE];
    size_t offset = 0;

    // writes op_code
    buffer_write_int(thread_buffer, offset, TFS_OP_CODE_MOUNT);
    offset += sizeof(int);

    // reads the client's pathname from the server's pipe
    char client_pipe_path[MAX_SIZE_PATHNAME];
    ssize_t ret = pipe_read(server_pipe, client_pipe_path, MAX_SIZE_PATHNAME);
    // validates it
    if (ret == -1){
        return -1;
    }

    // writes the client's request to the thread's buffer
    buffer_write_char(thread_buffer, offset, client_pipe_path, MAX_SIZE_PATHNAME - 1);
    offset += sizeof(char) * (MAX_SIZE_PATHNAME - 1);

    int session_id = add_to_session_table();
    if (session_id == -1){
        int client_pipe = pipe_open(client_pipe_path, O_WRONLY);
        if (client_pipe == -1)
            return -1;

        // the aren't more sessions avaiable
        pipe_write_int(client_pipe, -1);
        return -1;
    }
    // thread executes client_mount();
    thread_status(session_id, THREAD_STATUS_ACTIVE, thread_buffer);

    return 0;
}

int request_thread_unmount(int session_id){
    // creates the buffer to write to the thread's buffer
    void *thread_buffer[MAX_BUFFER_SIZE];
    size_t offset = 0;

    // writes op_code
    buffer_write_int(thread_buffer, offset, TFS_OP_CODE_UNMOUNT);
    offset += sizeof(int);

    // thread executes client_unmount();
    thread_status(session_id, THREAD_STATUS_ACTIVE, thread_buffer);

    return 0;
}

int request_thread_open(int session_id){
    // creates the buffer to write to the thread's buffer
    void *thread_buffer[MAX_BUFFER_SIZE];
    size_t offset = 0;

    // writes op_code
    buffer_write_int(thread_buffer, offset, TFS_OP_CODE_OPEN);
    offset += sizeof(int);

    char name[MAX_SIZE_PATHNAME];
    ssize_t ret = pipe_read(server_pipe, name, MAX_SIZE_PATHNAME);
    if (ret <= 0){
        server_destroy();
        return -1;
    }

    // writes the client's request to the thread's buffer
    buffer_write_char(thread_buffer, offset, name, MAX_SIZE_PATHNAME - 1);
    offset += sizeof(char) * (MAX_SIZE_PATHNAME - 1);

    int flags = pipe_read_int(server_pipe);

    buffer_write_int(thread_buffer, offset, flags);
    offset += sizeof(int);

    // thread executes client_open();
    thread_status(session_id, THREAD_STATUS_ACTIVE, thread_buffer);

    return 0;
}

int request_thread_close(int session_id){
    // creates the buffer to write to the thread's buffer
    void *thread_buffer[MAX_BUFFER_SIZE];
    size_t offset = 0;

    // writes op_code
    buffer_write_int(thread_buffer, offset, TFS_OP_CODE_CLOSE);
    offset += sizeof(int);

    int fhandle = pipe_read_int(server_pipe);

    buffer_write_int(thread_buffer, offset, fhandle);
    offset += sizeof(int);

    // thread executes client_close();
    thread_status(session_id, THREAD_STATUS_ACTIVE, thread_buffer);
    
    return 0;
}

int request_thread_write(int session_id){
    // creates the buffer to write to the thread's buffer
    void *thread_buffer[MAX_BUFFER_SIZE];
    size_t offset = 0;

    // writes op_code
    buffer_write_int(thread_buffer, offset, TFS_OP_CODE_WRITE);
    offset += sizeof(int);

    int fhandle = pipe_read_int(server_pipe);

    buffer_write_int(thread_buffer, offset, fhandle);
    offset += sizeof(int);

    size_t len = pipe_read_size_t(server_pipe);

    buffer_write_size_t(thread_buffer, offset, len);
    offset += sizeof(size_t);

    char buffer[len];
    ssize_t ret = pipe_read(server_pipe, buffer, len);
    if (ret == 0)
        return -1;
    buffer[ret] = '\0';

    // writes the client's request to the thread's buffer
    buffer_write_char(thread_buffer, offset, buffer, len);
    offset += sizeof(char) * (len);

    // thread executes client_write();
    thread_status(session_id, THREAD_STATUS_ACTIVE, thread_buffer);

    return 0;
}

int request_thread_read(int session_id){
    // creates the buffer to write to the thread's buffer
    void *thread_buffer[MAX_BUFFER_SIZE];
    size_t offset = 0;

    // writes op_code
    buffer_write_int(thread_buffer, offset, TFS_OP_CODE_READ);
    offset += sizeof(int);

    int fhandle = pipe_read_int(server_pipe);

    buffer_write_int(thread_buffer, offset, fhandle);
    offset += sizeof(int);

    size_t len = pipe_read_size_t(server_pipe);

    buffer_write_size_t(thread_buffer, offset, len);
    offset += sizeof(size_t);

    // thread executes client_read();
    thread_status(session_id, THREAD_STATUS_ACTIVE, thread_buffer);
    
    return 0;
}

int request_thread_destroy(int session_id){
    // destroys the server
    if (server_destroy() == -1)
        return -1;

    // closes the client's pipe
    if (client_unmount(session_id) == -1)
        return -1;

    printf("[INFO] server destroyed\n");

    return 0;
}



int main(int argc, char **argv) {

    if (argc < 2) {
        printf("Please specify the pathname of the server's pipe.\n");
        return 1;
    }

    server_pipe_path = argv[1];
    printf("Starting TecnicoFS server with pipe called %s\n", server_pipe_path);

    //initializes the server
    assert(server_init() != -1);

    signal(SIGINT, cntrlc_server);
    signal(SIGPIPE, SIG_IGN);

    while (decode() != -1 && server_status == true) {}

    printf("[INFO] Server destroyed\n");

    return 0;
}

int server_init() {
    // creates open pipe table
    session_table_init();
    if (tfs_init() == -1)
        return -1;

    // creates server pipe (self)
    if (pipe_init(server_pipe_path) == -1)
        return -1;

    // open self pipe for reading
    server_pipe = pipe_open(server_pipe_path, O_RDONLY);

    if (server_pipe == -1)
        return -1;

    return 0;
}

int server_destroy(){
    // closes the server's pipe
    if (pipe_close(server_pipe) == -1)
        return -1;

    //destroys the server's pipe
    if (pipe_destroy(server_pipename) == -1)
        return -1;

    // TODO: add functionality to wait until all pipes are closed except one
    // TODO: every error until here is reported back to the client w/ -1

    return tfs_destroy_after_all_closed();
}

void cntrlc_server(){
    printf("Destroying_server\n");
    server_destroy();
    exit(0);
}

int decode(){
    int session_id;
    int command = pipe_read_int(server_pipe);

    switch (command){
    case TFS_OP_CODE_MOUNT:
        request_thread_mount();
        break;

    case TFS_OP_CODE_UNMOUNT:
        session_id = pipe_read_int(server_pipe);
        if (session_id == -1) // rebentar o server
            server_destroy();
        request_thread_unmount(session_id);
        break;

    case TFS_OP_CODE_OPEN:
        session_id = pipe_read_int(server_pipe);
        if (session_id == -1) // rebentar o server
            server_destroy();
        request_thread_open(session_id);
        break;

    case TFS_OP_CODE_CLOSE:
        session_id = pipe_read_int(server_pipe);
        if (session_id == -1) // rebentar o server
            server_destroy();
        request_thread_close(session_id);
        break;

    case TFS_OP_CODE_WRITE:
        session_id = pipe_read_int(server_pipe);
        if (session_id == -1) // rebentar o server
            server_destroy();
        request_thread_write(session_id);
        break;

    case TFS_OP_CODE_READ:
        session_id = pipe_read_int(server_pipe);
        if (session_id == -1) // rebentar o server
            server_destroy();
        request_thread_read(session_id);
        break;

    case TFS_OP_CODE_SHUTDOWN_AFTER_ALL_CLOSED:
        printf("CASE 7\n");
        session_id = pipe_read_int(server_pipe);
        if (session_id == -1) // rebentar o server
            server_destroy();
        request_thread_destroy(session_id);
        break;
    case TFS_OP_CODE_SERVER_PIPE_CLOSED:
        pipe_close(server_pipe);
        pipe_open(server_pipe_path, O_RDONLY);
        break;
    default:
        return -1;
        break;
    }

    return 0;
}
