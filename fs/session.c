#include "session.h"

// global variables
// session_id corresponds to the index of said client's pipe on the table
static session session_table[MAX_SESSION_IDS];
static bool free_session_table[MAX_SESSION_IDS];

// functions
bool valid_session(int pipe_phandle) {
    return pipe_phandle >= 0 && pipe_phandle < MAX_SESSION_IDS;
}

void session_table_init(){
    for (int i = 0; i < MAX_SESSION_IDS; i++) {
        free_session_table[i] = FREE;
    }
}

int add_to_session_table(int client_pipe, char *client_pipe_path){
    for (int i = 0; i < MAX_SESSION_IDS; i++) {
        if (free_session_table[i] == FREE){
            // adds client pipe to the table
            session_table[i].client_pipe = client_pipe;

            // adds client pipe path to the table
            session_table[i].client_pipe_path = client_pipe_path;

            // sets the session as taken
            free_session_table[i] = TAKEN;

            return i;
        }

    }
    return -1;
}

int remove_from_session_table(int session_id){
    if (!valid_session(session_id) || free_session_table[session_id] != TAKEN)
        return -1;

    free_session_table[session_id] = FREE;

    return 0;
}

int get_phandle_from_session_table(int session_id){
    if (!valid_session(session_id)) {
        return -1;
    }
    return session_table[session_id].client_pipe;
}

char* get_pathname_from_session_table(int session_id){
    if (!valid_session(session_id)) {
        return NULL;
    }
    return session_table[session_id].client_pipe_path;
}
