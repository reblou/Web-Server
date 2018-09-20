#include <stdio.h>
#include <pthread.h>

void *thread_code(void *arg);
void *increment(void *arg);

int val = 0;

int main()
{
    pthread_t threads[10];

    int i;
    for(i =0; i<10; i++) {
        pthread_create(&threads[i], NULL, &increment, (void *) &i);
    }

    void *retval = NULL;

    for( i=0; i < 10; i++) {
        pthread_join(threads[i], &retval);
        printf("joined: %d\n", *((int *) retval));
    }

    printf("final val: %d\n", val);
}

void *thread_code(void *arg) {
    printf("thread %d\n", *((int *) arg));
    return arg;
}

void *increment(void *arg) {
    int *id = (int *) arg;

    val++;

    printf("thread: %d, val: %d\n", *id, val);
    return arg;
}
