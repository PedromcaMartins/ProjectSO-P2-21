#include "fs/operations.h"
#include <assert.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

/*  Simple test to check whether the implementation of
    tfs_destroy_after_all_closed is correct.
    Note: This test uses TecnicoFS as a library, not
    as a standalone server.
    We recommend trying more elaborate tests of tfs_destroy_after_all_closed.
    Also, we suggest trying out a similar test once the
    client-server version is ready (calling the tfs_shutdown_after_all_closed 
    operation).
*/

#define NUM_THREADS 50

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int closed_file = 0;

void *fn_thread(void *arg) {
    int index = *((int *)arg);
    printf("THREAD %d: Started.\n", index);

    int f = tfs_open("/f1", TFS_O_CREAT);
    if (f == -1)
        return NULL;

    /* set *before* closing the file, so that it is set before
       tfs_destroy_after_all_close returns in the main thread
    */

    assert(tfs_close(f) != -1);

    printf("THREAD %d: Ended.\n", index);
    return NULL;
}

int main() {
    pthread_t tid[NUM_THREADS];
    int thread_args[NUM_THREADS];

    assert(tfs_init() != -1);

    int i;
    for (i = 0; i < NUM_THREADS; i++){
        printf("IN MAIN: Creating thread %d.\n", i);
        thread_args[i] = i;
        assert(pthread_create(&tid[i], NULL, fn_thread, &thread_args[i]) == 0);
    }

    printf("IN MAIN: All threads are created.\n");

    assert(tfs_destroy_after_all_closed() != -1);
    printf("Todas as threads fecharam os ficheiros\n");

    pthread_mutex_destroy(&lock);

    // No need to join thread
    printf("Successful test.\n");

    return 0;
}
