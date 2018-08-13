#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>

void *respond(char *request);

#define PORT 80

int main()
{
    int yes = 1;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes) == -1) {
        perror("setsockopt");
        exit(1);
    }


    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1) {
        printf("bind error.\n");
        return -1;
    }
    printf("Socket bound successfully.\n");


    listen(sockfd, SOMAXCONN);

    int insockfd;

    if ((insockfd = accept(sockfd, NULL, NULL)) == -1) {
        printf("accept error.\n");
        return -1;
    }
    printf("Accepted connection successfully.\n");

    FILE *in = fdopen(insockfd, "r+");
    char * str = NULL;
    size_t len = 0;

    /*
    while (getline(&str, &len, in))  {
        printf("%s", str);
    }
    */

    //getline(&str, &len, in);
    FILE *html = fopen("html/index.html", "r");

    if (html == NULL) {
        printf("fopen error.\n");
        return -1;
    }

    /*
    HTTP/1.1 200 OK
    Date: Mon, 27 Jul 2009 12:28:53 GMT
    Server: Apache/2.2.14 (Win32)
    Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT
    Content-Length: 88
    Content-Type: text/html
    Connection: Closed
    */
    printf("Reading\n");
    getline(&str, &len, in);
    while (str[0] != '\r' && str[0] != '\n')  {
        printf("%x, %s, %c\n", *str, str, str[0]);
        getline(&str, &len, in);
    }


    printf("Writing to output\n");
    fprintf(in, "HTTP/1.1 200 OK\n");
    while (1) {
        unsigned char buffer[256] = {0};
        int nread = fread(buffer, 1, 256, html);
        printf("nread: %d\n", nread);
        if (nread < 1) {
            break;
        }
        int nwritten = fprintf(in, buffer);
        if (nwritten < 1) {
            printf("Writting error.\n");
        }
    }
    //respond(str);

    printf("Returing 0\n");
    return 0;
}

void *respond(char *request)
{
    printf("req:\n%s", request);
    return NULL;
}

void close_handler(int sig)
{

}
