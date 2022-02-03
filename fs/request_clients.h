#ifndef REQUEST_CLIENTS_H
#define REQUEST_CLIENTS_H

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

#include "session.h"
#include "common/pipe_control_functions.h"
#include "common/common.h"
#include "operations.h"

int client_mount(int session_id, void *input);
int client_unmount(int session_id);
int client_open(int session_id, void *input);
int client_close(int session_id, void *input);
int client_write(int session_id, void *input);
int client_read(int session_id, void *input);
int client_destroy(int session_id);

#endif // REQUEST_CLIENTS_H