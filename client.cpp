#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include "utils.h"


int fd,errcode;
ssize_t n;
socklen_t addrlen;
struct addrinfo hints,*res;
struct sockaddr_in addr;
char buffer[1024];
char inputFromUser[1024];
std::string playerID;
std::string currentWord;
std::string machineIP = "tejo.tecnico.ulisboa.pt"; //O default é o IP da máquina -> DESKTOP-8NS8GE1
std::string port="58011";     //O default devia ser 58002

//  RECEBER INPUTS QUANDO PROGRAMA COMEÇA 
//  STOI DÁ PROBLEMAS??
//  LIMITAR NUMERO DE PORTS
//  PROBLEMA A USAR CIN?

int main(int argc, char const *argv[]) {
    
    void readFlags(int argc, char const *argv[]);

    int trial = 1; 
    int maxErrors = 0; 

    readFlags(argc, argv);
    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET; //IPv4
    hints.ai_socktype=SOCK_DGRAM; //UDP socket
    errcode=getaddrinfo(machineIP.c_str(),port.c_str(),&hints,&res);
    if(errcode!=0) /*error*/ exit(1);

    while(1) {
        std::string word; 
        std::cin >> word;
        
        printf("%s", inputFromUser);
        if(strcmp(word.c_str(),"start") == 0 or strcmp(word.c_str(),"sg") ==0) {
            fd=socket(AF_INET,SOCK_DGRAM,0); // create UDP socket
            if(fd==-1) /*error*/exit(1);

            /* Read playerID */
            char id[6];
            scanf("%s", id);

            if(id[0] == '1' or strlen(id) != 6) {
                printf("Invalid playerID. Please make sure that your playerID starts with '0' and has six digits.\n");
                continue;
            }
        
            /* Empty buffer */
            memset(buffer, 0 , 1024);

            /* Save playerID */
            playerID = id;

            /* Create message to send to GS */
            memcpy(buffer, "SNG", 3);
            memcpy(buffer+3, " ", 1);
            memcpy(buffer+4, playerID.c_str(), 6);
            memcpy(buffer+10, "\n", 1);
            
            n = sendto(fd, buffer, strlen(buffer), 0, res->ai_addr, res->ai_addrlen);
            if (n == -1) /*error*/ exit(1);
            /* Receive GS with the status, checking if the player can start a game */
            n = recvfrom(fd, buffer, 1024, 0, (struct sockaddr*)&addr, (socklen_t*)&res->ai_addrlen);
            if (n == -1) /*error*/ exit(1);
            buffer[n-1] = '\0';
            //printf("%s", buffer);

            /* Split the buffer information into different words */
            std::vector <std::string> parameters = stringSplit(std::string(buffer), ' ');

            /* If game is good to go, create word, else print error */
            if (strcmp(parameters[1].c_str(), "OK") == 0) {
                /* Create empty  word*/
                std::string wordSpaces = repeat("_ ", stoi(parameters[2]));
                wordSpaces[wordSpaces.length()-1] = '\0';
                currentWord = wordSpaces;

                /* Remove '\n' from last maxErrors (since the server replies with \n at the end)*/
                //std::string maxErrorsStr = parameters[3].substr(0, parameters[3].length()-1);
                
                /* Declare max errors */
                maxErrors = stoi(parameters[3]);
                
                printf("New game started. Guess %s letter word: %s. You have %s attempts.\n", parameters[2].c_str(), wordSpaces.c_str(), parameters[3].c_str());
                close(fd);
            }
            else {
                printf("You can't start a new game. You have to wait for the current game to finish.\n");
                close(fd);
            }
        }
        else if(strcmp(word.c_str(),"play") == 0 or strcmp(word.c_str(),"pl") ==0) {
            fd=socket(AF_INET,SOCK_DGRAM,0); // create UDP socket
            if(fd==-1) /*error*/exit(1);

            /* Read first word */
            std::string letter;
            std::cin >> letter;

            if (letter.length() != 1) {
                printf("Invalid letter. Please make sure that you only input one letter.\n");
                continue;
            }
            

            if(playerID.length() == 0) {
                printf("You have to start a game first.\n");
                continue;
            }
            
            /*
            if('a'>letter && letter[0]<='z') {
                printf("Invalid letter. Please make sure that your letter is between 'a' and 'z'.\n");
                continue;
            }
            */
            
            
            /* Empty buffer */
            memset(buffer, 0 , 1024);
            
            /* Convert trial to string */
            std::string trialStr = std::to_string(trial);
            //printf("%d", (int)trialStr.length());
            
            //printf("%d", (int)trialStr.length());
            /* Create message to send to GS */
            memcpy(buffer, "PLG", 3);
            memcpy(buffer+3, " ", 1);
            memcpy(buffer+4, playerID.c_str(), 6);
            memcpy(buffer+10, " ", 1);
            memcpy(buffer+11, letter.c_str( ), 1);
            memcpy(buffer+12, " ", 1);
            memcpy(buffer+13, trialStr.c_str(), trialStr.length());
            memcpy(buffer+13+trialStr.length(), "\n", 1);

            //printf("%s", buffer);
            //printf("%d",(int)strlen(buffer));
            n = sendto(fd, buffer, strlen(buffer), 0, res->ai_addr, res->ai_addrlen);
            if (n == -1) /*error*/ exit(1);
            //QUANDO BUFFER RECEBE, RECEBE DADOS COM LIXO PORQUE CASO SEJA UM NOP SO ESCREVE NOS PRIMEIROS BITS
            /* Receive status from GS to check if it is a hit, miss, e.t.c */
            n = recvfrom(fd, buffer, 1024, 0, (struct sockaddr*)&addr, (socklen_t*)&res->ai_addrlen);
            if (n == -1) /*error*/ exit(1);
            buffer[n-1] = '\0';
            //printf("%s %d", buffer, (int)strlen(buffer));
            
            /* Split the buffer information into different words */
            std::vector <std::string> parameters = stringSplit(std::string(buffer), ' ');
            //printf("%s", parameters[1].c_str());
            //printf("%d", (int)parameters.size());

            /* Remove '\n' from last position (since the server replies with \n at the end)*/
            /*
            std::string lastParameterStr = parameters[parameters.size()-1]; 
            lastParameterStr[lastParameterStr.length()-1] = '\0';
            */
            /* If ok, replace the letters in the given positions */
            //POSICAO COMEÇA NO 0, PROBLEMA: O ULTIMO NUMERO VAI TER \N QUE PODE ATROFIAR COM O STOI
            if (strcmp(parameters[1].c_str(), "OK") == 0) {
                for (int i=0; i<stoi(parameters[3]); i++) {
                    //printf("%s %s", parameters[3].c_str(), parameters[4+i].c_str());
                    int pos;
                    pos = stoi(parameters[4+i]);
                    currentWord[(pos-1)*2] = letter[0];
                }
                trial += 1;
                printf("Word: %s\n", currentWord.c_str());
            }
            else if (strcmp(parameters[1].c_str(), "WIN") == 0) {
                for (int i = 0; i<(int)currentWord.length(); i++) {
                    if (currentWord[i] == '_') {
                        currentWord[i] = letter[0];
                    }
                }
                printf("Well done! You've gessed the right word: %s.\n", currentWord.c_str());
                trial = 1;
            }
            else if (strcmp(parameters[1].c_str(), "DUP") == 0) {
                printf("You have already tried that letter. Try another letter please.\n");
            }
            else if (strcmp(parameters[1].c_str(), "NOK") == 0) {
                printf("The letter %s is not part of the word: %s.\n", letter.c_str(), currentWord.c_str());
                trial += 1;
                maxErrors -= 1;
            }
            else if (strcmp(parameters[1].c_str(), "OVR") == 0) {
                //DAR PRINT À PALVRA CERTA?
                //Como parar o jogo?
                printf("You have exceeded the maximum number of errors. You've lost the game.\n");
                trial = 1; 
            }
            else if (strcmp(parameters[1].c_str(), "INV") == 0) {
                //VER O QUE É ISTO
                printf("Something went wrong. Restart the program.\n");
                exit(1);
            }
            else if (strcmp(parameters[1].c_str(), "ERR") == 0) {
                printf("The data sent is not valid. Please check if your playerID is valid. If you don't " \
                "have a game ongoing, please start one with the command: 'start (yourID)'.\n");
            }
            else {
                exit(1);
            }

            close(fd);
        }
        else if(strcmp(word.c_str(),"guess") == 0 or strcmp(word.c_str(),"gw") ==0) {
            fd=socket(AF_INET,SOCK_DGRAM,0); // create UDP socket
            if(fd==-1) /*error*/exit(1);

            /* Read word to be guessed */
            std::string guessedWord;
            std::cin >> guessedWord;

            if(guessedWord.length()<3 or guessedWord.length()>30) {
                printf("Invalid word. Please make sure that your word has between 3 and 30 characters.\n");
                continue;
            }
                
            /* Empty buffer */
            memset(buffer, 0 , 1024);
            
            /* Convert trial to string */
            std::string trialStr = std::to_string(trial);

            /* Create message to send to GS */
            memcpy(buffer, "PWG", 3);
            memcpy(buffer+3, " ", 1);
            memcpy(buffer+4, playerID.c_str(), 6);
            memcpy(buffer+10, " ", 1);
            memcpy(buffer+11, guessedWord.c_str(), guessedWord.length());
            memcpy(buffer+11+guessedWord.length(), " ", 1);
            memcpy(buffer+11+guessedWord.length()+1, trialStr.c_str(), trialStr.length());
            memcpy(buffer+11+guessedWord.length()+1+trialStr.length(), "\n", 1);
            n = sendto(fd, buffer, strlen(buffer), 0, res->ai_addr, res->ai_addrlen);
            if (n == -1) /*error*/ exit(1);
            /* Receive status from GS to check if it is a hit, miss, e.t.c */
            n = recvfrom(fd, buffer, 1024, 0, (struct sockaddr*)&addr, (socklen_t*)&res->ai_addrlen);
            if (n == -1) /*error*/ exit(1);
            buffer[n-1] = '\0';
            
            /* Split the buffer information into different words */
            std::vector <std::string> parameters = stringSplit(std::string(buffer), ' ');

            if (strcmp(parameters[1].c_str(), "WIN") == 0) {
                //COMO É QUE VOU BUSCAR A PALAVRA CERTA?   
                printf("Great job! You've gessed the right word.\n");
                trial = 1;
            }
            else if (strcmp(parameters[1].c_str(), "NOK") == 0) {
                printf("The word %s is not the right word.\n", guessedWord.c_str());
                trial += 1;
                maxErrors -= 1;
            }
            else if (strcmp(parameters[1].c_str(), "OVR") == 0) {
                //DAR PRINT À PALVRA CERTA?
                printf("You have exceeded the maximum number of errors. You've lost the game.\n");
                trial = 1; 
            }
            else if (strcmp(parameters[1].c_str(), "INV") == 0) {
                //VER O QUE É ISTO
                printf("Something went wrong. Restart the program.\n");
                exit(1);
            }
            else if (strcmp(parameters[1].c_str(), "ERR") == 0) {
                printf("The data sent is not valid. Please check if your playerID is valid. If you don't " \
                "have a game ongoing, please start one with the command: 'start (yourID)'.\n");
            }
            else {
                exit(1);
            }
            close(fd);            
        }
        else if(strcmp(word.c_str(),"scoreboard") == 0 or strcmp(word.c_str(),"sb") ==0) {
            fd=socket(AF_INET,SOCK_DGRAM,0); // create UDP socket
            if(fd==-1) /*error*/exit(1);

            /* Empty buffer */
            memset(buffer, 0 , 1024);

            /* Create message to send to GS */
            memcpy(buffer, "GSB", 3);
            memcpy(buffer+3, "\n", 1);
            n = sendto(fd, buffer, 1024, 0, res->ai_addr, res->ai_addrlen);
            if (n == -1) /*error*/ exit(1);
            /* Receive status from GS, the file */
            n = recvfrom(fd, buffer, 1024, 0, (struct sockaddr*)&addr, (socklen_t*)&res->ai_addrlen);
            if (n == -1) /*error*/ exit(1);
            close(fd);
        }
        else if(strcmp(word.c_str(),"hint") == 0 or strcmp(word.c_str(),"h") ==0) {
            fd=socket(AF_INET,SOCK_DGRAM,0); // create UDP socket
            if(fd==-1) /*error*/exit(1);
            /*Receive Player id form server*/
            char playerID[6];
            memcpy(playerID, "199222", 6);

            /* Empty buffer */
            memset(buffer, 0 , 1024);

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
            close(fd);

        }
        else if(strcmp(word.c_str(),"state") == 0 or strcmp(word.c_str(),"st") ==0) {
            fd=socket(AF_INET,SOCK_DGRAM,0); // create UDP socket
            if(fd==-1) /*error*/exit(1);

            /*Receive Player id form server*/
            char playerID[6];
            memcpy(playerID, "099222", 6);

            /* Empty buffer */
            memset(buffer, 0 , 1024);

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
            close(fd);
        }
        else if(strcmp(word.c_str(),"quit") == 0) {
            fd=socket(AF_INET,SOCK_DGRAM,0); // create UDP socket
            if(fd==-1) /*error*/exit(1);

            /* Empty buffer */
            memset(buffer, 0 , 1024);

            /* Create message to send to GS */
            memcpy(buffer, "QUT", 3);
            memcpy(buffer+3, " ", 1);
            memcpy(buffer+4, playerID.c_str(), 6);
            memcpy(buffer+10, "\n", 1);
            n = sendto(fd, buffer, strlen(buffer), 0, res->ai_addr, res->ai_addrlen);
            if (n == -1) /*error*/ exit(1);
            /* Receive GS with the status, checking if a game is underway */
            n = recvfrom(fd, buffer, 1024, 0, (struct sockaddr*)&addr, (socklen_t*)&res->ai_addrlen);
            if (n == -1) /*error*/ exit(1);
            //printf("%s", buffer);
            buffer[n-1] = '\0';
            
            /* Split the buffer information into different words */
            std::vector <std::string> parameters = stringSplit(std::string(buffer), ' ');

            if(strcmp(parameters[1].c_str(), "ERR") == 0){
                printf("You can't quit the game because you don't have a game ongoing.\n");            }
            else {
                printf("You have successfully quit the game.\n");
            }
            close(fd);
        }
        else if(strcmp(word.c_str(),"exit") == 0) {
            fd=socket(AF_INET,SOCK_DGRAM,0); // create UDP socket
            if(fd==-1) /*error*/exit(1);

            /* Empty buffer */
            memset(buffer, 0 , 1024);

            /* Create message to send to GS */
            memcpy(buffer, "QUT", 3);
            memcpy(buffer+3, " ", 1);
            memcpy(buffer+4, playerID.c_str(), 6);
            memcpy(buffer+10, "\n", 1);
            n = sendto(fd, buffer, strlen(buffer), 0, res->ai_addr, res->ai_addrlen);
            if (n == -1) /*error*/ exit(1);
            freeaddrinfo(res);
            close(fd);
            exit(0);
        }
        else {
            printf("Invalid command, please try again.\n");
            /* Read useless information  from line and clear buffer*/
            fgets(buffer, 1024, stdin);
            memset(buffer, 0 , 1024);
            continue;
        }
    }

    freeaddrinfo(res);
    close(fd);
    exit(0);
}

void readFlags(int argc, char const *argv[]) {
    int argn = 1;
    while (argn < argc) {
        if (strcmp(argv[argn],"-n") == 0) {
            if (argn + 1 < argc) {
                machineIP = argv[argn + 1];
                argn += 2;
            } else {
                machineIP = "tejo.tecnico.ulisboa.pt";
            }
        }
        else if (strcmp(argv[argn],"-p") == 0) {
            if (argn + 1 < argc) {
                port = argv[argn + 1];
                argn += 2;
            } else {
                port = "58011";
            }
        } else {
            argn += 1;
        }
    }
}
