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
#include <iostream>
#include <fstream>
#include <sys/stat.h>

int fd,errcode;
socklen_t addrlen;
struct addrinfo hints,*res;
struct sockaddr_in addr;
std::string fileName;
std::string port = "58002";
std::string wordG = "batata";
bool verbose = false;

int main(int argc, char const *argv[])
{
    // Function declarations 
    // void handleCtrlC(int s);
    void readFlags(int argc, char const *argv[]);
    void readWord(std::string fileName, std::string * word);
    void startGame(std::string playerID);
    void makePlay(std::string player, char letter, int trial);
    void makeGuess(std::string playerID, std::string guess, int trial);

    ssize_t n;
    readFlags(argc, argv);
    printf("Port: %s, File: %s, Verbose: %d\n", port.c_str(), fileName.c_str(), verbose);   
    int pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }
    /* signal(SIGINT, handleCtrlC); */
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
        while (1) {
            addrlen=sizeof(addr);
            printf("Waiting for message...\n");
            n=recvfrom(fd,buffer, 128, 0, (struct sockaddr*)&addr,&addrlen);
            buffer[128] = '\0';
            if(n==-1)/*error*/exit(1);
            if (strncmp(buffer, "SNG", 3) == 0) {
                    /* Read PlayerID*/
                    char id[7];
                    memcpy(id, buffer+4, 6);
                    id[6] = '\0';
                    if(n==-1)/*error*/exit(1);
                    printf("PlayerID: %s\n", id);
                    startGame(std::string(id));
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
                makePlay(std::string(id), letter[0], trial_int);

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
                makeGuess(std::string(id), word_str, trial_int);

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

void readFlags(int argc, char const *argv[]) {
    if (argc > 0) {
        fileName = argv[1];

    }
    int argn = 1;
    while (argn < argc) {
        if (strcmp(argv[argn], "-p") == 0) {
            if (argn + 1 < argc) {
                port = argv[argn + 1];
                argn += 2;
            }
            else {
                printf("Port number not specified");
                port = "58002";
            }
        }
        else if (strcmp(argv[argn], "-v") == 0) {
            verbose = true;
            argn += 1;
        }
        else {
            argn += 1;
        }
    }
}

void readWord(std::string fileName, std::string * word) {
    std::ifstream file(fileName);
    std::string line;
    std::vector<std::string> lines;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    int lineNumber = random(0, lines.size());
    *word = stringSplit(lines[lineNumber], ' ')[0];
    std::cout << *word << std::endl;
    file.close();
}

void createGameFile(std::string playerID, std::string word) {
    std::ofstream file;
    file.open("GAMES/GAME_" + playerID);
    file << word + " " + word + ".png" << std::endl;
    file.close();
}

int verifyExistence(std::string filename) {
    std::ifstream file(filename);
    if (file.good()) {
        file.close();
        return 1;
    }
    else {
        file.close();
        return 0;
    }
}

void startGame(std::string playerID) {
    std::string word;
    readWord(fileName, &word);
    wordG = word;
    std::string status;
    std::string message;
    if (verifyExistence("GAMES/GAME_" + playerID)) {
        status = "NOK";
        message = "RSG " + status;
    }
    else {
        status = "OK";
        createGameFile(playerID, word);
        std::string letterNumber = std::to_string(word.length());
        int errorsN = maxErrors(wordG);
        std::string maxErrors = std::to_string(errorsN);
        message = "RSG " + status + " " + letterNumber + " " + maxErrors + "\n";
    }
    sendto(fd, message.c_str(), message.length(), 0, (struct sockaddr*)&addr, addrlen); // TODO : check if sends using n = 
}

void appendFile(std::string filename, std::string text) {
    std::ofstream file;
    file.open(filename, std::ios::app);
    file << text;
    file.close();
}

void savePlay(std::string playerID, std::string status, std::string play) {
    std::string filename = "GAMES/GAME_" + playerID;
    appendFile(filename, status + " " + play + "\n");
}

void makePlay(std::string playerID, char letter, int trial) {
    std::string status;
    char word[30]; // TODO : maybe change this to std::string
    strcpy(word, wordG.c_str());
    printf("%s\n", word);
    std::vector<int> pos = getPos(word, letter);
    int maxErrorsN = maxErrors(wordG);
    if (pos.size() > 0) {
        status = "OK";
    }
    else if (trial + 1 > maxErrorsN) { // TODO : fix to see only errors
        status = "OVR";
    }
    else {
        status = "NOK";
    }
    std::string message;
    message = "RLG " + status + " " + std::to_string(trial);
    size_t len = pos.size();
    if (len > 0)
    {
        message += " " + std::to_string(pos.size());
    }
    for (size_t i = 0; i < len; i++) {
        message += " " + std::to_string(pos[i]);
    }
    message += "\n";
    savePlay(playerID, "T", std::string(1, letter));
    sendto(fd,message.c_str(), message.length() ,0 , (struct sockaddr*)&addr, addrlen); // TODO : check if sends using n = 
}

void makeGuess(std::string playerID, std::string guess, int trial) {
    std::string status;
    if (guess == wordG) {
        status = "WIN";
    }
    else if (trial + 1 > 9) {
        status = "OVR";
    }
    else {
        status = "NOK";
    }
    std::string message;
    message = "RWG " + status + " " + std::to_string(trial) + "\n";
    savePlay(playerID, "G", guess);
    sendto(fd, message.c_str(), message.length() , 0, (struct sockaddr*)&addr, addrlen); // TODO : check if sends using n =
}

