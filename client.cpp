#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#define PORT "58001"

int fd,errcode;
ssize_t n;
socklen_t addrlen;
struct addrinfo hints,*res;
struct sockaddr_in addr;
char buffer[1024];

int main(void) {
    /* Ir incrementando o trial à medida que vamos adivinhando?*/
    int trial = 0;

    fd=socket(AF_INET,SOCK_DGRAM,0); //UDP socket
    if(fd==-1) /*error*/exit(1);
    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET; //IPv4
    hints.ai_socktype=SOCK_DGRAM; //UDP socket
    //DESKTOP-8NS8GE1
    errcode=getaddrinfo("127.0.0.1","58004",&hints,&res);
    if(errcode!=0) /*error*/ exit(1);
    
    while(1) {
        char word[10]; // ver tamanho da primeira palavra //
        scanf("%s", word);
        if(strcmp(word,"start") == 0 or strcmp(word,"sg") ==0) {
            /* Read first word. Assumir que o playerID é sempre o mesmo e declarar como external? */
            char playerID[6];
            scanf("%s", playerID);
            /* Create message to send to GS */
            memcpy(buffer, "SNG", 3);
            memcpy(buffer+3, " ", 1);
            memcpy(buffer+4, playerID, 6);
            memcpy(buffer+10, "\n", 1);
            n = sendto(fd, buffer, 128, 0, res->ai_addr, res->ai_addrlen);
            if (n == -1) /*error*/ exit(1);
            /* Receive GS with the status, checking if the player can start a game */
            n = recvfrom(fd, buffer, 128, 0, (struct sockaddr*)&addr, (socklen_t*)&res->ai_addrlen);
            if (n == -1) /*error*/ exit(1);
        }
        if(strcmp(word,"play") == 0 or strcmp(word,"pl") ==0) {
            /* Read first word */
            char letter[1];
            scanf("%s", letter);
            /*Receive Player id form server*/
            char playerID[6];
            memcpy(playerID, "199222", 6);
            /* Increment trial (Recieve information from GS?) */
            char newTrial[1];
            trial += 1; // Há casos especiais - Rever
            newTrial[0] = (trial) + '0';
            /* Create message to send to GS */
            memcpy(buffer, "PLG", 3);
            memcpy(buffer+3, " ", 1);
            memcpy(buffer+4, playerID, 6);
            memcpy(buffer+10, " ", 1);
            memcpy(buffer+11, letter, 1);
            memcpy(buffer+12, " ", 1);
            memcpy(buffer+13, newTrial, 1);
            memcpy(buffer+14, "\n", 1);
            n = sendto(fd, buffer, 1024, 0, res->ai_addr, res->ai_addrlen);
            if (n == -1) /*error*/ exit(1);
            /* Receive status from GS to check if it is a hit, miss, e.t.c */
            n = recvfrom(fd, buffer, 1024, 0, (struct sockaddr*)&addr, (socklen_t*)&res->ai_addrlen);
            if (n == -1) /*error*/ exit(1);
            printf("%s", buffer);

        }
        if(strcmp(word,"guess") == 0 or strcmp(word,"gw") ==0) {
            /* Read first word */
            char guessedWord[30]; //ver tamanho da palavra a adivinhar, fazer com string?
            scanf("%s", guessedWord);
            /*Receive Player id form server*/
            char playerID[6];
            memcpy(playerID, "199222", 6);
            /* Increment trial (Recieve information from GS?) */
            char newTrial[1];
            trial += 1; //Há casos especiais - Rever
            newTrial[0] = (trial) + '0';
            /* Create message to send to GS */
            memcpy(buffer, "PWG", 3);
            memcpy(buffer+3, " ", 1);
            memcpy(buffer+4, playerID, 6);
            memcpy(buffer+10, " ", 1);
            memcpy(buffer+11, guessedWord, 30);
            memcpy(buffer+42, " ", 1);
            memcpy(buffer+43, newTrial, 1);
            memcpy(buffer+44, "\n", 1);
            n = sendto(fd, buffer, 128, 0, res->ai_addr, res->ai_addrlen);
            if (n == -1) /*error*/ exit(1);
            /* Receive status from GS to check if it is a hit, miss, e.t.c */
            n = recvfrom(fd, buffer, 128, 0, (struct sockaddr*)&addr, (socklen_t*)&res->ai_addrlen);
            if (n == -1) /*error*/ exit(1);
            printf("%s", buffer);
            
        }
        if(strcmp(word,"scoreboard") == 0 or strcmp(word,"sb") ==0) {
            /* Create message to send to GS */
            memcpy(buffer, "GSB", 3);
            memcpy(buffer+3, "\n", 1);
            n = sendto(fd, buffer, 1024, 0, res->ai_addr, res->ai_addrlen);
            if (n == -1) /*error*/ exit(1);
            /* Receive status from GS, the file */
            n = recvfrom(fd, buffer, 1024, 0, (struct sockaddr*)&addr, (socklen_t*)&res->ai_addrlen);
            if (n == -1) /*error*/ exit(1);
        }
        if(strcmp(word,"hint") == 0 or strcmp(word,"h") ==0) {
            /*Receive Player id form server*/
            char playerID[6];
            memcpy(playerID, "199222", 6);
            /* Create message to send to GS */
            memcpy(buffer, "GHL", 3);
            memcpy(buffer+3, " ", 1);
            memcpy(buffer+4, playerID, 6);
            memcpy(buffer+10, "\n", 1);
            n = sendto(fd, buffer, 1024, 0, res->ai_addr, res->ai_addrlen);
            if (n == -1) /*error*/ exit(1);
            /* Receive GS with the status, checking if the player can start a game */
            n = recvfrom(fd, buffer, 1024, 0, (struct sockaddr*)&addr, (socklen_t*)&res->ai_addrlen);
            if (n == -1) /*error*/ exit(1);

        }
        if(strcmp(word,"state") == 0 or strcmp(word,"st") ==0) {
            /*Receive Player id form server*/
            char playerID[6];
            memcpy(playerID, "199222", 6);
            /* Create message to send to GS */
            memcpy(buffer, "STA", 3);
            memcpy(buffer+3, " ", 1);
            memcpy(buffer+4, playerID, 6);
            memcpy(buffer+10, "\n", 1);
            n = sendto(fd, buffer, 1024, 0, res->ai_addr, res->ai_addrlen);
            if (n == -1) /*error*/ exit(1);
            /* Receive GS with the status, checking if the player can start a game */
            n = recvfrom(fd, buffer, 1024, 0, (struct sockaddr*)&addr, (socklen_t*)&res->ai_addrlen);
            if (n == -1) /*error*/ exit(1);
        }
        if(strcmp(word,"quit") == 0) {
            /*Receive Player id form server*/
            char playerID[6];
            memcpy(playerID, "199222", 6);
            /* Create message to send to GS */
            memcpy(buffer, "QUT", 3);
            memcpy(buffer+3, " ", 1);
            memcpy(buffer+4, playerID, 6);
            memcpy(buffer+10, "\n", 1);
            n = sendto(fd, buffer, 1024, 0, res->ai_addr, res->ai_addrlen);
            if (n == -1) /*error*/ exit(1);
            /* Receive GS with the status, checking if a game is underway */
            n = recvfrom(fd, buffer, 1024, 0, (struct sockaddr*)&addr, (socklen_t*)&res->ai_addrlen);
            if (n == -1) /*error*/ exit(1);
            printf("%s", buffer);
        }
        if(strcmp(word,"exit") == 0) {
            /*Receive Player id form server*/
            char playerID[6];
            memcpy(playerID, "199222", 6);
            /* Create message to send to GS */
            memcpy(buffer, "QUT", 3);
            memcpy(buffer+3, " ", 1);
            memcpy(buffer+4, playerID, 6);
            memcpy(buffer+10, "\n", 1);
            n = sendto(fd, buffer, 1024, 0, res->ai_addr, res->ai_addrlen);
            if (n == -1) /*error*/ exit(1);
            freeaddrinfo(res);
            close(fd);
            exit(0);
        }
    }
    
    
    //n=sendto(fd,"SNG 199222",10,0,res->ai_addr,res->ai_addrlen);
    //if(n==-1) /*error*/ exit(1);
    //addrlen=sizeof(addr);
    //n=recvfrom(fd,buffer,128,0,(struct sockaddr*)&addr,&addrlen);
    //if(n==-1) /*error*/ exit(1);
    //write(1,"echo: ",6); write(1,buffer,n);
    
    freeaddrinfo(res);
    close(fd);
    exit(0);
}