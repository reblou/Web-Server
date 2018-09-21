#include <stdio.h>
#include <pthread.h>

void *thread_code(void *arg);
void *increment(void *arg);

int val = 0;
pthread_mutex_t val_lock;
#define NUM_THREADS 10

int main()
{
    pthread_t threads[NUM_THREADS];
    pthread_mutex_init(&val_lock, NULL);

    int i;
    for(i =0; i<NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, &increment, (void *) &i);
    }

    void *retval = NULL;

    for( i=0; i < 10; i++) {
        pthread_join(threads[i], &retval);
        printf("joined: %d\n", *((int *) retval));
    }
    pthread_mutex_destroy(&val_lock);

    printf("final val: %d\n", val);
}

void *thread_code(void *arg) {
    printf("thread %d\n", *((int *) arg));
    return arg;
}

void *increment(void *arg) {
    int *id = (int *) arg;
    pthread_mutex_lock(&val_lock);
    val++;
    printf("thread: %d, val: %d\n", *id, val);
    pthread_mutex_unlock(&val_lock);
    return arg;
}
