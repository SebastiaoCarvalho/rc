#include "utils.h"
#include "filehandling.h"
#include "serverModule/tcp.h"
#include "serverModule/common.h"
#include "serverModule/udp.h"
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
#include <string>
#include <signal.h>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <sstream>

// Global variables
int fd;                // Socket file descriptor
struct addrinfo hints,*res; // Socket address info

int main(int argc, char const *argv[])
{
    // Function declarations 
    void readFlags(int argc, char const *argv[], std::string  * fileName, std::string  * port, bool  * verbose, bool * sequentialRead);
    void bootServer();
    void handleCtrlC(int s);
    void createDirs();

    // Variable declarations
    ssize_t n;
    struct sigaction act;
    int newfd, errcode, readArg = 0; // readArg is either a seed or lineNumber depending on sequentialRead
    socklen_t addrlen;
    struct sockaddr_in addr;
    std::string fileName;
    std::string port = "58002"; // default port
    bool verbose = false; // default verbose
    bool sequentialRead = false; // default read is random

    readFlags(argc, argv, &fileName, &port, &verbose, &sequentialRead);
    createDirs();
    bootServer();
    if (verbose) {
        printf("Port: %s, File: %s, Verbose: %d, Read: %s\n", port.c_str(), fileName.c_str(), verbose, sequentialRead ? "Sequential":"Random");
    }   
    int pid = fork();
    if (pid == -1) {
        if (verbose) {
            printf("Error forking process. Please restart server.\n");
        }
        exitServer(1, fd, res);
    }
    signal(SIGINT, handleCtrlC);
    memset(&act,0,sizeof act);
    act.sa_handler=SIG_IGN;
    if(sigaction(SIGPIPE,&act,NULL)==-1)/*error*/exit(1);
    if (pid > 0)
    {

        char buffer[129];
        // child process
        fd=socket(AF_INET,SOCK_DGRAM,0); //UDP socket
        if(fd==-1) /*error*/exitServer(1, fd, res);
        int iSetOption = 1;
        n = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&iSetOption, sizeof(iSetOption));
        if (n == -1) exitServer(1, fd, res);
        memset(&hints,0,sizeof hints);
        hints.ai_family=AF_INET; // IPv4
        hints.ai_socktype=SOCK_DGRAM; // UDP socket
        hints.ai_flags=AI_PASSIVE;
        errcode=getaddrinfo(NULL, port.c_str(),&hints,&res);
        if(errcode!=0) /*error*/ exitServer(errcode, fd, res);
        n=bind(fd,res->ai_addr, res->ai_addrlen);
        if(n==-1) /*error*/ exitServer(1, fd, res);
        while (1) {
            addrlen=sizeof(addr);
            if (verbose) {
                printf("Waiting for UDP request ...\n");
            }
            memset(buffer,0,128);
            n=recvfrom(fd,buffer, 128, 0, (struct sockaddr*)&addr,&addrlen);
            if(n==-1)/*error*/exitServer(1, fd, res);
            buffer[128] = '\0';
            std::string message = buffer;
            message = strip(message);
            std::vector<std::string> tokens = stringSplit(message, ' ');
            if (strncmp(buffer, "SNG", 3) == 0) { // SNG command
                if (tokens.size() != 2) {
                    sendUDP(fd, "RSG ERR\n", 8, addr, addrlen);
                    continue;
                }
                /* Read PlayerID*/
                std::string playerID = tokens[1];
                int errcond = playerID.length() != 6  || ! isNumber(playerID);
                if (errcond) {
                    sendUDP(fd, "RSG ERR\n", 8, addr, addrlen);
                    continue;
                }
                if (verbose) {
                    printf("Received SNG with playerID: %s from IP address: %s and port: %d\n", playerID.c_str(), inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
                }
                startGame(playerID, sequentialRead, &readArg, fileName, fd, addr, addrlen, verbose);
            }
            else if (strncmp(buffer, "PLG", 3) == 0) { // PLG command
                if (tokens.size() != 4) {
                    sendUDP(fd, "RLG ERR\n", 8, addr, addrlen);
                    continue;
                }
                /* Read PlayerID*/
                std::string playerID = tokens[1];
                int errCond = playerID.length() != 6  || ! isNumber(playerID) || tokens[2].length() != 1 || ! isNumber(tokens[3]);
                /*Read letter*/
                if (errCond ) {
                    sendUDP(fd, "RLG ERR\n", 8, addr, addrlen);
                    continue;
                }
                char letter = tokens[2][0];
                /*Read Trial*/
                std::string trial = tokens[3];
                if (verbose) {
                    printf("Received PLG with playerID: %s, letter: %c, trial: %s from IP address: %s and port: %d\n", playerID.c_str(), letter, trial.c_str(), inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
                }
                int trial_int = atoi(trial.c_str());
                makePlay(playerID, letter, trial_int, fd, addr, addrlen, verbose);

            }
            else if (strncmp(buffer, "PWG", 3) == 0) { // PWG command
                if (tokens.size() != 4) {
                    sendUDP(fd, "RLG ERR\n", 8, addr, addrlen);
                    continue;
                }
                /* Read PlayerID*/
                std::string playerID = tokens[1];
                std::string word = tokens[2];
                /*Read Trial*/
                std::string trial = tokens[3];
                int errCond = playerID.length() != 6  || ! isNumber(playerID)  || ! isNumber(tokens[3]);
                /*Read letter*/
                if (errCond ) {
                    sendUDP(fd, "RLG ERR\n", 8, addr, addrlen);
                    continue;
                }

                
                if (verbose) {
                    printf("Received PWG with playerID: %s, word: %s, trial: %s from IP address: %s and port: %d\n", playerID.c_str(), word.c_str(), trial.c_str(), inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
                }
                int trial_int = atoi(trial.c_str());
                makeGuess(playerID, word, trial_int, fd, addr, addrlen, verbose);

            }
            else if (strncmp(buffer, "QUT", 3) == 0) { // QUT command
                if (tokens.size() != 2) {
                    sendUDP(fd, "RQT ERR\n", 8, addr, addrlen);
                    continue;
                }
                /* Read PlayerID*/
                std::string playerID = tokens[1];
                int errCond = playerID.length() != 6  || ! isNumber(playerID);
                if (errCond) {
                    sendUDP(fd, "RQT ERR\n", 8, addr, addrlen);
                    continue;
                }
                if (verbose) {
                    printf("Received QUT with playerID: %s from IP address: %s and port: %d\n", playerID.c_str(), inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
                }
                quitGame(playerID, fd, addr, addrlen, verbose);

            }      
            else if (strncmp(buffer, "REV", 3) == 0) {
                // Not implemented
            }
            else {
                if (verbose) {
                    printf("Received invalid message from IP address: %s and port: %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
                }
                sendUDP(fd, "ERR\n", 8, addr, addrlen);
            }

            for (std::string word : tokens) { // free memory
                std::string().swap(word);
            }
            std::vector<std::string>().swap(tokens);
        }
        freeaddrinfo(res);
        close(fd);
        return 0;
    }
    else {

        char buffer[129];
        int ret;
        struct sigaction act2; 
        memset(&act2,0,sizeof(act2));
        act2.sa_handler=SIG_IGN;
        if(sigaction(SIGCHLD,&act2,NULL)==-1)/*error*/exitServer(1, fd, res);
        if((fd=socket(AF_INET,SOCK_STREAM,0))==-1)exitServer(1, fd, res);//error
        int iSetOption = 1;
        n = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&iSetOption, sizeof(iSetOption));
        if (n == -1) exitServer(1, fd, res);
        memset(&hints,0,sizeof hints);
        hints.ai_family=AF_INET;//IPv4
        hints.ai_socktype=SOCK_STREAM;//TCP socket
        hints.ai_flags=AI_PASSIVE;
        if ((ret=getaddrinfo(NULL, port.c_str(), &hints, &res))!=0)/*error*/exitServer(1, fd, res);
        if (bind(fd,res->ai_addr,res->ai_addrlen)==-1)/*error*/exitServer(1, fd, res);
        if (listen(fd,5)==-1)/*error*/exitServer(1, fd, res);
        while(1) {
            if (verbose) {
                printf("Waiting for TCP connection ...\n");
            }
            memset(buffer,0,128);
            addrlen=sizeof(addr);
            do newfd=accept(fd,(struct sockaddr*)&addr,&addrlen);//wait for a connection
            while(newfd==-1&&errno==EINTR);
            if(newfd==-1)/*error*/exitServer(1, fd, res);
            struct timeval timeout;      
            timeout.tv_sec = 10;
            timeout.tv_usec = 0;
            if (setsockopt (newfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout) < 0) {
                if (verbose) {
                    printf("Error on setting socket options.\n");
                }
            } 
            if((pid=fork())==-1)/*error*/exitServer(1, fd, res);
            else if(pid==0) { // child process
                
                n = readn(newfd, buffer, 4);
                if (n == -1) /*error*/ {
                    if (verbose) {
                        printf("Error on reading system call.\n");
                    }
                    close(newfd); // close connection and end request
                    exit(0);
                }
                buffer[n - 1] = '\0'; // Replace newline with null terminator
                if (strncmp(buffer, "GSB", 3) == 0) {
                    if (verbose) {
                        printf("Received GSB from IP address: %s and port: %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
                    }
                    sendScoreBoard(newfd, verbose);
                }
                else if (strncmp(buffer, "GHL", 3) == 0) {
                    n = readn(newfd, buffer, 7);
                    if (n == -1) /*error*/ {
                        if (verbose) {
                            printf("Error on reading system call.\n");
                        }
                        close(newfd); // close connection and end request
                        exit(0);
                    }
                    buffer[n - 1] = '\0'; // Replace newline with null terminator
                    std::string playerID = buffer;
                    if (verbose) {
                        printf("Received GHL with playerID: %s from IP address: %s and port: %d\n", playerID.c_str(), inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
                    }                    
                    sendHint(newfd, playerID, verbose);
                }
                else if (strncmp(buffer, "STA", 3) == 0) {
                    n = readn(newfd, buffer, 7);
                    if (n == -1) /*error*/ {
                        if (verbose) {
                            printf("Error on reading system call.\n");
                        }
                        close(newfd); // close connection and end request
                        exit(0);
                    }
                    buffer[n - 1] = '\0'; // Replace newline with null terminator
                    std::string playerID = buffer;
                    if (verbose) {
                        printf("Received STA with playerID: %s from IP address: %s and port: %d\n", playerID.c_str(), inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
                    }
                    sendState(newfd, playerID, verbose);
                }
                else {
                    if (verbose) {
                        printf("Received invalid message from IP address: %s and port: %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
                    }
                    sendTCP(newfd, "ERR\n", 4);
                }

                close(newfd); 
                exit(0);
            }
            // parent process
            do ret=close(newfd);while(ret==-1&&errno==EINTR);
            if(ret==-1)/*error*/exitServer(1, fd, res);
        }
    }
}

/* Handle Ctrl C Event (SIGINT) */
void handleCtrlC(int s){
    printf("Caught exitting signal.\n Exiting...\n");
    exitServer(s, fd, res);
}

/* Read flags from command line */
void readFlags(int argc, char const *argv[], std::string * fileName, std::string * port, bool * verbose, bool * sequentialRead) {
    if (argc == 1) {
        printf("No file name specified. Using default file name: words.txt\n");
        * fileName = "words.txt";
    }
    else {
        if (! verifyExistence(argv[1])) {
            printf("File name specified does not exist. Using default file name: words.txt\n");
            * fileName = "words.txt";
        }
        else {
            * fileName = argv[1];
        }
    }
    int argn = 1;
    while (argn < argc) {
        if (strcmp(argv[argn], "-p") == 0) {
            if (argn + 1 < argc) {
                * port = argv[argn + 1];
                argn += 2;
            }
            else {
                * port = "58002";
            }
        }
        else if (strcmp(argv[argn], "-v") == 0) {
            * verbose = true;
            argn += 1;
        }
        else if (strcmp(argv[argn], "-s") == 0) {
            * sequentialRead = true;
            argn += 1;
        }
        else {
            argn += 1;
        }
    }
}

/* Boot the server*/
void bootServer() {
    std::vector<std::string> files = listDirectory("GAMES");
    size_t size = files.size();
    for (size_t i = 0; i < size; i++) {
        int res = deleteFile("GAMES/" + files[i]);
        if (res == 0) {
            std::cout << "Error deleting file " << files[i] << std::endl;
        }
    }
    files.clear();
    std::vector<std::string>().swap(files); // free memory
    files =  listDirectory("SCORES");
    for (size_t i = 0; i < files.size(); i++) {
        int res = deleteFile("SCORES/" + files[i]);
        if (res == 0) {
            std::cout << "Error deleting file " << files[i] << std::endl;
        }
    }
    for (std::string s : files) {
        s.clear();
        s.shrink_to_fit();
    }
    std::vector<std::string>().swap(files); // free memory
}

/* Create GAMES and SCORES directories if they are not used*/
void createDirs() {
    if (! verifyExistence("GAMES")) {
        mkdir("GAMES", 0777);;
    }
    if (! verifyExistence("SCORES")) {
        mkdir("SCORES", 0777);;
    }
}




