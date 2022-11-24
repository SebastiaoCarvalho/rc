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


int main(int argc, char const *argv[])
{
    /* int pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }
    if (pid > 0)
    { */

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
        errcode=getaddrinfo(NULL,"58003",&hints,&res);
        if(errcode!=0) /*error*/ exit(1);
        n=bind(fd,res->ai_addr, res->ai_addrlen);
        if(n==-1) /*error*/ exit(1);
        while (1){
            addrlen=sizeof(addr);
            n=recvfrom(fd,msgCode,3,0, (struct sockaddr*)&addr,&addrlen);
            msgCode[3] = '\0';
            if(n==-1)/*error*/exit(1);
            printf("%d\n", n);
            printf("%s\n", msgCode);
            printf("%d\n", strncmp(msgCode, "SNG", 3));
            if (1) {
                printf("SNG");
                char id[7];
                // n = recvfrom(fd, id, 1, 0, (struct sockaddr*)&addr, &addrlen);
                // printf("debug: %d\n", n);
                // if(n==-1)/*error*/exit(1);
                // n = recvfrom(fd,id,6,0, (struct sockaddr*)&addr,&addrlen);
                n = read(fd, id, 1);
                printf("debug: %d\n", n);
                if(n==-1)/*error*/exit(1);
                n = read(fd,id,6);
                id[6] = '\0';
                if(n==-1)/*error*/exit(1);
                printf("read id %s", id);
            }
            else if (strcmp(msgCode, "RSG") == 0) {
                printf("RSG");
            }
            else if (strcmp(msgCode, "PLG") == 0) {

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
    /* } */
    /* else
    {
        // parent process
        //servertcp();
    } */
}
