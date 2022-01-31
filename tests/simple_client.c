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

#define SERVER_PATHNAME "/tmp/server"
#define CLIENT_PATHNAME "/tmp/acks-pipe"
#define MAX_SIZE_PATHNAME 40

int main() {
    char client_pathname[MAX_SIZE_PATHNAME] = CLIENT_PATHNAME;
    char server_pathname[MAX_SIZE_PATHNAME] = SERVER_PATHNAME;

    assert(tfs_mount(client_pathname, server_pathname) != -1);

    char file_pathname[MAX_SIZE_PATHNAME] = "/f1";
    char message[MAX_SIZE_BUFFER] = "Mensagem teste :(";
    int fhandle = tfs_open(file_pathname, 1);
    assert(fhandle != -1);

    assert(tfs_write(fhandle, message, MAX_SIZE_BUFFER) == MAX_SIZE_BUFFER);

    assert(tfs_read(fhandle, message, MAX_SIZE_BUFFER) == MAX_SIZE_BUFFER);

    assert(tfs_close(fhandle) != -1);

    sleep(2);

    assert(tfs_shutdown_after_all_closed() != -1);
}
