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
#include <vector>
#include "utils.h"
#include <string>
#include <signal.h>

int fd,errcode;
socklen_t addrlen;
struct addrinfo hints,*res;
struct sockaddr_in addr;

int main(int argc, char const *argv[])
{
    
    // void handleCtrlC(int s);
    
    ssize_t n;
    int pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }
    signal(SIGINT, handleCtrlC);
    if (pid > 0)
    {

        char buffer[129];
        // child process
        fd=socket(AF_INET,SOCK_DGRAM,0); //UDP socket
        if(fd==-1) /*error*/exit(1);
        memset(&hints,0,sizeof hints);
        hints.ai_family=AF_INET; // IPv4
        hints.ai_socktype=SOCK_DGRAM; // UDP socket
        hints.ai_flags=AI_PASSIVE;
        errcode=getaddrinfo(NULL,"58004",&hints,&res);
        if(errcode!=0) /*error*/ exit(1);
        n=bind(fd,res->ai_addr, res->ai_addrlen);
        printf("%d\n", errno);
        if(n==-1) /*error*/ exit(1);
        while (1){
            addrlen=sizeof(addr);
            printf("Waiting for message...\n");
            n=recvfrom(fd,buffer, 128, 0, (struct sockaddr*)&addr,&addrlen);
            buffer[128] = '\0';
            if(n==-1)/*error*/exit(1);
            int pid2 = fork();
            if (pid2 == -1) {
                perror("fork");
                exit(1);
            }
            if (pid2 > 0)
            {
                if (strncmp(buffer, "SNG", 3) == 0) {
                    /* Read PlayerID*/
                    char id[7];
                    memcpy(id, buffer+4, 6);
                    id[6] = '\0';
                    if(n==-1)/*error*/exit(1);
                    printf("PlayerID: %s\n", id);
                    ssize_t offset = 0;
                    memcpy(buffer, "RNG ", 4);
                    offset += 4;
                    memcpy(buffer + offset, "OK ", 3);
                    offset += 3;
                    std::string word = "batata";
                    size_t wordLen = word.length();
                    std::string wordLenStr = std::to_string(wordLen);
                    memcpy(buffer + offset, wordLenStr.c_str(), wordLenStr.length());
                    offset += wordLenStr.length();
                    memcpy(buffer + offset, " ", 1);
                    offset += 1;
                    int tries = maxErrors(word);
                    std::string tries_str = std::to_string(tries);
                    memcpy(buffer + offset, tries_str.c_str() , tries_str.length());
                    offset += tries_str.length();
                    memcpy(buffer + offset, "\n\0", 2);
                    n=sendto(fd,buffer, 128 ,0 , (struct sockaddr*)&addr, addrlen);
                }
                else if (strncmp(buffer, "PLG", 3) == 0) {
                    /* Read PlayerID*/
                    char id[7];
                    memcpy(id, buffer+4, 6);
                    id[6] = '\0';
                    if(n==-1)/*error*/exit(1);
                    printf("%s\n", id);
                    /*Read letter*/
                    char letter[2];
                    memcpy(letter, buffer+11, 1);
                    letter[1] = '\0';
                    printf("%s\n", letter);
                    /*Read Trial*/
                    char trial[2];
                    memcpy(trial, buffer+13, 1);
                    trial[1] = '\0';
                    printf("%s\n", trial);
                    int trial_int = atoi(trial);
                    printf("trial_int: %d\n", trial_int);
                    std::string status;
                    char word[30];
                    strcpy(word, "batata");
                    std::vector<int> pos = getPos(word, letter[0]);
                    if (trial_int + 1 > 9) {
                        status = "OVR";
                    }
                    else if (pos.size() > 0) {
                        status = "OK";
                    }
                    else {
                        status = "NOK";
                    }
                    memcpy(buffer, "RLG ", 3);
                    ssize_t offset = 4;
                    memcpy(buffer + offset, status.c_str(), status.length());
                    offset += status.length();
                    memcpy(buffer + offset, " ", 1);
                    offset += 1;
                    memcpy(buffer + offset, trial, 1);
                    offset += 1;
                    size_t len = pos.size();
                    if (len > 0)
                    {
                        memcpy(buffer + offset, " ", 1);
                        memcpy(buffer + offset + 1, std::to_string(len).c_str(), std::to_string(len).length());
                        offset += 1 + std::to_string(len).length();
                    }
                    for (size_t i = 0; i < len; i++) {
                        memcpy(buffer + offset, " ", 1);
                        std::string pos_str = std::to_string(pos[i]);
                        memcpy(buffer + offset + 1, pos_str.c_str(), pos_str.length());
                        offset += pos_str.length() + 1;
                    }
                    memcpy(buffer + offset, "\n", 1);
                    offset += 1;
                    buffer[offset] = '\0';
                    n=sendto(fd,buffer, 128 ,0 , (struct sockaddr*)&addr, addrlen);

                }
                else if (strncmp(buffer, "PWG", 3) == 0) {
                    /* Read PlayerID*/
                    char id[7];
                    memcpy(id, buffer+4, 6);
                    id[6] = '\0';
                    if(n==-1)/*error*/exit(1);
                    printf("%s\n", id);
                    char word[30];
                    memcpy(word, buffer+11, 30);
                    word[30] = '\0';
                    std::string word_str = word;
                    printf("%s\n", word);
                    char trial[2];
                    memcpy(trial, buffer+42, 1);
                    trial[1] = '\0';
                    printf("%s\n", trial);
                    int trial_int = atoi(trial);
                    std::string status;
                    if (word_str == "batata") {
                        status = "WIN";
                    }
                    else if (trial_int + 1 > 9) {
                        status = "OVR";
                    }
                    else {
                        status = "NOK";
                    }
                    memcpy(buffer, "RWG ", 4);
                    ssize_t offset = 4;
                    memcpy(buffer + offset, status.c_str(), status.length());
                    offset += status.length();
                    memcpy(buffer + offset, " ", 1);
                    offset += 1;
                    memcpy(buffer + offset, trial, 1);
                    offset += 1;
                    memcpy(buffer + offset, "\n\0", 2);
                    n=sendto(fd,buffer, 128 ,0 , (struct sockaddr*)&addr, addrlen);
                    if (n==-1)/*error*/ exit(1);

                }
                else if (strncmp(buffer, "QUT", 3) == 0) {
                    /* Read PlayerID*/
                    char id[7];
                    memcpy(id, buffer+4, 6);
                    id[6] = '\0';
                    if(n==-1)/*error*/exit(1);
                    printf("%s\n", id);
                    memcpy(buffer, "RQT ", 4);
                    ssize_t offset = 4;
                    memcpy(buffer + offset, "OK", 2);
                    offset += 2;
                    memcpy(buffer + offset, "\n\0", 2);
                    n=sendto(fd,buffer, 128 ,0 , (struct sockaddr*)&addr, addrlen);
                    if (n==-1)/*error*/ exit(1);
                }      
                else if (strncmp(buffer, "REV", 3) == 0) {
                    
                }
                else if (strncmp(buffer, "RRV", 3) == 0) {
                    
                }
                else {
                    printf("Invalid message code");
                }
                exit(0);
            }
        }
        freeaddrinfo(res);
        close(fd);
        return 0;
    }
    else
    {
        // parent process
        //servertcp();
    }
}


/* void handleCtrlC(int s){
    freeaddrinfo(res);
    close(fd);
} */