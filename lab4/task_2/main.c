#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


typedef struct {
    pthread_mutex_t mutex;
    int executed;
} Once;

void once_init(Once *once) {
    pthread_mutex_init(&once->mutex, NULL);
    once->executed = 0;
}

void once_exec(Once *once, void (*func)(void*), void *arg) {
    pthread_mutex_lock(&once->mutex);
    
    if (!once->executed) {
        func(arg);
        once->executed = 1;
    }
    
    pthread_mutex_unlock(&once->mutex);
}

void once_destroy(Once *once) {
    pthread_mutex_destroy(&once->mutex);
}


void demo_function(void *arg) {
    printf("This function runs only once. (call #%d)\n", *(int*)arg);
}

int main() {
    
    Once once;
    once_init(&once);
    
    int call_numbers[] = {1, 2, 3, 4, 5};
    
    printf("Calling once_exec 5 times:\n");
    for (int i = 0; i < 5; i++) {
        printf("Call #%d: ", i + 1);
        once_exec(&once, demo_function, &call_numbers[i]);
    }
    
    once_destroy(&once);
    
    
    return 0;
}
