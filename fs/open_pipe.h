#ifndef OPEN_PIPE_H
#define OPEN_PIPE_H

#include <stdbool.h>
#include <stdlib.h>

#define MAX_SESSION_IDS 20
#define FREE 1
#define TAKEN 0

// struct
/*
 * Open pipe entry (in open pipe table)
 */
typedef struct {
    int client_pipe;
    char *client_pipe_path;
} session;

//prototypes
bool valid_open_pipe(int pipe_file_handle);
void open_pipe_table_init();
int add_to_open_pipe_table(int client_pipe, char *client_pipe_path);
int remove_from_open_pipe_table(int session_id);
int get_phandle_from_open_pipe_table(int session_id);
char* get_pathname_from_open_pipe_table(int session_id);


#endif // OPEN_PIPE_H