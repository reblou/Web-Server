#include "dispatch.h"

#include <pcap.h>

#include "analysis.h"
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include <netinet/ip.h>
#include <netinet/tcp.h>

#include <netinet/if_ether.h>

#define NUM_THREADS 4

/*
 * Struct to contain all parameters to dispatch
 */
struct packet {
    struct pcap_pkthdr *header;
    unsigned char *packet;
    int verbose;
};

/*
 * Linked list element
 */
struct element {
    struct element *next;
    struct packet *pack;
};

/*
 * Linked List Struct
 */
struct linked_list {
    struct element *head;
};



pthread_t threads[NUM_THREADS];
// Indicates whether threads should continue to check queue or return
int threadscont = 1;

struct linked_list ll = {NULL};

// Mutex lock for opertaions on the global linked list used as a thread pool queue
pthread_mutex_t queuelock = PTHREAD_MUTEX_INITIALIZER;

void create_threads();
void thread_code(void *arg);
void sig_handler(int signo);
void append_pack(struct packet *p);
struct packet *take_pack();

void dispatch(struct pcap_pkthdr *header,
        const unsigned char *packet,
        int verbose) {

    // Check for termination signal
    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        printf("can't catch SIGINT\n");
    }
    int pckt_len = header->len;
    
    // Allocate memory for packet data on the heap
    unsigned char *permpackt = malloc(pckt_len + 1);
    // Allocate memory for the header data
    struct pcap_pkthdr *hd = malloc(sizeof(struct pcap_pkthdr));
    
    // Copy header and packet data onto the heap
    memcpy(permpackt, packet, pckt_len);
    memcpy(hd, header, sizeof(struct pcap_pkthdr));

    // Allocate memory for container packet struct to be added to linked list
    struct packet *p = malloc(sizeof(struct packet)+(pckt_len + 1));
    p->header = hd;
    p->packet = permpackt;
    p->verbose = verbose;

    pthread_mutex_lock(&queuelock);
    append_pack(p);
    pthread_mutex_unlock(&queuelock);
}

/*
 * Called in the case of a termination signal. Calls the funtion to print
 * the report, tells the threads to stop processing new packets, and rejoins
 * the threads. 
 */
void sig_handler(int signo) 
{
    switch (signo) {
        case SIGINT:
            printf("\nreceived SIGINT\n");
            print_report();
            break;
        default:
            printf("received unknown signal\n");
            break;
    }
    threadscont = 0; /* Tells threads to stop checking for new packets */
    
    int i;
    for (i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }
    
    exit(EXIT_SUCCESS);
}

/*
 * Creates a certain number of threads and starts them executing thread_code
 */
void create_threads()
{
    int i;
    for (i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, &thread_code, (void *) NULL);
    }
}

/*
 * Threads check the linked list for packets and pass them to analyse. Once
 * a packet has been processed the memory allocated on the heap to store them
 * in the linked list is freed.
 */
void thread_code(void *arg)
{
    struct packet *p;
    while (threadscont) {
        pthread_mutex_lock(&queuelock);

        p = take_pack();
        // If there are no packets waiting to be processed
        if (p == NULL) { 
            pthread_mutex_unlock(&queuelock);
            continue;
        }

        pthread_mutex_unlock(&queuelock);
        analyse(p->header, p->packet, p->verbose);

        free(p->packet);
        free(p->header);
        free(p);
    }
    return;
}

/*
 * Adds a packet struct to the end of the linked list
 */
void append_pack(struct packet *p) 
{
    struct element *elem = malloc(sizeof(struct element));
    elem->pack = p;
    elem->next = NULL;

    if (ll.head == NULL) {
        ll.head = elem;
    } else {
        struct element *tail = ll.head;
        while (tail->next != NULL) {
            tail = tail->next;
        }
        tail->next = elem;
    }
}

/*
 * Returns a pointer to the first element in the linked list
 */
struct packet *take_pack()
{
    if (ll.head != NULL) {
        struct element *next = (ll.head)->next;
        struct packet *p = ll.head->pack;
        free(ll.head); // Free the elements memory allocated on the heap
        ll.head = next;
        return p;
    }
    return NULL; // If the list is empty
}
