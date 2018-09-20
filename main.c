#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

int bind_socket(int *sock);
int respond(char *file, int sockfd, FILE *in);
int read_input(FILE* in);
int get_file_size(int fd);
long thread_code(void *arg);

#define PORT 80

int main()
{
    int yes = 1;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes) == -1) {
        perror("setsockopt");
        exit(1);
        return -1;
    }

    bind_socket(&sockfd);

    listen(sockfd, SOMAXCONN);
    printf("Listening\n");

    int insockfd;

    while (1) {
        if ((insockfd = accept(sockfd, NULL, NULL)) == -1) {
            printf("accept error.\n");
            return -1;
        }
        printf("Accepted connection successfully.\n");

        FILE *in = fdopen(insockfd, "r+");
        read_input(in);

        char *file = "html/index.html";

        respond(file, insockfd, in);

        printf("Returing 0\n");

        close(insockfd);
    }
    return 0;
}

int bind_socket(int *sock) {
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(*sock, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1) {
        printf("bind error.\n");
        return -1;
    }
    printf("Socket bound successfully.\n");

}

int read_input(FILE* in) {
    char * str = NULL;
    size_t len = 0;

    printf("Reading\n");
    getline(&str, &len, in);
    while (str[0] != '\r' && str[0] != '\n')  {
        printf("%x, %s, %c\n", *str, str, str[0]);
        getline(&str, &len, in);
    }
    return 0;

}


int respond(char *file, int sockfd, FILE *in) {

    printf("Writing to output\n");

    //fprintf(in, "HTTP/1.1 200 OK\n");

    int fd = open(file, O_RDONLY, 0); // try to open the file
    printf("%s\n", file);

    int length;
    unsigned char* ptr = NULL;

    if( (length = get_file_size(fd)) == -1) {
        printf("failed getting resource file size");
        return -1;
    }
    if( (ptr = (unsigned char *) malloc(length)) == NULL) {
        printf("failed allocating memory for reading resource");
        printf("test");
        return -1;
    }
    printf("%d\n", length);
    read(fd, ptr, length); // read the file into memory
    send(sockfd, ptr, length, 0);  // send it to socket

    return 0;
}

int get_file_size(int fd) {
   struct stat stat_struct;

   if(fstat(fd, &stat_struct) == -1)
      return -1;
   return (int) stat_struct.st_size;
}
