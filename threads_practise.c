#include <stdio.h>
#include <pthread.h>

void *thread_code(void *arg);

int main()
{
    pthread_t threads[10];

    int i;
    for(i =0; i<10; i++) {
        pthread_create(&threads[i], NULL, &thread_code, (void *) &i);
    }

    void *retval = NULL;

    for( i=0; i < 10; i++) {
        pthread_join(threads[i], &retval);
        printf("joined: %d\n", *((int *) retval));
    }
}

void *thread_code(void *arg) {
    printf("thread %d\n", *((int *) arg));
    return arg;
}
