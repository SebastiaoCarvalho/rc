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
        char buffer[129];
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
            printf("Waiting for message...\n");
            n=recvfrom(fd,buffer, 128, 0, (struct sockaddr*)&addr,&addrlen);
            buffer[128] = '\0';
            if(n==-1)/*error*/exit(1);
            if (strncmp(buffer, "SNG", 3) == 0){
                /* Read PlayerID*/
                char id[7];
                strncpy(id, buffer+4, 6);
                id[6] = '\0';
                if(n==-1)/*error*/exit(1);
                printf("PlayerID: %s\n", id);
                ssize_t offset = 0;
                strncpy(buffer, "RNG ", 4);
                offset += 4;
                strncpy(buffer + offset, "OK", 2);
                offset += 2;
                strncpy(buffer + offset, "5", 1);
                offset += 1;
                strncpy(buffer + offset, " ", 1);
                offset += 1;
                strncpy(buffer + offset, "7", 1);
                offset += 1;
                buffer[offset] = '\0';
                n=sendto(fd,buffer, 128 ,0 , (struct sockaddr*)&addr, addrlen);
            }
            else if (strncmp(buffer, "PLG", 3) == 0) {
                /* Read PlayerID*/
                char id[7];
                strncpy(id, buffer+4, 6);
                id[6] = '\0';
                if(n==-1)/*error*/exit(1);
                printf("%s\n", id);
                /*Read letter*/
                char letter[2];
                strncpy(letter, buffer+11, 1);
                letter[1] = '\0';
                printf("%s\n", letter);
                /*Read Trial*/
                char trial[2];
                strncpy(trial, buffer+13, 1);
                trial[1] = '\0';
                int trial_int = atoi(trial);
                trial_int++;
                snprintf(trial, 1, "%d", trial_int);
                trial[1] = '\0';
                printf("%s\n", trial);
                char status[3];
                if (trial_int > 9) {
                    strncpy(status, "OVR", 3);
                }
                else {
                    strncpy(status, "OK", 2);
                    strncpy(status + 2, "", 1);
                }
                strncpy(buffer, "RLG ", 3);
                ssize_t offset = 4;
                strncpy(buffer + offset, status, 3);
                offset += 3;
                strncpy(buffer + offset, " ", 1);
                offset += 1;
                strncpy(buffer + offset, trial, 1);
                offset += 1;
                strncpy(buffer + offset, " ", 1);
                offset += 1;
                strncpy(buffer + offset, "1", 1);
                buffer[offset] = '\0';
                n=sendto(fd,buffer, 128 ,0 , (struct sockaddr*)&addr, addrlen);

            }
            else if (strncmp(buffer, "PWG", 3) == 0) {

            }
            else if (strncmp(buffer, "RWG", 3) == 0) {

            }
            else if (strncmp(buffer, "QUT", 3) == 0) {

            }            
            else if (strncmp(buffer, "RQT", 3) == 0) {

            }
            else if (strncmp(buffer, "REV", 3) == 0) {
                
            }
            else if (strncmp(buffer, "RRV", 3) == 0) {
                
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

