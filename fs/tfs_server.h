#ifndef TFS_SERVER_H
#define TFS_SERVER_H

#include "operations.h"
#include "session.h"
#include "common/pipe_control_functions.h"
#include "common/common.h"
#include "thread.h"

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

// variaveis globais
int server_pipe;
char *server_pipename;
bool server_status = true;

// prototipos
int server_init(char const *server_pipe_path);
int server_destroy();
int decode();

#endif //TFS_SERVER_H