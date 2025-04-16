#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "event_loop.h"

event_loop_t el;

int arr[] = {1, 2, 3, 4, 5};

typedef struct arg_obj_ {
    int *arr;
    int n;
}arg_obj_t;

EL_RES_T sum(void *arg) {
    arg_obj_t * inArg = (arg_obj_t*)arg;
    int n = inArg->n;
    int *arr = inArg->arr;
    int res = 0;
    for (int i = 0; i < n; i++) {
        res += arr[i];
    }
    printf("Sum: %d\n", res);
    return EL_CONTINUE;
}

int mul(void *arg) {
    arg_obj_t * inArg = (arg_obj_t*)arg;
    int n = inArg->n;
    int *arr = inArg->arr;
    int res = 1;
    for (int i = 0; i < n; i++) {
        res *= arr[i];
    }
    printf("Mul: %d\n", res);
    return EL_CONTINUE;
}

int main(int argc, char **argv){
    event_loop_init(&el);
    event_loop_run(&el);
    
    sleep(1);

    arg_obj_t *argObjSum = calloc(1, sizeof(arg_obj_t));
    argObjSum->arr = arr;
    argObjSum->n = sizeof(arr)/sizeof(arr[0]);
    task_t *task_sum = task_create_new_job(&el, sum, (void *)argObjSum);


    arg_obj_t *argObjMul = (arg_obj_t *)calloc(1, sizeof(arg_obj_t));
    argObjMul->arr = arr;
    argObjMul->n = sizeof(arr)/sizeof(arr[0]);
    task_t *task_mul = task_create_new_job(&el, mul, (void *)argObjMul);


    printf ("End of main\n");
    scanf("\n");
    return 0;
}