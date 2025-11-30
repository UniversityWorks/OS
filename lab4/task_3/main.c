#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

typedef struct Message {
    char *content;
    struct Message *next;
} Message;

typedef struct {
    Message *head;
    Message *tail;
    int size;
    int max_size;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} MessageQueue;

void queue_init(MessageQueue *queue, int max_size) {
    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
    queue->max_size = max_size;
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->not_empty, NULL);
    pthread_cond_init(&queue->not_full, NULL);
}

void queue_add(MessageQueue *queue, const char *content) {
    pthread_mutex_lock(&queue->mutex);
    
    while (queue->size >= queue->max_size) {
        pthread_cond_wait(&queue->not_full, &queue->mutex);
    }
    
    Message *msg = (Message*)malloc(sizeof(Message));
    msg->content = strdup(content);
    msg->next = NULL;
    
    if (queue->tail == NULL) {
        queue->head = msg;
        queue->tail = msg;
    } else {
        queue->tail->next = msg;
        queue->tail = msg;
    }
    
    queue->size++;
    
    pthread_cond_signal(&queue->not_empty);
    
    pthread_mutex_unlock(&queue->mutex);
}

/*(FIFO)*/
char* queue_poll(MessageQueue *queue) {
    pthread_mutex_lock(&queue->mutex);
    
    while (queue->size == 0) {
        pthread_cond_wait(&queue->not_empty, &queue->mutex);
    }
    
    Message *msg = queue->head;
    char *content = msg->content;
    
    queue->head = msg->next;
    if (queue->head == NULL) {
        queue->tail = NULL;
    }
    
    queue->size--;
    
    pthread_cond_signal(&queue->not_full);
    
    pthread_mutex_unlock(&queue->mutex);
    
    free(msg);
    return content;
}

void queue_destroy(MessageQueue *queue) {
    pthread_mutex_lock(&queue->mutex);
    
    Message *current = queue->head;
    while (current != NULL) {
        Message *next = current->next;
        free(current->content);
        free(current);
        current = next;
    }
    
    pthread_mutex_unlock(&queue->mutex);
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->not_empty);
    pthread_cond_destroy(&queue->not_full);
}


MessageQueue queue;

void* producer(void *arg) {
    int id = *(int*)arg;
    
    for (int i = 1; i <= 5; i++) {
        char message[100];
        sprintf(message, "Message %d from Producer %d", i, id);
        
        printf("[Producer %d] Adding: %s\n", id, message);
        queue_add(&queue, message);
        
        usleep(100000); 
    }
    
    return NULL;
}

void* consumer(void *arg) {
    int id = *(int*)arg;
    
    for (int i = 0; i < 5; i++) {
        char *message = queue_poll(&queue);
        printf("[Consumer %d] Got: %s\n", id, message);
        free(message);
        
        usleep(150000);
    }
    
    return NULL;
}

int main() {
    
    queue_init(&queue, 3);
    
    pthread_t producers[2];
    pthread_t consumers[2];
    int producer_ids[] = {1, 2};
    int consumer_ids[] = {1, 2};
    
    for (int i = 0; i < 2; i++) {pthread_create(&producers[i], NULL, producer, &producer_ids[i]);}
    for (int i = 0; i < 2; i++) { pthread_create(&consumers[i], NULL, consumer, &consumer_ids[i]);}
    for (int i = 0; i < 2; i++) {pthread_join(producers[i], NULL);}
    for (int i = 0; i < 2; i++) { pthread_join(consumers[i], NULL);}
    
    queue_destroy(&queue);
    
    
    return 0;
}
