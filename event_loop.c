#include "event_loop.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>

bool static el_debug = false;

void event_loop_init(event_loop_t *el){
    el->task_array_head = NULL;
    pthread_mutex_init(&el->ev_loop_mutex, NULL);
    el->ev_loop_state = EV_LOOP_IDLE;
    pthread_cond_init(&el->ev_loop_cv, NULL);
    el->current_task = NULL;
}

static task_t *event_loop_get_next_task_to_run(event_loop_t *el){
    task_t *task;
    if (!el->task_array_head) return NULL;
    task = el->task_array_head;
    el->task_array_head = task->right;
    if (el->task_array_head) {
        el->task_array_head->left = NULL;
    }
    task->left = NULL;
    task->right = NULL;
    return task;
}

static void event_loop_add_task_in_task_array(event_loop_t *el, task_t *new_task){
    task_t *task, *prev_task;
    prev_task = NULL;
    task = el->task_array_head;
    while (task) {
        prev_task = task;
        task = task->right;
    }
    if (prev_task) {
        prev_task->right = new_task;
        new_task->left = prev_task;
    }
    else {
        el->task_array_head = new_task;
    }
}

static bool task_is_present_in_task_array(task_t *task){
    return !(task->left == NULL && task->right == NULL);
}

static void event_loop_schedule_task(event_loop_t *el, task_t *task){
    pthread_mutex_lock(&el->ev_loop_mutex);
    event_loop_add_task_in_task_array(el, task);
    if(el->ev_loop_state == EV_LOOP_BUSY){
        pthread_mutex_unlock(&el->ev_loop_mutex);
        return;
    }
    pthread_cond_signal(&el->ev_loop_cv);
    pthread_mutex_unlock(&el->ev_loop_mutex);
}

static void *event_loop_thread(void *arg){
    task_t *task;
    EL_RES_T res;
    event_loop_t *el = (event_loop_t*)arg;
    while(1){
        pthread_mutex_lock(&el->ev_loop_mutex);
        while((task = event_loop_get_next_task_to_run(el)) == NULL){
            if(el_debug){
                printf("Task array is empty, EL thread is suspending\n");
            }
            el->ev_loop_state = EV_LOOP_IDLE;
            pthread_cond_wait(&el->ev_loop_cv, &el->ev_loop_mutex);
        }
        el->ev_loop_state = EV_LOOP_BUSY;
        pthread_mutex_unlock(&el->ev_loop_mutex);
        if(el_debug){
            printf("EL thread woken up, firing the task\n");
        }
        el->current_task = task;
        res = task->cbk(task->arg);
        el->current_task = NULL;
        if (res == EL_CONTINUE) {
            event_loop_schedule_task(el, task);
        }
        else {
            free(task);
        }
    }
}

static void event_loop_remove_task_from_task_array(event_loop_t *el, task_t *task) {
    if (el->task_array_head == task) {
        el->task_array_head = task->right;
    }
    if(!task->left){
        if(task->right){
            task->right->left = NULL;
            task->right = 0;
            return;
        }
        return;
    }
    if(!task->right){
        task->left->right = NULL;
        task->left = NULL;
        return;
    }
    task->left->right = task->right;
    task->right->left = task->left;
    task->left = 0;
    task->right = 0;
}

task_t *task_create_new_job(event_loop_t *el, event_cbk cbk, void *arg){
    task_t *task = (task_t*)calloc(1, sizeof(task_t));
    task->cbk = cbk;
    task->arg = arg;
    task->left = NULL;
    task->right = NULL;
    event_loop_schedule_task(el, task);
    return task;
}

void event_loop_run(event_loop_t *el){
    pthread_attr_t attr;
    assert(el->thread == NULL);
    el->thread = (pthread_t*)calloc(1, sizeof(pthread_t));
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(el->thread, &attr, event_loop_thread, (void*)el);
}