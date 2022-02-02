#include "client/tecnicofs_client_api.h"

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

#define CLIENT_PATHNAME "/tmp/acks-pipe"
#define MAX_SIZE_PATHNAME 40

int main(int argc, char **argv) {

    if (argc < 2) {
        printf("Please specify the pathname of the server's pipe.\n");
        return 1;
    }

    char *server_pathname = argv[1];
    printf("Starting TecnicoFS server with pipe called %s\n", server_pathname);

    char client_pathname[MAX_SIZE_PATHNAME] = CLIENT_PATHNAME;

    assert(tfs_mount(client_pathname, server_pathname) != -1);

    char file_pathname[MAX_SIZE_PATHNAME] = "/f1";
    char message[MAX_SIZE_PATHNAME] = "Mensagem teste :(";
    int fhandle = tfs_open(file_pathname, 1);
    assert(fhandle != -1);

    ssize_t res = tfs_write(fhandle, message, MAX_SIZE_PATHNAME);
    printf("result: %ld\n", res);

    assert(res == MAX_SIZE_PATHNAME);

    assert(tfs_read(fhandle, message, MAX_SIZE_PATHNAME) == MAX_SIZE_PATHNAME);

    assert(tfs_close(fhandle) != -1);

    sleep(2);

    assert(tfs_shutdown_after_all_closed() != -1);
}