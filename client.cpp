#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>

int main(int argc, char const *argv[])
{
    int fd,errcode;
    ssize_t n;
    // socklen_t addrlen;
    struct addrinfo hints,*res;
    struct sockaddr_in addr;

    fd=socket(AF_INET,SOCK_DGRAM,0); //UDP socket
    if(fd==-1) /*error*/exit(1);
    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET; //IPv4
    hints.ai_socktype=SOCK_DGRAM; //UDP socket
    errcode=getaddrinfo("127.0.0.1","58002",&hints,&res);
    if(errcode!=0) /*error*/ exit(1);
    n=sendto(fd,"SNG", 3 ,0 , res->ai_addr, res->ai_addrlen);
    if(n==-1) /*error*/ exit(1);
    n=sendto(fd, " ", 1, 0, res->ai_addr, res->ai_addrlen);
    if(n==-1) /*error*/ exit(1);
    n=sendto(fd, "123456", 6, 0, res->ai_addr, res->ai_addrlen);
    if(n==-1) /*error*/ exit(1);
    // addrlen=sizeof(addr);
    // n=recvfrom(fd,buffer,128,0,
    // (struct sockaddr*)&addr,&addrlen);
    // if(n==-1) /*error*/ exit(1);
    // write(1,"echo: ",6); write(1,buffer,n);
    freeaddrinfo(res);
    close(fd);

    return 0;
}
