#include "request_clients.h"

// char client_pipe_path[MAX_SIZE_PATHNAME]
int client_mount(void *input){
    size_t offset = sizeof(int);
    // reads the client's pathname from the server's pipe
    char client_pipe_path[MAX_SIZE_PATHNAME];
    buffer_read_char(input, offset, client_pipe_path, MAX_SIZE_PATHNAME);
    offset += sizeof(char) * (MAX_SIZE_PATHNAME - 1);

    // opens client's pipe for writting
    int client_pipe = pipe_open(client_pipe_path, O_WRONLY);
    if (client_pipe == -1)
        return -1;

    // tries to save the client's pipe
    int session_id = add_to_session_table(client_pipe, client_pipe_path);
    if (session_id == -1){
        // the aren't more sessions avaiable
        pipe_write_int(client_pipe, -1);
        return -1;
    }

    // writes to the client the amount of bytes read
    if (pipe_write_int(client_pipe, session_id) == -1)
        return -1;

    // returns the client's pipe
    return 0;
}

// TODO: #7 o que fazer no caso de erro?
int client_unmount(int session_id){
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

// char name[MAX_SIZE_PATHNAME]; inf flags;
int client_open(int session_id, void *input){
    size_t offset = sizeof(int);
    printf("%d\t", session_id);

    char name[MAX_SIZE_PATHNAME];
    buffer_read_char(input, offset, name, MAX_SIZE_PATHNAME);
    offset += sizeof(char) * (MAX_SIZE_PATHNAME - 1);
    printf("%s\t", name);

    int flags = buffer_read_int(input, offset);
    offset += sizeof(int);
    printf("%d\n", flags);

    int fhandle = tfs_open(name, flags);

    // writes to the client SUCCESS!
    int client_pipe = get_phandle_from_session_table(session_id);

    // write the server's response
    pipe_write_int(client_pipe, fhandle);
    return 0;
}

int client_close(int session_id, void *input){
    size_t offset = sizeof(int);
    printf("%d\t", session_id);

    int fhandle = buffer_read_int(input, offset);
    offset += sizeof(int);
    printf("%d\n", fhandle);

    int sucess = tfs_close(fhandle);

    // writes to the client SUCCESS!
    int client_pipe = get_phandle_from_session_table(session_id);

    // write the server's response
    pipe_write_int(client_pipe, sucess);
    
    return 0;
}

int client_write(int session_id, void *input){
    size_t offset = sizeof(int);
    printf("%d\t", session_id);

    int fhandle = buffer_read_int(input, offset);
    offset += sizeof(int);
    printf("%d\t", fhandle);

    size_t len = buffer_read_size_t(input, offset);
    offset += sizeof(size_t);

    printf("%ld\t", len);

    char buffer[len];
    buffer_read_char(input, offset, buffer, len);
    offset += sizeof(char) * (MAX_SIZE_PATHNAME - 1);
    buffer[len] = 0;
    printf("%s\n", buffer);

    ssize_t res = tfs_write(fhandle, buffer, len);
    res--;

    // writes to the client SUCCESS!
    int client_pipe = get_phandle_from_session_table(session_id);

    // write the server's response
    pipe_write_ssize_t(client_pipe, res);

    return 0;
}

int client_read(int session_id, void *input){
    size_t offset = sizeof(int); 
    printf("%d\t", session_id);

    int fhandle = buffer_read_int(input, offset);
    offset += sizeof(int);
    printf("%d\t", fhandle);

    size_t len = buffer_read_size_t(input, offset);
    offset += sizeof(size_t);
    printf("%ld\n", len);

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

int client_destroy(int session_id){
    // destroys the server
    /*if (server_destroy() == -1)   FIXME
        return -1;*/

    // closes the client's pipe
    if (client_unmount(session_id) == -1)
        return -1;

    printf("[INFO] server destroyed\n");

    return 0;
}
