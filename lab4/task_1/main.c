#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

typedef struct {
    char *title;
    int index;
} BookData;

void* fetch_book_status(void* arg) {
    BookData *data = (BookData*)arg;
    
    sleep(2); 
    
    char *statuses[] = {"AVAILABLE", "PRE_ORDER", "UNAVAILABLE"};
    int status_idx = data->index % 3;
    
    printf("%s - %s\n", data->title, statuses[status_idx]);
    
    return NULL;
}

int main() {
    char *titles[] = {
        "Harry Potter and the Philosopher's Stone",
        "Harry Potter and the Chamber of Secrets",
        "Harry Potter and the Prisoner of Azkaban",
        "Harry Potter and the Goblet of Fire",
        "Harry Potter and the Half-Blood Prince",
        "Harry Potter and the Deathly Hallows"
    };
    
    int num_books = 6;
    pthread_t threads[num_books];
    BookData book_data[num_books];
    
    long start_time = time(NULL);
    
    for (int i = 0; i < num_books; i++) {
        book_data[i].title = titles[i];
        book_data[i].index = i;
        pthread_create(&threads[i], NULL, fetch_book_status, &book_data[i]);
    }
    
    for (int i = 0; i < num_books; i++) {
        pthread_join(threads[i], NULL);
    }
    
    long end_time = time(NULL);
    printf("\nTime elapsed: %ld seconds\n", end_time - start_time);
    
    return 0;
}
