
#ifndef EVENT_LOOP
#define EVENT_LOOP
#include <pthread.h>

typedef void (*event_cbk)(void*);
typedef struct task_{
    event_cbk cbk;
    void *arg;
    struct task_ *left, *right;
}task_t;

typedef enum{
    EV_LOOP_IDLE,
    EV_LOOP_BUSY
}EV_LOOP_STATE;

typedef struct event_loop{
    struct task_ *task_array_head;
    pthread_mutex_t ev_loop_mutex;
    EV_LOOP_STATE ev_loop_state;
    pthread_cond_t ev_loop_cv;
    pthread_t *thread;
    struct task_ *current_task;
}event_loop_t;

void event_loop_init(event_loop_t *el);
void event_loop_run(event_loop_t *el);
task_t *task_create_new_job(event_loop_t *el, event_cbk cbk, void *arg);

#endif