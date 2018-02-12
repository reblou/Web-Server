#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>

void *respond(char *request);

int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(80);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1) {
        printf("bind error.\n");
        return -1;
    }


    listen(sockfd, SOMAXCONN);

    int insockfd;

    if ((insockfd = accept(sockfd, NULL, NULL)) == -1) {
        printf("accept error.\n");
        return -1;
    }

    FILE *in = fdopen(insockfd, "r+");
    char * str = NULL;
    size_t len = 0;
    /*
    while (getline(&str, &len, in))  {
        printf("%s", str);
    }
    */
    getline(&str, &len, in);
    respond(str);
    return 0;
}

void *respond(char *request)
{
    printf("req:\n%s", request);
    return NULL;

}
