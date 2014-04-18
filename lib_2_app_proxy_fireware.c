#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <getopt.h>
#include <pthread.h>
#include <arpa/inet.h>

#define REMOTE_SERVER_PORT 80
#define BUF_SIZE 4096
#define QUEUE_SIZE 100
#define BLACKED_SERVER "www.renren.com"

char ALLOWD_CLIENTIP[20] = "127.0.0.1";

char lastservername[256] = "";
int lastserverip = 0;
pthread_mutex_t conp_mutex;


int main(int argc, char **argv);
int checkclient(in_addr_t cli_addr);
void dealonereq(void * arg);
int checkserver(char *hostname);
int gethostname(char *buf, char *hostname,int length);
int connectserver(char * hostname);


int main(int argc, char **argv)
{
    printf("starting\n");
    fflush(stdout);
    short port = 0;
    char opt;
    struct sockaddr_in cl_addr,proxyserver_addr;
    socklen_t sin_size = sizeof(struct sockaddr_in);
    int sockfd,accept_sockfd,on = 1;
    pthread_t Clitid;
    printf("deal argu\n");
    fflush(stdout);
    while((opt = getopt(argc, argv, "p:")) != EOF)
    {
        switch(opt)
        {
            case 'p':
                port = (short)atoi(optarg);
                break;
            default:
                printf("Usage: %s -p port\n", argv[0]);
                return -1;
        }
    }
    if (port == 0)
    {
        printf("Invalid port number,try again.\n");
        printf("Usage: %s -p port\n", argv[0]);
        return -1;
    }
    printf("create socket\n");
    fflush(stdout);
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
    {
        printf("Socket failed .. Abort...\n");
    }

    memset(&proxyserver_addr, 0, sizeof(proxyserver_addr));
    proxyserver_addr.sin_family = AF_INET;
    proxyserver_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    proxyserver_addr.sin_port = htons(port);

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));

    if(bind(sockfd,(struct sockaddr *)&proxyserver_addr,sizeof(proxyserver_addr)) < 0)
    {
        printf("Bind failed...Abort...\n");
        return -1;
    }
    if(listen(sockfd, QUEUE_SIZE) < 0)
    {
        printf("listen failed...Abort...\n");
        return -1;
    }
    printf("enter while\n");
    fflush(stdout);
    while(1)
    {
        printf("wait....\n");
		fflush(stdout);
        accept_sockfd = accept(sockfd, (struct sockaddr *)&cl_addr, &sin_size);
	    printf("accept....a packet.\n");
        fflush(stdout);
        if(accept_sockfd < 0)
        {
            printf("accept failed");
            continue;
        }
        printf("Received a request from %s:%d\n", inet_ntoa(cl_addr.sin_addr), ntohs(cl_addr.sin_port));
        if(checkclient(cl_addr.sin_addr.s_addr) == 1)
        {
            printf("create thread!\n");
            pthread_create(&Clitid, NULL, (void*)dealonereq, (void*)accept_sockfd);

        }
        else
        {
            close(accept_sockfd);
        }
    }
    return 0;
}

int checkclient(in_addr_t cli_addr)
{
    int allowedip;
    inet_aton(ALLOWD_CLIENTIP,&allowedip);
    printf("allowedip:%d\n", allowedip); 
    printf("cli_addr:%d\n", cli_addr);
    if(allowedip != cli_addr)
    {
        printf("Client Ip authentication failed!\n");
        return -1;
    }

    return 1;
}

void dealonereq(void *arg)
{
    char buf[BUF_SIZE];
    int bytes;
    char recvbuf[BUF_SIZE];
    char hostname[256];
    int remotesocket;
    int accept_sockfd;
    accept_sockfd = (int)arg;
    pthread_detach(pthread_self());
    bzero(buf, BUF_SIZE);
    bzero(recvbuf, BUF_SIZE);
    printf("read data from client\n");
    fflush(stdout);
    bytes = read(accept_sockfd, buf, BUF_SIZE);
    if(bytes <= 0)
    {
        close(accept_sockfd);
        return;
    }
    gethostname(buf, hostname, bytes);

    if(sizeof(hostname) == 0)
    {
        printf("Invalid host name");
        close(accept_sockfd);
        return;
    }

    if(checkserver(hostname) == -1)
    {
        close(accept_sockfd);
        return;
    }

    remotesocket = connectserver(hostname);
    if(remotesocket == -1)
    {
        close(accept_sockfd);
        return;
    }
    send(remotesocket, buf, bytes, 0);
    while(1)
    {
        int readSizeOnce = 0;
        printf("read data from server!\n");
        readSizeOnce = read(remotesocket, recvbuf, BUF_SIZE);

        if(readSizeOnce <= 0)
        {
            break;
        }
        printf("send data to client");
        send(accept_sockfd, recvbuf, readSizeOnce, 0);
    }

    close(remotesocket);
    close(accept_sockfd);
}

int gethostname(char *buf, char *hostname, int length)
{
    printf("get host name starting.\n");
    fflush(stdout);
    char *p;
    int i,j = 0;
    bzero(hostname, 256);
    p = strstr(buf,"Host: ");
    if(!p)
        p = strstr(buf, "host: ");

    i = (p - buf) + 6;
    for(j = 0; i<length; i++,j++)
    {
        if(buf[i] == '\r')
        { 
            hostname[j] = '\0';
            return 0;
        }
        else
        {
            hostname[j] = buf[i];
        }
    }
    printf("get host name ended.\n");
    fflush(stdout);
    return -1;
}

int checkserver(char *hostname)
{
    if(strstr(hostname, BLACKED_SERVER) != NULL)
    {
        printf("Destination blocked! \n");
        return -1;
    }
    return 0;
}

int connectserver(char * hostname)
{
    int cnt_stat;
    struct hostent * hostinfo;
    struct sockaddr_in server_addr;
    int remotesocket;
    printf("connect server starting..\n");
    fflush(stdout);
    remotesocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(remotesocket < 0)
    {
        printf("can not create socket!");
        return -1;
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(REMOTE_SERVER_PORT);
    pthread_mutex_lock(&conp_mutex);

    if(strcmp(lastservername, hostname) != 0)
    {
        hostinfo = gethostbyname(hostname);
        if(!hostinfo)
        {
            printf("gethostbyname failed!\n");
            return -1;
        }

        strcpy(lastservername, hostname);
        lastserverip = *(int *)hostinfo->h_addr;
        server_addr.sin_addr.s_addr = lastserverip;
    }
    else
        server_addr.sin_addr.s_addr = lastserverip;

    pthread_mutex_unlock(&conp_mutex);
    cnt_stat = connect(remotesocket, (struct sockaddr*)&server_addr, sizeof(server_addr));

    if(cnt_stat < 0)
    {
        printf("remote connect failed !\n");
        close(remotesocket);

        return -1;
    }
    else
    {
        printf("connected remote server------------------>%s:%u.\n",
            inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
    }
    return remotesocket;
}
