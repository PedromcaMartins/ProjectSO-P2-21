#include "operations.h"
#include "open_pipe.h"
#include "common/pipe_control_functions.h"

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

#define MAX_BUFFER_SIZE 1024
#define MAX_SIZE_PATHNAME 40

// variaveis globais
int server_pipe;
char *server_pipename;
bool server_status = true;

// prototipos
int server_init(char const *server_pipe_path);
int server_destroy();
int decode();

int client_mount();
int client_unmount(int session_id);
int client_open(int session_id);
int client_close(int session_id);
int client_write(int session_id);
int client_read(int session_id);
int client_destroy(int session_id);


int main(int argc, char **argv) {

    if (argc < 2) {
        printf("Please specify the pathname of the server's pipe.\n");
        return 1;
    }

    server_pipename = argv[1];
    printf("Starting TecnicoFS server with pipe called %s\n", server_pipename);

    
    //initializes the server
    assert(server_init(server_pipename) != -1);

    while (decode() != -1 && server_status == true) { sleep(1); }

    printf("[INFO] Server destroyed\n");

    return 0;
}



int server_init(char const *server_pipe_path) {
    // creates open pipe table
    open_pipe_table_init();
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
    // TODO: add functionality to wait until all pipes are closed except one
    // TODO: every error until here is reported back to the client w/ -1

    server_status = false;

    return tfs_destroy_after_all_closed();
}

int client_destroy(int session_id){
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

int client_mount(){
    // reads the client's pathname from the server's pipe
    char client_pipe_path[MAX_SIZE_PATHNAME];
    ssize_t ret = pipe_read(server_pipe, client_pipe_path, MAX_SIZE_PATHNAME);
    // validates it
    if (ret == -1){
        return -1;
    }

    // opens client's pipe for writting
    int client_pipe = pipe_open(client_pipe_path, O_WRONLY);
    if (client_pipe == -1)
        return -1;

    // tries to save the client's pipe
    int session_id = add_to_open_pipe_table(client_pipe, client_pipe_path);
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
    int client_pipe = get_phandle_from_open_pipe_table(session_id);
    char *client_pathname = get_pathname_from_open_pipe_table(session_id);

    // unregisters the client's session_id
    if (remove_from_open_pipe_table(session_id) == -1){
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

int client_open(int session_id){
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
    int client_pipe = get_phandle_from_open_pipe_table(session_id);

    // write the server's response
    pipe_write_int(client_pipe, fhandle);
    return 0;
}

int client_close(int session_id){
    printf("%d\t", session_id);

    int fhandle = pipe_read_int(server_pipe);
    printf("%d\n", fhandle);

    int sucess = tfs_close(fhandle);

    // writes to the client SUCCESS!
    int client_pipe = get_phandle_from_open_pipe_table(session_id);

    // write the server's response
    pipe_write_int(client_pipe, sucess);
    
    return 0;
}

int client_write(int session_id){
    printf("%d\t", session_id);

    int fhandle = pipe_read_int(server_pipe);
    printf("%d\t", fhandle);

    size_t len = pipe_read_size_t(server_pipe);
    printf("%ld\t", len);

    char buffer[len];
    ssize_t ret = pipe_read(server_pipe, buffer, len);
    if (ret == 0)
        return -1;
    ret++;
    printf("%s\n", buffer);

    ssize_t res = tfs_write(fhandle, buffer, len);

    // writes to the client SUCCESS!
    int client_pipe = get_phandle_from_open_pipe_table(session_id);

    // write the server's response
    pipe_write_ssize_t(client_pipe, res);
    
    return 0;
}

int client_read(int session_id){
    printf("%d\t", session_id);

    int fhandle = pipe_read_int(server_pipe);
    printf("%d\t", fhandle);

    size_t len = pipe_read_size_t(server_pipe);
    printf("%ld\n", len);

    // TODO: adicinoar funcao tfs_read();
    char buffer[len];
    ssize_t res = tfs_read(fhandle, buffer, len);

    // writes to the client SUCCESS!
    int client_pipe = get_phandle_from_open_pipe_table(session_id);

    // write the server's response
    pipe_write_ssize_t(client_pipe, res);
    if (res == -1)
        return -1;
    pipe_write(client_pipe, buffer, (size_t)(res));
    
    return 0;
}

int decode(){
    int session_id;
    char buffer[10];
    pipe_read(server_pipe, buffer, sizeof(char)+1);
    int command = atoi(buffer);

    switch (command){
    case TFS_OP_CODE_MOUNT:
        printf("CASE 1\n");
        client_mount();
        break;

    case TFS_OP_CODE_UNMOUNT:
        printf("CASE 2\n");
        session_id = pipe_read_int(server_pipe);
        if (session_id == -1) // rebentar o server
            server_destroy();
        client_unmount(session_id);
        break;

    case TFS_OP_CODE_OPEN:
        printf("CASE 3\n");
        session_id = pipe_read_int(server_pipe);
        if (session_id == -1) // rebentar o server
            server_destroy();
        client_open(session_id);
        break;

    case TFS_OP_CODE_CLOSE:
        printf("CASE 4\n");
        session_id = pipe_read_int(server_pipe);
        if (session_id == -1) // rebentar o server
            server_destroy();
        client_close(session_id);
        break;

    case TFS_OP_CODE_WRITE:
        printf("CASE 5\n");
        session_id = pipe_read_int(server_pipe);
        if (session_id == -1) // rebentar o server
            server_destroy();
        client_write(session_id);
        break;

    case TFS_OP_CODE_READ:
        printf("CASE 6\n");
        session_id = pipe_read_int(server_pipe);
        if (session_id == -1) // rebentar o server
            server_destroy();
        client_read(session_id);
        break;

    case TFS_OP_CODE_SHUTDOWN_AFTER_ALL_CLOSED:
        printf("CASE 7\n");
        session_id = pipe_read_int(server_pipe);
        if (session_id == -1) // rebentar o server
            server_destroy();
        client_destroy(session_id);
        break;

    default:
        return -1;
        break;
    }

    return 0;
}
