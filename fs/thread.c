#include "thread.h"

/* atualizar status e o parametro do session */
void thread_status(int session_id, int status, void *buffer){
    session *s = get_session(session_id);
    s->status = status;
    s->buffer = buffer;

    pthread_mutex_lock(&s->lock);
    pthread_cond_signal(&s->cond);
	pthread_mutex_unlock(&s->lock);
}

void *thread_execute(void *arg){
    int session_id = buffer_read_int(arg, 0);

    session *s = get_session(session_id);
	while (1){	
		pthread_mutex_lock(&s->lock);
		while (s->status == THREAD_STATUS_SLEEP){
			pthread_cond_wait(&s->cond, &s->lock);
		}
		pthread_mutex_unlock(&s->lock);

		if (s->status == THREAD_STATUS_ERROR){
			session_destroy(session_id);
			return NULL; // add code if needed, else combine w/ next if
		}

		if (s->status == THREAD_STATUS_DESTROY){
			session_destroy(session_id);
			return NULL;
		}

		//thread is active
		while (s->status == THREAD_STATUS_ACTIVE){
			int op_code = buffer_read_int(s->buffer, 0);
			switch(op_code){

			case TFS_OP_CODE_MOUNT:
				printf("THREAD CASE 1\n");
				client_mount(s->buffer);
				break;

			case TFS_OP_CODE_UNMOUNT:
				printf("THREAD CASE 2\n");
				client_unmount(s->session_id);
				break;

			case TFS_OP_CODE_OPEN:
				printf("THREAD CASE 3\n");
				client_open(s->session_id, s->buffer);
				break;

			case TFS_OP_CODE_CLOSE:
				printf("THREAD CASE 4\n");
				client_close(s->session_id, s->buffer);
				break;

			case TFS_OP_CODE_WRITE:
				printf("THREAD CASE 5\n");
				client_write(s->session_id, s->buffer);
				break;

			case TFS_OP_CODE_READ:
				printf("THREAD CASE 6\n");
				client_read(s->session_id, s->buffer);
				break;

			case TFS_OP_CODE_SHUTDOWN_AFTER_ALL_CLOSED:
				printf("THREAD CASE 7\n");
				client_destroy(s->session_id);
				break;

			default:
				return NULL;
				break;
			}
			pthread_mutex_lock(&s->lock);
			pthread_cond_wait(&s->cond, &s->lock);
			pthread_mutex_unlock(&s->lock);
		}
	}

	return NULL;
}

//thread_status