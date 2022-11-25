/* Função do gameserver: criar os dois servers(udp e tcp) e depois ir lendo 
os comandos que o player vai mandando*/

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>

int udp(void){
    int fd,errcode;
    ssize_t n;
    socklen_t addrlen;
    struct addrinfo hints,*res;
    struct sockaddr_in addr;
    char msgCode[4];
    // child process
    fd=socket(AF_INET,SOCK_DGRAM,0); //UDP socket
    if(fd==-1) /*error*/exit(1);
    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET; // IPv4
    hints.ai_socktype=SOCK_DGRAM; // UDP socket
    hints.ai_flags=AI_PASSIVE;
    errcode=getaddrinfo(NULL,"58002",&hints,&res);
    if(errcode!=0) /*error*/ exit(1);
    n=bind(fd,res->ai_addr, res->ai_addrlen);
    if(n==-1) /*error*/ exit(1);
    while (1){
        addrlen=sizeof(addr);
        n=recvfrom(fd,msgCode,3,0, (struct sockaddr*)&addr,&addrlen);
        msgCode[3] = '\0';
        if(n==-1)/*error*/exit(1);
        if (strcmp(msgCode, "SNG") == 0){
            /* Read PlayerID*/
            char id[7];
            n = recvfrom(fd, id, 1, 0, (struct sockaddr*)&addr, &addrlen);
            if(n==-1)/*error*/exit(1);
            n = recvfrom(fd, id, 6, 0, (struct sockaddr*)&addr,&addrlen);
            id[6] = '\0';
            if(n==-1)/*error*/exit(1);
        }
        else if (strcmp(msgCode, "RSG") == 0) {
            printf("RSG");
            
        }
        else if (strcmp(msgCode, "PLG") == 0) {
            /* Read PlayerID*/
            char id[7];
            n = recvfrom(fd, id, 1, 0, (struct sockaddr*)&addr, &addrlen);
            if(n==-1)/*error*/exit(1);
            n = recvfrom(fd, id, 6, 0, (struct sockaddr*)&addr,&addrlen);
            id[6] = '\0';
            if(n==-1)/*error*/exit(1);
            printf("%s", id);
            /*Read letter*/
            char letter[2];
            n = recvfrom(fd, letter, 1, 0, (struct sockaddr*)&addr, &addrlen);
            if(n==-1)/*error*/exit(1);
            letter[1] = '\0';
            printf("%s", letter);
            /*Read Trial*/
            char trial[2];
            n = recvfrom(fd, &trial, 1, 0, (struct sockaddr*)&addr, &addrlen);
            if(n==-1)/*error*/exit(1);
            trial[1] = '\0';
            printf("%s\n", trial);
        }
        else if (strcmp(msgCode, "RLG") == 0) {

        }
        else if (strcmp(msgCode, "PWG") == 0) {

        }
        else if (strcmp(msgCode, "RWG") == 0) {

        }
        else if (strcmp(msgCode, "QUT") == 0) {

        }            
        else if (strcmp(msgCode, "RQT") == 0) {

        }
        else if (strcmp(msgCode, "REV") == 0) {
                
        }
        else if (strcmp(msgCode, "RRV") == 0) {
                
        }
        else {
            printf("Invalid message code");
        }
    }
    freeaddrinfo(res);
    close(fd);
    return 0;
}


int main(int argc, char const *argv[])
{
    int pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }
    if (pid > 0) {
        
        udp();
    } else{
        // parent process
        //servertcp();
    } 
}
