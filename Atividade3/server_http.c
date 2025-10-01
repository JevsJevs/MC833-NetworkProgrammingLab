#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define LISTENQ      10
#define MAXDATASIZE  256
#define MAXLINE      4096

//=============================================================================================
//=============================WRAPPERS========================================================
int Socket(int domain, int type, int protocol){
    int listenfd;
    if ((listenfd = socket(domain, type, protocol)) < -1) {
        perror("socket");
        exit(1);
    }
    return listenfd;
}

int Bind(int listenfd, struct sockaddr* adrptr, int adrSize){
    int ret = bind(listenfd, (struct sockaddr *)adrptr, adrSize);
    if (ret == -1) {
        perror("bind");
        close(listenfd);
        exit(1);
    }
    return ret;
}

int Listen(int listenfd, int queueSize){
    int ret = listen(listenfd, queueSize);
    if (ret == -1) {
        perror("listen");
        close(listenfd);
        return 1;
    }
    return ret;
}

int Accept(int fd, struct sockaddr * addr, socklen_t * addr_len){
    int connfd = accept(fd, (struct sockaddr*)addr, addr_len);
    if (connfd == -1) {
        perror("accept");
        exit(1);
    }
    return connfd;
}

int Fork(){
    pid_t pid = fork();
    if (pid == -1){
        perror("fork");
        exit(1);
    }
    return pid;
}

int Close(int fd){
    int ret = close(fd);
    if (ret == -1){
        perror("close");
        exit(1);
    }
    return ret;
}

ssize_t Read(int fd, void *buf, size_t n){
    ssize_t ret = read(fd, buf, n);
    if (ret == -1){
        perror("read");
        close(fd);
        return -1;
    }
    return ret;
}

ssize_t Write(int fd, void *buf, size_t n){
    ssize_t ret = write(fd, buf, n);
    if (ret == -1){
        perror("write");
        close(fd);
        return -1;
    }
    return ret;
}

int GetSockName(int listenfd, struct sockaddr_in* bound, socklen_t boundSize){
    int ret = getsockname(listenfd, (struct sockaddr*) bound, &boundSize);
    if (ret != 0) {
        printf(" Error");
        return -1;
    }
    return ret;
}

int GetPeerName(int fd, struct sockaddr *addr, socklen_t len){
    int ret = getpeername(fd, (struct sockaddr *) addr, &len);
    if (ret < 0)
    {
        perror("getpeername");
        exit(1);
    }
    return ret;
}

int main(void) {
    int listenfd, connfd;
    struct sockaddr_in servaddr, checkadr;
    memset(&checkadr, 0, sizeof(checkadr)); //variaveis para checagem de adr de connexão

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); 
    
    // Escolhe porta aleatória entre 1 e 65535
    srand(time(NULL));
    servaddr.sin_port        = htons(rand() % 65535 + 1);              

    Bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    // Descobrir porta real e divulgar em arquivo server.info
    struct sockaddr_in bound; socklen_t blen = sizeof(bound);
    if (GetSockName(listenfd, &bound, blen) == 0){
        unsigned short p = ntohs(bound.sin_port);
        printf("[SERVIDOR] Escutando em 127.0.0.1:%u\n", p);
        FILE *f = fopen("server.info", "w");
        if (f) { fprintf(f, "IP=127.0.0.1\nPORT=%u\n", p); fclose(f); }
        fflush(stdout);
    }
    
    Listen(listenfd, LISTENQ);
    
    // laço: aceita clientes, envia banner e fecha a conexão do cliente
    for (;;) {
        // Aceita conexões de forma concorrente criando processos filhos
        (void) Fork();

        connfd = Accept(listenfd, NULL, NULL);
        
        GetPeerName(connfd, (struct sockaddr *) &checkadr, sizeof(checkadr));

        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &checkadr.sin_addr, ip_str, INET_ADDRSTRLEN); //converte o adr recuperado por getsockname ao formato de chars e guarda em ip_str 
        printf("remote: %s, Porta: %d\n", ip_str, ntohs(checkadr.sin_port));//exibe o ip e porta (convertida para int)
        

        // envia banner "Hello + Time"
        char buf[MAXDATASIZE];
        time_t ticks = time(NULL); // ctime() já inclui '\n'
        snprintf(buf, sizeof(buf), "Hello from server!\nTime: %.24s\r\n", ctime(&ticks));
        Write(connfd, buf, strlen(buf));

        sleep(5);
        Close(connfd); // fecha só a conexão aceita; servidor segue escutando
    }

    return 0;
}
