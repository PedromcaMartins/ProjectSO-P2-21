#ifndef SESSION_H
#define SESSION_H

#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>

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
    int session_id;
    int fhandle;
    int status;
    void *buffer;
    pthread_mutex_t lock;
    pthread_cond_t cond; // utilizar o write_to_buffer pra escrever thread_*(int)(session)*

} session;

//prototypes
bool valid_session(int pipe_file_handle);
void session_table_init();
int add_to_session_table(int client_pipe, char *client_pipe_path);
int remove_from_session_table(int session_id);
int get_phandle_from_session_table(int session_id);
char* get_pathname_from_session_table(int session_id);


#endif // SESSION_H