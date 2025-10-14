#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

#define MAXDATASIZE  512
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

typedef void Sigfunc(int);
//Função wrapper que trata os sinais, recebe o codigo do sinal + função tratadora
Sigfunc * Signal (int signo, Sigfunc *func) {
    struct sigaction act, oact;
    act.sa_handler = func;
    sigemptyset (&act.sa_mask); /* Outros sinais não são bloqueados*/
    act.sa_flags = 0;
    if (signo == SIGALRM) { /* Para reiniciar chamadas interrompidas */
    #ifdef SA_INTERRUPT
    act.sa_flags |= SA_INTERRUPT; /* SunOS 4.x */
    #endif
    } else {
    #ifdef SA_RESTART
    act.sa_flags |= SA_RESTART; /* SVR4, 4.4BSD */
    #endif
    }
    if (sigaction (signo, &act, &oact) < 0)
        return (SIG_ERR);
    return (oact.sa_handler);
 }

 //Função tratadora do SIGCHLD
 void sigchld_handler(int sig) {
    (void)sig; // Ignora o argumento, pois só usaremos para tratar o SIGCHLD
    int saved_errno = errno; // salva o num erro para não sobrescrever em caso de falha do waitpid

    // Limpa os zombies
    while (waitpid(-1, NULL, 1) > 0);

    errno = saved_errno;
}

int main(int argc, char *argv[]) {

    if(argc != 4){
        perror("Existem argumentos faltando");
        exit(1);
    }

    //Lê parametros do programa
    int porta = atoi(argv[1]);
    int backlog = atoi(argv[2]);
    int tempoSleep = atoi(argv[3]);

    int listenfd, connfd;
    struct sockaddr_in servaddr, checkadr;
    memset(&checkadr, 0, sizeof(checkadr)); //variaveis para checagem de adr de connexão

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    memset(&servaddr, 0, sizeof(servaddr)); // Remove lixo armazenado na memoria no momento da alocação
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); 
    
    // Escolhe porta aleatória entre 1024 e 65535 caso o parametro 'porta' for 0, senão a porta de conexão é a definida no argumento
    if(porta == 0){        
        srand(time(NULL));
        servaddr.sin_port = htons(rand() % 64512 + 1024);     
    }
    else
        servaddr.sin_port = htons(porta);

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
    
    Listen(listenfd, backlog);
    
    // laço: aceita clientes, envia banner e fecha a conexão do cliente
    for (;;) {
        // Aceita conexões de forma concorrente criando processos filhos
        connfd = Accept(listenfd, NULL, NULL);
        GetPeerName(connfd, (struct sockaddr *) &checkadr, sizeof(checkadr));

        //Trata SIGCHILD eliminando zombies
        if (Signal(SIGCHLD, sigchld_handler) == SIG_ERR) {
            perror("Signal error");
            exit(EXIT_FAILURE);
        }
        
        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &checkadr.sin_addr, ip_str, INET_ADDRSTRLEN); //converte o adr recuperado por getsockname ao formato de chars e guarda em ip_str 
        printf("remote: %s, Porta: %d\n", ip_str, ntohs(checkadr.sin_port));//exibe o ip e porta (convertida para int)
        
        int pid = Fork();

        if(pid == 0){
            Close(listenfd); //Fecha listener pois no processo filho não é necessário
            // Lê Request do cliente
            char buf[MAXDATASIZE];
            Read(connfd, buf, MAXDATASIZE);
            
            char header[15];
            memcpy(header, buf, 14);
            header[14] = '\0';

            printf("Lido do client: %s\n", header); //Exibe mensagem recebida do cliente

            //Testa a mensagem recebida comparando-a com o esperado
            if((strcmp(header, "GET / HTTP/1.0")  == 0) || (strcmp(header, "GET / HTTP/1.1")  == 0)){
                // envia banner "Hello + Time"
                snprintf(buf, sizeof(buf), "HTTP/1.0 200 OK\r\n\
                                        Content-Type: text/html\r\n\
                                        Content-Length: 91\r\n\
                                        Connection: close\r\n\
                                        \r\n<html><head><title>MC833</title</head><body><h1>MC833 TCP\
                                        Concorrente </h1></body></html>");
                                        Write(connfd, buf, strlen(buf));
            }
            else {
                snprintf(buf, sizeof(buf), "400: Bad Request");
                Write(connfd, buf, strlen(buf));
            }
            
            sleep(tempoSleep);
            Close(connfd); // fecha só a conexão aceita; servidor segue escutando
            exit(0);
        }
        Close(connfd);
    }

    return 0;
}
//./client_http localhost 0 
