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

int main(void) {
    int listenfd, connfd;
    struct sockaddr_in servaddr, checkadr;
    memset(&checkadr, 0, sizeof(checkadr)); //variaveis para checagem de adr de connexão
    socklen_t addr_len = sizeof(checkadr);

    // socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        return 1;
    }

    
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); 
    servaddr.sin_port        = htons(13);              

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind");
        close(listenfd);
        return 1;
    }
    // Descobrir porta real e divulgar em arquivo server.info
    struct sockaddr_in bound; socklen_t blen = sizeof(bound);
    if (getsockname(listenfd, (struct sockaddr*)&bound, &blen) == 0) {
        unsigned short p = ntohs(bound.sin_port);
        printf("[SERVIDOR] Escutando em 127.0.0.1:%u\n", p);
        FILE *f = fopen("server.info", "w");
        if (f) { fprintf(f, "IP=127.0.0.1\nPORT=%u\n", p); fclose(f); }
        fflush(stdout);
    }

    // listen
    if (listen(listenfd, LISTENQ) == -1) {
        perror("listen");
        close(listenfd);
        return 1;
    }
    
    
    // laço: aceita clientes, envia banner e fecha a conexão do cliente
    for (;;) {
        connfd = accept(listenfd, NULL, NULL);
        if (connfd == -1) {
            perror("accept");
            continue; // segue escutando
        }
        
        if(getpeername(connfd, (struct sockaddr *) &checkadr, &addr_len) < 0) {
            printf("getsocket infor fail");
            return 1;
        }

        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &checkadr.sin_addr, ip_str, INET_ADDRSTRLEN); //converte o adr recuperado por getsockname ao formato de chars e guarda em ip_str 
        printf("remote: %s, Porta: %d\n", ip_str, ntohs(checkadr.sin_port));//exibe o ip e porta (convertida para int)
        

        // envia banner "Hello + Time"
        char buf[MAXDATASIZE];
        time_t ticks = time(NULL); // ctime() já inclui '\n'
        snprintf(buf, sizeof(buf), "Hello from server!\nTime: %.24s\r\n", ctime(&ticks));
        (void)write(connfd, buf, strlen(buf));

        close(connfd); // fecha só a conexão aceita; servidor segue escutando
    }

    return 0;
}
