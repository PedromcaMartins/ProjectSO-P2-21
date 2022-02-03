#include "tfs_server.h"

//fixme! remove
int request_thread_mount();
int request_thread_unmount(int session_id);
int request_thread_open(int session_id);
int request_thread_close(int session_id);
int request_thread_write(int session_id);
int request_thread_read(int session_id);
int request_thread_destroy(int session_id);

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

    int session_id = thread_mount();
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

// TODO: #7 o que fazer no caso de erro?
int request_thread_unmount(int session_id){
    // gets the client's info
    int client_pipe = get_phandle_from_session_table(session_id);
    char *client_pathname = get_pathname_from_session_table(session_id);

    // unregisters the client's session_id
    if (remove_from_session_table(session_id) == -1){
        // if gone wrong, returns an error message
        pipe_write_int(client_pipe, -1);
        return -1;
    }

    // sends SUCCESS message to the client
    if (pipe_write_int(client_pipe, 0) == -1)
        return -1;

    // closes the client's pipe, resuming the execution it's code
    if (pipe_close(client_pipe) == -1)
        return -1;
    
    // destroys the client's pipe
    if(pipe_destroy(client_pathname) == -1)
        return -1;

    return 0;
}

int request_thread_open(int session_id){
    printf("%d\t", session_id);

    char name[MAX_SIZE_PATHNAME];
    ssize_t ret = pipe_read(server_pipe, name, MAX_SIZE_PATHNAME);
    if (ret <= 0){
        server_destroy();
        return -1;
    }
    printf("%s\t", name);

    int flags = pipe_read_int(server_pipe);
    printf("%d\n", flags);

    int fhandle = tfs_open(name, flags);

    // writes to the client SUCCESS!
    int client_pipe = get_phandle_from_session_table(session_id);

    // write the server's response
    pipe_write_int(client_pipe, fhandle);
    return 0;
}

int request_thread_close(int session_id){
    printf("%d\t", session_id);

    int fhandle = pipe_read_int(server_pipe);
    printf("%d\n", fhandle);

    int sucess = tfs_close(fhandle);

    // writes to the client SUCCESS!
    int client_pipe = get_phandle_from_session_table(session_id);

    // write the server's response
    pipe_write_int(client_pipe, sucess);
    
    return 0;
}

int request_thread_write(int session_id){
    printf("%d\t", session_id);

    int fhandle = pipe_read_int(server_pipe);
    printf("%d\t", fhandle);

    size_t len = pipe_read_size_t(server_pipe);
    printf("%ld\t", len);

    char buffer[len];
    ssize_t ret = pipe_read(server_pipe, buffer, len);
    if (ret == 0)
        return -1;
    buffer[ret] = 0;
    printf("%s\n", buffer);

    ssize_t res = tfs_write(fhandle, buffer, len);
    res--;

    // writes to the client SUCCESS!
    int client_pipe = get_phandle_from_session_table(session_id);

    // write the server's response
    pipe_write_ssize_t(client_pipe, res);

    return 0;
}

int request_thread_read(int session_id){
    printf("%d\t", session_id);

    int fhandle = pipe_read_int(server_pipe);
    printf("%d\t", fhandle);

    size_t len = pipe_read_size_t(server_pipe);
    printf("%ld\n", len);

    // TODO: adicinoar funcao tfs_read();
    char buffer[len];
    ssize_t res = tfs_read(fhandle, buffer, len);

    // writes to the client SUCCESS!
    int client_pipe = get_phandle_from_session_table(session_id);

    // write the server's response
    pipe_write_ssize_t(client_pipe, res);
    if (res == -1)
        return -1;
    pipe_write(client_pipe, buffer, (size_t)(res));
    
    return 0;
}

int request_thread_destroy(int session_id){
    // destroys the server
    if (server_destroy() == -1)
        return -1;

    // closes the client's pipe
    if (client_unmount(session_id) == -1)
        return -1;

    // closes the server's pipe
    if (pipe_close(server_pipe) == -1)
        return -1;

    //destroys the server's pipe
    if (pipe_destroy(server_pipename) == -1)
        return -1;

    printf("[INFO] server destroyed\n");

    return 0;
}


//fixme! remove

int main(int argc, char **argv) {

    if (argc < 2) {
        printf("Please specify the pathname of the server's pipe.\n");
        return 1;
    }

    server_pipename = argv[1];
    printf("Starting TecnicoFS server with pipe called %s\n", server_pipename);

    
    //initializes the server
    assert(server_init(server_pipename) != -1);

    while (decode() != -1 && server_status == true) {}

    printf("[INFO] Server destroyed\n");

    return 0;
}

int server_init(char const *server_pipe_path) {
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

int decode(){
    int session_id;
    int command = pipe_read_int(server_pipe);

    switch (command){
    case TFS_OP_CODE_MOUNT:
        printf("CASE 1\n");
        request_thread_mount();
        break;

    case TFS_OP_CODE_UNMOUNT:
        printf("CASE 2\n");
        session_id = pipe_read_int(server_pipe);
        if (session_id == -1) // rebentar o server
            server_destroy();
        request_thread_unmount(session_id);
        break;

    case TFS_OP_CODE_OPEN:
        printf("CASE 3\n");
        session_id = pipe_read_int(server_pipe);
        if (session_id == -1) // rebentar o server
            server_destroy();
        request_thread_open(session_id);
        break;

    case TFS_OP_CODE_CLOSE:
        printf("CASE 4\n");
        session_id = pipe_read_int(server_pipe);
        if (session_id == -1) // rebentar o server
            server_destroy();
        request_thread_close(session_id);
        break;

    case TFS_OP_CODE_WRITE:
        printf("CASE 5\n");
        session_id = pipe_read_int(server_pipe);
        if (session_id == -1) // rebentar o server
            server_destroy();
        request_thread_write(session_id);
        break;

    case TFS_OP_CODE_READ:
        printf("CASE 6\n");
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

    default:
        return -1;
        break;
    }

    return 0;
}
