#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>
#define BUFFSIZE 4
#define MAXPENDING 5
#define MAXTHREADPOOL 5
const int HTTP_EOF_LEN = 4;
const char *HTTP_EOF = "\r\n\r\n";

int serversock;
struct sockaddr_in server;
    
static void Die(const char *mess)
{ 
    perror(mess); 
    exit(1);
}

static void _set_tcp_nodelay(int fd)
{
    int enable = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void*)&enable, sizeof(enable));
}

static void _set_tcp_reuseaddr(int fd)
{
    int enable = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void*)&enable, sizeof(int));
}

void *handle(int *sock)
{
    char *src = NULL, *dst = NULL;
    char buffer[BUFFSIZE];
    ssize_t received = -1;
    int count = 0, offset = 0;
    do {
        if ((received = recv(*sock, buffer, BUFFSIZE, 0)) < 0) {
            fprintf(stderr, "HTTP包接收失败.");
            return (void *)1;
        }
        src = dst;
        count += received;
        dst = (char *)malloc((count + 1) * sizeof(char));
        if (src != NULL) {
            memcpy(dst, src, offset);
            free(src);
        }
        memcpy(dst + offset, buffer, received * sizeof(char));
        dst[count] = '\0';
        offset = count * sizeof(char);
        if (memcmp(dst + strlen(dst) - HTTP_EOF_LEN, HTTP_EOF, HTTP_EOF_LEN) == 0) {
            // puts("HTTP包接收完毕.");
            break;
        }
    } while (received > 0);
    // puts(dst);
    return (void *)0;
}

pthread_t go(void *(*func)(void *), void *args)
{
    int err;
    pthread_t tid;
    if ((err = pthread_create(&tid, NULL, func, args)) != 0) {
        Die(strcat("Failed to create thread: ", strerror(err)));
    }
    return tid;
}

void loop()
{
    int sock;
    struct sockaddr_in client;
    socklen_t len;
    for (;;) {
        if ((sock =
                 accept(serversock, (struct sockaddr *)&client, &len)) < 0) {
            Die("Failed to accept client connection");
        }
        fprintf(stdout, "Client connected: %s\n",
                inet_ntoa(client.sin_addr));

        go((void *(*)(void *))handle, (void *)&sock);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 4) {
        fprintf(stderr, "USAGE: HTTPServer <ServerIP> <Port> <Word>");
        exit(1);
    }
    if ((serversock =
             socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        Die("Failed to create socket");
    }
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons(atoi(argv[2]));
    _set_tcp_nodelay(serversock);
    _set_tcp_reuseaddr(serversock);
    if (bind(serversock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        Die("Failed to bind the server socket");
        exit(1);
    }
    if (listen(serversock, MAXPENDING) < 0) {
        Die("Failed to listen on server port");
    }
    loop();
    return 0;
}