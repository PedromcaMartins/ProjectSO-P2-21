#ifndef THREAD_H
#define THREAD_H

#include "session.h"
#include "common/common.h"
#include "common/pipe_control_functions.h"
#include "request_clients.h"

void thread_status(int session_id, int status, void *buffer);

void *thread_execute(void *arg);

#endif // THREAD_H