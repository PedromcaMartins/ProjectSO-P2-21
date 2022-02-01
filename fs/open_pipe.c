#include "open_pipe.h"

// global variables
// session_id corresponds to the index of said client's pipe on the table
static session open_pipe_table[MAX_SESSION_IDS];
static bool free_open_pipe_table[MAX_SESSION_IDS];

// functions
bool valid_open_pipe(int pipe_file_handle) {
    return pipe_file_handle >= 0 && pipe_file_handle < MAX_SESSION_IDS;
}

void open_pipe_table_init(){
    for (int i = 0; i < MAX_SESSION_IDS; i++) {
        free_open_pipe_table[i] = FREE;
    }
}

int add_to_open_pipe_table(int client_pipe, char *client_pipe_path){
    for (int i = 0; i < MAX_SESSION_IDS; i++) {
        if (free_open_pipe_table[i] == FREE){
            // adds client pipe to the table
            open_pipe_table[i].client_pipe = client_pipe;

            // adds client pipe path to the table
            open_pipe_table[i].client_pipe_path = client_pipe_path;

            // sets the session as taken
            free_open_pipe_table[i] = TAKEN;

            return i;
        }

    }
    return -1;
}

int remove_from_open_pipe_table(int session_id){
    if (!valid_open_pipe(session_id) || free_open_pipe_table[session_id] != TAKEN)
        return -1;

    free_open_pipe_table[session_id] = FREE;

    return 0;
}

int get_phandle_from_open_pipe_table(int session_id){
    if (!valid_open_pipe(session_id)) {
        return -1;
    }
    return open_pipe_table[session_id].client_pipe;
}

char* get_pathname_from_open_pipe_table(int session_id){
    if (!valid_open_pipe(session_id)) {
        return NULL;
    }
    return open_pipe_table[session_id].client_pipe_path;
}
