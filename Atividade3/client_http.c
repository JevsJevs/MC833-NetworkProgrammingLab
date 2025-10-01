// cliente_base.c — conecta, lê banner do servidor e fecha
// Compilação: gcc -Wall cliente_base.c -o cliente
// Uso: ./cliente [IP] [PORT]

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#define LISTENQ      10
#define MAXDATASIZE  256
#define MAXLINE 4096
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

int Connect(int fd, void *servaddr, size_t n){
    int ret = connect(fd, (struct sockaddr *)servaddr, n);
    if (ret < 0) {
        perror("connect error");
        close(fd);
        return 1;
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
    if (ret < 0){
        perror("getpeername");
        exit(1);
    }
    return ret;
}



int main(int argc, char **argv) {
    int    sockfd;
    struct sockaddr_in servaddr, checkadr;
    void *ret;

    // IP/PORT (argumentos ou server.info)
    char ip[INET_ADDRSTRLEN] = "127.0.0.1";
    unsigned short port = 13;

    if (argc >= 2) strncpy(ip, argv[1], sizeof(ip)-1);
    if (argc >= 3) port = (unsigned short)atoi(argv[2]);

    if (port == 0) {
        FILE *f = fopen("server.info", "r");
        if (f) {
            char line[128]; int got_p = 0;
            while ((ret = (void*)fgets(line, sizeof(line), f))) {
                (void)sscanf(line, "IP=%127s", ip);        // lê IP se houver, sem flag
                if (sscanf(line, "PORT=%hu", &port) == 1) got_p = 1;
            }
            fclose(f);
            if (!got_p) port = 0;
        }
        if (port == 0) {
            fprintf(stderr, "Uso: %s <IP> [PORT] (ou forneça server.info)\n", argv[0]);
            return 1;
        }

    }

    // socket
    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    // connect
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(port);
    if (inet_pton(AF_INET, ip, &servaddr.sin_addr) <= 0) {
        perror("inet_pton error");
        Close(sockfd);
        return 1;
    }
    Connect(sockfd, &servaddr, sizeof(servaddr));

    //Mostra dados da conexao
    memset(&checkadr, 0, sizeof(checkadr)); //Zera o conteudo do checkadr, que conterá o adr recuperado da conexão
    socklen_t addr_len = sizeof(checkadr);

    // envia banner "Hello + Time"
    char buf[MAXDATASIZE];
    snprintf(buf, sizeof(buf), "GET / HTTP/1.1");
    Write(sockfd, buf, strlen(buf));
    
    GetSockName(sockfd, &checkadr, addr_len);

    char ip_str[INET_ADDRSTRLEN]; //Declara "string" que conterá o IP
    inet_ntop(AF_INET, &checkadr.sin_addr, ip_str, INET_ADDRSTRLEN); //converte o adr recuperado por getsockname ao formato de chars e guarda em ip_str 
    printf("local: %s, Porta: %d\n", ip_str, ntohs(checkadr.sin_port)); //exibe o ip e porta (convertida para int)

    // lê e imprime o banner (uma leitura basta neste cenário)
    char banner[MAXLINE + 1];
    ssize_t n = Read(sockfd, banner, MAXLINE);
    if (n > 0) {
        banner[n] = 0;
        fputs(banner, stdout);
        fflush(stdout);
    }
    sleep(5);
    Close(sockfd);
    return 0;
}
