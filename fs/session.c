#include "session.h"
#include "thread.h"

// global variables
// session_id corresponds to the index of said client's pipe on the table
static session session_table[MAX_SESSION_IDS];
static pthread_t thread_table[MAX_SESSION_IDS];
static bool free_session_table[MAX_SESSION_IDS];

pthread_mutex_t session_table_lock;

// functions
bool valid_session(int pipe_phandle) {
    return pipe_phandle >= 0 && pipe_phandle < MAX_SESSION_IDS;
}

void session_table_init(){
    void *input[sizeof(int)];

    for (int i = 0; i < MAX_SESSION_IDS; i++) {
        session_init(i);
        free_session_table[i] = FREE;

        printf("IN MAIN: Creating thread %d.\n", i);
        buffer_write_int(input, 0, i);
        pthread_create(&thread_table[i], NULL, thread_execute, input);
    }

    pthread_mutex_init(&session_table_lock, NULL);
}

void session_table_destroy(){
    for (int i = 0; i < MAX_SESSION_IDS; i++) {
        thread_status(i, THREAD_STATUS_DESTROY, NULL);
        pthread_join(thread_table[i], NULL);
        session_destroy(i);
    }

    pthread_mutex_destroy(&session_table_lock);
}

int add_to_session_table(){
    pthread_mutex_lock(&session_table_lock);
    for (int i = 0; i < MAX_SESSION_IDS; i++) {
        if (free_session_table[i] == FREE){

            // sets the session as taken
            free_session_table[i] = TAKEN;

            pthread_mutex_unlock(&session_table_lock);
            return i;
        }

    }

    pthread_mutex_unlock(&session_table_lock);
    return -1;
}

void add_to_session(int session_id, int client_pipe, char *client_pipe_path){
    // adds client pipe to the table
    session_table[session_id].client_pipe = client_pipe;

    // adds client pipe path to the table
    session_table[session_id].client_pipe_path = client_pipe_path;


}

int remove_from_session_table(int session_id){
    if (!valid_session(session_id) || free_session_table[session_id] != TAKEN)
        return -1;

    free_session_table[session_id] = FREE;

    // thread is set to sleep();
    thread_status(session_id, THREAD_STATUS_SLEEP, NULL);

    return 0;
}

session *get_session(int session_id){
    if (!valid_session(session_id)) {
        return NULL;
    }

    return &session_table[session_id];
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

void session_init(int session_id){
    if (!valid_session(session_id))
        return;

    session *s = &session_table[session_id];
    s->buffer = NULL;
    s->client_pipe = -1;
    s->client_pipe_path = NULL;
    s->fhandle = -1;
    s->status = THREAD_STATUS_SLEEP;
    s->session_id = session_id;
    pthread_mutex_init(&s->lock, NULL);
    pthread_cond_init(&s->cond, NULL);
}

void session_destroy(int session_id){
    if (!valid_session(session_id))
        return;

    session *s = &session_table[session_id];
    pthread_mutex_destroy(&s->lock);
    pthread_cond_destroy(&s->cond);
}
