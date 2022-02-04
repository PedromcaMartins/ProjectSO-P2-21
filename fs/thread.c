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
	pthread_mutex_lock(&s->lock);

	while (1){	
		while (s->status == THREAD_STATUS_SLEEP){
			pthread_cond_wait(&s->cond, &s->lock);
		}

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
				printf("THREAD %d CASE 1\n", s->session_id);
				client_mount(s->session_id, s->buffer);
				break;

			case TFS_OP_CODE_UNMOUNT:
				printf("THREAD %d CASE 2\n", s->session_id);
				pthread_mutex_unlock(&s->lock);
				client_unmount(s->session_id);
				break;

			case TFS_OP_CODE_OPEN:
				printf("THREAD %d CASE 3\n", s->session_id);
				client_open(s->session_id, s->buffer);
				break;

			case TFS_OP_CODE_CLOSE:
				printf("THREAD %d CASE 4\n", s->session_id);
				client_close(s->session_id, s->buffer);
				break;

			case TFS_OP_CODE_WRITE:
				printf("THREAD %d CASE 5\n", s->session_id);
				client_write(s->session_id, s->buffer);
				break;

			case TFS_OP_CODE_READ:
				printf("THREAD %d CASE 6\n", s->session_id);
				client_read(s->session_id, s->buffer);
				break;

			case TFS_OP_CODE_SHUTDOWN_AFTER_ALL_CLOSED:
				printf("THREAD %d CASE 7\n", s->session_id);
				pthread_mutex_unlock(&s->lock);
				client_destroy(s->session_id);
				break;

			default:
				return NULL;
				break;
			}

			if (s->status != THREAD_STATUS_ACTIVE)
				break;

			pthread_cond_wait(&s->cond, &s->lock);
			pthread_mutex_unlock(&s->lock);
			pthread_mutex_lock(&s->lock);
		}
	}

	return NULL;
}

//thread_status