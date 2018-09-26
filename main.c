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
int read_input(FILE* in, char *buffer);
int get_file_size(int fd);
long thread_code(void *arg);

#define PORT 80
#define HTML_ROOT "html"
#define DEFAULT_PAGE "/index.html"
#define NOT_FOUND "/404.html"

int main()
{
    int yes = 1;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // stops socket bind errors when running the program frequently
    if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes) == -1) {
        perror("setsockopt");
        exit(1);
    }

    bind_socket(&sockfd);

    listen(sockfd, SOMAXCONN);
    printf("Listening\n");

    int insockfd;

    // Main loop runs accpeting and replying to requests until program terminated
    while (1) {
        if ((insockfd = accept(sockfd, NULL, NULL)) == -1) {
            printf("accept error.\n");
            return -1;
        }
        printf("Accepted connection successfully.\n");

        FILE *in = fdopen(insockfd, "r+"); // open connection stream
        char page[500];
        read_input(in, page);
        printf("test\n");

        // copy requested filename from read_input into file
        char file[500];
        strcpy(file, HTML_ROOT);
        strcat(file, page);
        printf("file: %s\n", file);

        respond(file, insockfd, in);

        printf("Returing 0\n");

        close(insockfd);
    }
    return 0;
}

/*
    sets socket options and binds socket
*/
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

/*
    given an incoming connection and a buffer, reads the http request details,
    and puts the requested filename into the buffer.
*/
int read_input(FILE* in, char *buffer) {
    char * str = NULL;
    char * url;
    strcpy(buffer, DEFAULT_PAGE);
    size_t len = 0;

    printf("Reading\n");
    getline(&str, &len, in);
    while (str[0] != '\r' && str[0] != '\n')  {
        printf("%s", str);
        if (strncmp(str, "GET ", 4) == 0) {
            url = str+4;
            char *ptr = strchr(url, ' ');
            if (ptr != NULL) {
                *ptr = '\0';
            }

            if (strcmp(url, "/") != 0) {
                strcpy(buffer, url);
            }
            printf("url: \"%s\"\n", url);
        }
        getline(&str, &len, in);
    }
    return 0;

}


/*
    given an inconming connection, sends the requested web page to the client.
*/
int respond(char *file, int sockfd, FILE *in) {

    printf("Writing to output\n");

    //fprintf(in, "HTTP/1.1 200 OK\n");

    int fd;
    if ((fd = open(file, O_RDONLY, 0)) == -1) {
        printf("Failed to open file\n");
        char f[500];
        strcpy(file, HTML_ROOT);
        strcat(file, NOT_FOUND);
        if ((fd = open(file, O_RDONLY, 0)) == -1) {
            printf("Failed to open 404 file: %s\n", file);
            return -1;
        }
    } // try to open the file
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

/*
    returns the filesize of a file.
*/
int get_file_size(int fd) {
   struct stat stat_struct;

   if(fstat(fd, &stat_struct) == -1)
      return -1;
   return (int) stat_struct.st_size;
}
