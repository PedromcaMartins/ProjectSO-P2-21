#ifndef COMMON_H
#define COMMON_H

#define MAX_BUFFER_SIZE 2048

/* tfs_open flags */
enum {
    TFS_O_CREAT = 0b001,
    TFS_O_TRUNC = 0b010,
    TFS_O_APPEND = 0b100,
};

/* thread states */
enum {
    THREAD_STATUS_SLEEP = 0b001,
    THREAD_STATUS_ACTIVE = 0b010,
    THREAD_STATUS_ERROR = 0b011,
    THREAD_STATUS_DESTROY = 0b100,
};

/* operation codes (for client-server requests) */
enum {
    TFS_OP_CODE_MOUNT = 1,
    TFS_OP_CODE_UNMOUNT = 2,
    TFS_OP_CODE_OPEN = 3,
    TFS_OP_CODE_CLOSE = 4,
    TFS_OP_CODE_WRITE = 5,
    TFS_OP_CODE_READ = 6,
    TFS_OP_CODE_SHUTDOWN_AFTER_ALL_CLOSED = 7,
    TFS_OP_CODE_SERVER_PIPE_CLOSED = 0,
};

#endif /* COMMON_H */