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
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include "utils.h"

int fd,errcode;
ssize_t n;
socklen_t addrlen;
struct addrinfo hints,*res;
struct sockaddr_in addr;
std::string machineIP = "127.0.0.1"; //O default é o IP da máquina -> DESKTOP-8NS8GE1
std::string port="58002";     //O default devia ser 58002

//  STOI DÁ PROBLEMAS??
//  LIMITAR NUMERO DE PORTS
//  PROBLEMA A USAR CIN?
// COTA BUFFER FICA COM LIXO DO NADA, CHECKAR ISSO

int main(int argc, char const *argv[]) {
    
    void readFlags(int argc, char const *argv[]);
    std::string getWordFromBuffer(char* buffer, std::string filename);

    FILE *scoreboard;
    FILE *state;
    char buffer[1024];           //mudar para 1024
    std::string playerID;
    std::string currentWord;
    //int gameActive = 0;
    int trial = 1; 

    readFlags(argc, argv);
    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET; //IPv4
    hints.ai_socktype=SOCK_DGRAM; //UDP socket
    errcode=getaddrinfo(machineIP.c_str(),port.c_str(),&hints,&res);
    if(errcode!=0) /*error*/ exit(1);

    while(1) {
    
        std::string word; 
        std::cin >> word;
        
        if(strcmp(word.c_str(),"start") == 0 or strcmp(word.c_str(),"sg") ==0) {
            fd=socket(AF_INET,SOCK_DGRAM,0); // create UDP socket
            if(fd==-1) exit(1);

            memset(&hints,0,sizeof hints);
            hints.ai_family=AF_INET; //IPv4
            hints.ai_socktype=SOCK_DGRAM; //UDP socket

            // Read playerID 
            std::string id;
            std::cin >> id;

            /* if(id[0] == '1' or strlen(id) != 6) {
                printf("Invalid playerID. Please make sure that your playerID starts with '0' and has six digits.\n");
                continue;
            } */
        
            // Save playerID 
            playerID = id;

            // Create message 
            std::string messageToSend;
            messageToSend = "SNG " + playerID + "\n";

            // Send message
            n = sendto(fd, messageToSend.c_str(), messageToSend.length(), 0, res->ai_addr, res->ai_addrlen);
            if (n == -1) exit(1);
            
            // Empty buffer 
            memset(buffer, 0 , 1024);

            // Receive GS with the status, checking if the player can start a game 
            n = recvfrom(fd, buffer, 1024, 0, (struct sockaddr*)&addr, (socklen_t*)&res->ai_addrlen);
            if (n == -1) exit(1);
            // Remove \n from last position
            buffer[n-1] = '\0';

            // Split the buffer information into different words 
            std::vector <std::string> parameters = stringSplit(std::string(buffer), ' ');

            // Empty buffer 
            memset(buffer, 0 , 1024);
            // If game is good to go, create word, else print error message
            if (strcmp(parameters[1].c_str(), "OK") == 0) {
                // Create empty  word
                std::string wordSpaces = repeat("_ ", atoi(parameters[2].c_str()));
                wordSpaces[wordSpaces.length()-1] = '\0';
                // Save word
                currentWord = wordSpaces;
                
                printf("New game started. Guess a %s letter word: %s. You can miss up to %s times.\n", parameters[2].c_str(), wordSpaces.c_str(), parameters[3].c_str());
                //gameActive = 1;
                close(fd);
            }
            else {
                printf("You can't start a new game. You have to wait for the current game to finish.\n");
                close(fd);
            }
        }
        else if(strcmp(word.c_str(),"play") == 0 or strcmp(word.c_str(),"pl") ==0) {
            fd=socket(AF_INET,SOCK_DGRAM,0); // create UDP socket
            if(fd==-1) exit(1);

            memset(&hints,0,sizeof hints);
            hints.ai_family=AF_INET; //IPv4
            hints.ai_socktype=SOCK_DGRAM; //UDP socket

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
            
            /* Create message to send*/
            std::string messageToSend;
            messageToSend = "PLG " + playerID + " " + letter + " " + std::to_string(trial) + "\n";

            n = sendto(fd, messageToSend.c_str(), messageToSend.length(), 0, res->ai_addr, res->ai_addrlen);
            if (n == -1) exit(1);
            
            /* Empty buffer */
            memset(buffer, 0 , 1024);

            /* Receive status from GS to check if it is a hit, miss, e.t.c */
            n = recvfrom(fd, buffer, 1024, 0, (struct sockaddr*)&addr, (socklen_t*)&res->ai_addrlen);
            if (n == -1) exit(1);
            buffer[n-1] = '\0';
            
            /* Split the buffer information into different words */
            std::vector <std::string> parameters = stringSplit(std::string(buffer), ' ');

            // Empty buffer 
            memset(buffer, 0 , 1024);

            /* If ok, replace the letters in the given positions */
            if (strcmp(parameters[1].c_str(), "OK") == 0) {
                for (int i=0; i<atoi(parameters[3].c_str()); i++) {
                    int pos;
                    pos = atoi(parameters[4+i].c_str());
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
            }
            else if (strcmp(parameters[1].c_str(), "OVR") == 0) {
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
            if(fd==-1) exit(1);

            memset(&hints,0,sizeof hints);
            hints.ai_family=AF_INET; //IPv4
            hints.ai_socktype=SOCK_DGRAM; //UDP socket

            // Read word to be guessed 
            std::string guessedWord;
            std::cin >> guessedWord;

            if(guessedWord.length()<3 or guessedWord.length()>30) {
                printf("Invalid word. Please make sure that your word has between 3 and 30 characters.\n");
                continue;
            }
                
            if(playerID.length() == 0) {
                printf("You have to start a game before trying to guess the word.\n");
                continue;
            }

            //Create message
            std::string messageToSend;
            messageToSend = "PWG " + playerID + " " + guessedWord + " " + std::to_string(trial) + "\n";
            
            //Send message
            n = sendto(fd, messageToSend.c_str(), messageToSend.length(), 0, res->ai_addr, res->ai_addrlen);
            if (n == -1) exit(1);
            
            // Empty buffer 
            memset(buffer, 0 , 1024);

            // Receive status from GS to check if it is a hit, miss, e.t.c 
            n = recvfrom(fd, buffer, 1024, 0, (struct sockaddr*)&addr, (socklen_t*)&res->ai_addrlen);
            if (n == -1) //error exit(1);
            buffer[n-1] = '\0';
            
            // Split the buffer information into different words 
            std::vector <std::string> parameters = stringSplit(std::string(buffer), ' ');


            if (strcmp(parameters[1].c_str(), "WIN") == 0) { 
                printf("Great job! You've gessed the right word.\n");
                trial = 1;
            }
            else if (strcmp(parameters[1].c_str(), "NOK") == 0) {
                printf("The word %s is not the right word.\n", guessedWord.c_str());
                trial += 1;
            }
            else if (strcmp(parameters[1].c_str(), "OVR") == 0) {
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
            fd=socket(AF_INET,SOCK_STREAM,0); // create TCP socket
            if(fd==-1) /*error*/exit(1);
            
            memset(&hints,0,sizeof hints);
            hints.ai_family=AF_INET; //IPv4
            hints.ai_socktype=SOCK_STREAM; //TCP socket
            
            int fSize = 0;
            int iterationSize = 4;
            int wordsRead = 0;
            //BUG DO DEMON APARECE LIXO NO BUFFER DO START GAME
            char status[5];
            memset(status, 0, 6);
            std::string sizeOfFile;
            std::string filename;
            int  i = 0;
            int checkExit = 0;

            /* Empty buffer */
            memset(buffer, 0 , 1024);

            /* Create message to send to GS */
            memcpy(buffer, "GSB", 3);
            memcpy(buffer+3, "\n", 1);

            n = connect(fd, res->ai_addr, res->ai_addrlen);
            if (n == -1) /*error*/ exit(1);

            int nw = 0;
            char * ptr = buffer;
            size_t nn = strlen(buffer);
            while(nn>0) {
                if ((nw=write(fd, ptr, nn))<=0) exit(1);
                nn -= nw;
            }
            if (n == -1) /*error*/ exit(1);
            
            while((n = read(fd, buffer, iterationSize)) != 0) {
                if (n == -1)  exit(1);
                //printf("%s %d\n", buffer, buffer[0]);
                //Da primeira vez lê apenas "RSB "
                if (wordsRead == 0) { 
                    wordsRead+=1;
                    iterationSize = 1;
                }
                //Ler palavra status
                else if (wordsRead == 1) {
                    if (buffer[0] != 32 && buffer[0] != 10){
                        memcpy(status+i, buffer, 1);
                        i+=1;
                    }
                    else {
                        //printf("%s\n", status);
                        if(strcmp(status,"EMPTY") == 0) {
                            printf("The scoreboard is still empty.\n");
                            checkExit = 1;
                        }
                        else if(strcmp(status, "OK") != 0 && strcmp(status, "EMPTY") != 0) {
                            printf("Something went wrong. Please try again.\n");
                            close(fd);
                            exit(1);
                        }
                        else if (strcmp(status, "OK") == 0) {
                            wordsRead+=1;
                            i = 0; 
                        }
                    }
                }
                // Ler nome do ficheiro
                else if (wordsRead == 2) {
                    if (buffer[0] != 32) {
                        filename += buffer;
                        continue;
                    } else {
                        wordsRead+=1;
                        i = 0;
                    }
                }
                // Ler tamanho do ficheiro
                else if (wordsRead == 3) {
                    if (buffer[0] != 32) {
                        sizeOfFile += buffer;
                        continue;
                    } else {
                        wordsRead+=1;
                        fSize = atoi(sizeOfFile.c_str());
                        iterationSize = 256;
                        scoreboard = fopen(filename.c_str(), "w");
                    }   
                }
                // Ler o ficheiro
                else if (wordsRead == 4) {
                    //printf("%d %ld\n", fSize, n);
                    //Última leitura
                    if (fSize < 256) {
                        buffer[n-4]='\0';
                        checkExit = 1;
                        fprintf(scoreboard, "%s", buffer);
                        printf("%s\n", buffer); 
                        fclose(scoreboard);  
                        break;
                    } else {
                        fSize -= 256;
                        if (fSize < 256) {
                            iterationSize = fSize;
                        }
                        fprintf(scoreboard, "%s", buffer);
                        printf("%s", buffer); 
                    }
                }
                //printf("%d\n", wordsRead);
                memset(buffer,0,1024);
            }  
            close(fd);
        }
        else if(strcmp(word.c_str(),"hint") == 0 or strcmp(word.c_str(),"h") ==0) {
            fd=socket(AF_INET,SOCK_STREAM,0); // create TCP socket
            if(fd==-1) /*error*/exit(1);
            
            memset(&hints,0,sizeof hints);
            hints.ai_family=AF_INET; //IPv4
            hints.ai_socktype=SOCK_STREAM; //UDP socket
            
            if(playerID.length() == 0) {
                printf("You have to start a game first.\n");
                continue;
            }

            int fSize = 0;
            int iterationSize = 4;
            int wordsRead = 0;
            //BUG DO DEMON APARECE LIXO NO BUFFER DO START GAME
            char status[5];
            memset(status, 0, 1024);
            std::string sizeOfFile;
            std::string filename;
            int checkExit = 0;
            int i = 0;

            /* Empty buffer */
            memset(buffer, 0 , 1024);

            /* Create message to send to GS */
            memcpy(buffer, "GHL", 3);
            memcpy(buffer+3, " ", 1);
            memcpy(buffer+4, playerID.c_str(), 6);
            memcpy(buffer+10, "\n", 1);

            n = connect(fd, res->ai_addr, res->ai_addrlen);
            if (n == -1) /*error*/ exit(1);

            n = write(fd, buffer, strlen(buffer));
            if (n == -1) /*error*/ exit(1);
            
            memset(buffer, 0 , 1024);

            while((n = read(fd, buffer, iterationSize)) != 0) {
                if (n == -1)  exit(1);
                //printf("%s\n", buffer);
                //Da primeira vez lê apenas "RSB "
                if (wordsRead == 0) { 
                    wordsRead+=1;
                    iterationSize = 1;
                }
                else if (wordsRead == 1) {
                    if (buffer[0] != 32 && buffer[0] != 10){
                        memcpy(status+i, buffer, 1);
                        i+=1;
                    }
                    else {
                        printf("%s\n", status);
                        if(strcmp(status,"NOK") == 0) {
                            printf("Something went wrong. Please try again.\n");
                            checkExit = 1;
                        }
                        else {
                        //STATUS = FIN OU ACT
                        //printf("%s", status);
                            wordsRead+=1;
                            iterationSize = 1; 
                            i = 0;
                        }
                    }
                }
                // Ler nome do ficheiro
                else if (wordsRead == 2) {
                    if (buffer[0] != 32) {
                        filename += buffer;
                        continue;
                    } else {
                        wordsRead+=1;                    
                    }
                }
                // Ler tamanho do ficheiro
                else if (wordsRead == 3) {
                    if (buffer[0] != 32) {
                        sizeOfFile += buffer;
                        continue;
                    } else {
                        wordsRead+=1;
                        fSize = atoi(sizeOfFile.c_str());
                        //printf("%d\n", fSize);
                        iterationSize = 128;
                        state = fopen(filename.c_str(), "w");
                    }   
                }
                // Ler o ficheiro
                else if (wordsRead == 4) {
                    //printf("%d %ld %d\n", fSize, n, iterationSize);
                    //Última leitura
                    if (fSize < 128) {
                        buffer[n-1]='\0';
                        checkExit = 1;
                        fprintf(state, "%s", buffer);
                        memset(buffer,0,1024);
                        printf("%s\n", buffer); 
                        fclose(state);  
                        break;
                    } else {
                        fSize -= n;
                        if (fSize < 128) {
                            iterationSize = fSize;
                        }
                        std::ofstream filebro(("a"+filename).c_str() , std::ios::app);
                        filebro << buffer;
                        fprintf(state, "%s", buffer);
                        printf("%s\n", buffer);
                        printf("%ld", strlen(buffer)); 
                        memset(buffer,0,1024);
                    }
                }
                //printf("%d\n", wordsRead);
                memset(buffer,0,1024);
            }   
            close(fd);
        }
        //FALTA VER CASOS DE EXIT
        else if(strcmp(word.c_str(),"state") == 0 or strcmp(word.c_str(),"st") ==0) {
            fd=socket(AF_INET,SOCK_STREAM,0); // create TCP socket
            if(fd==-1) /*error*/exit(1);

            memset(&hints,0,sizeof hints);
            hints.ai_family=AF_INET; //IPv4
            hints.ai_socktype=SOCK_STREAM; //TCP socket

            if(playerID.length() == 0) {
                printf("You have to start a game first.\n");
                continue;
            }

            int fSize = 0;
            int iterationSize = 4;
            int wordsRead = 0;
            char status[3];
            std::string sizeOfFile;
            std::string filename;
            int checkExit = 0;

            /* Empty buffer */
            memset(buffer, 0 , 1024);

            /* Create message to send to GS */
            memcpy(buffer, "STA", 3);
            memcpy(buffer+3, " ", 1);
            memcpy(buffer+4, playerID.c_str(), 6);
            memcpy(buffer+10, "\n", 1);
            
            //printf("%s", buffer);
            n = connect(fd, res->ai_addr, res->ai_addrlen);
            if (n == -1) /*error*/ exit(1);
            n = write(fd, buffer, strlen(buffer));
            if (n == -1) /*error*/ exit(1);
            
            while((n = read(fd, buffer, iterationSize)) != 0) {
                if (n == -1)  exit(1);
                //printf("%s %d\n", buffer, buffer[0]);
                //Da primeira vez lê apenas "RST "
                if (wordsRead == 0) { 
                    wordsRead+=1;
                }
                //Ler palavra status
                else if (wordsRead == 1) { 
                    if(strcmp(status,"NOK") == 0) {
                        printf("You haven't completed a game yet. To play start a game type: 'sg (yourID)'\n");
                        checkExit = 1;
                    }
                    else {
                        //STATUS = FIN OU ACT
                        //printf("%s", status);
                            wordsRead+=1;
                            iterationSize = 1; 
                    }
                }
                // Ler nome do ficheiro
                else if (wordsRead == 2) {
                    if (buffer[0] != 32) {
                        filename += buffer;
                        continue;
                    } else {
                        wordsRead+=1;
                    }
                }
                // Ler tamanho do ficheiro
                else if (wordsRead == 3) {
                    if (buffer[0] != 32) {
                        sizeOfFile += buffer;
                        continue;
                    } else {
                        wordsRead+=1;
                        fSize = atoi(sizeOfFile.c_str());
                        iterationSize = 256;
                        scoreboard = fopen(filename.c_str(), "w");
                    }   
                }
                // Ler o ficheiro
                else if (wordsRead == 4) {
                    //printf("%d %ld\n", fSize, n);
                    //Última leitura
                    if (fSize < 256) {
                        buffer[n-2]='\0';
                        checkExit = 1;
                        fprintf(scoreboard, "%s", buffer);
                        printf("%s\n", buffer); 
                        fclose(scoreboard);  
                        break;
                    } else {
                        fSize -= 256;
                        if (fSize < 256) {
                            iterationSize = fSize;
                        }
                        fprintf(scoreboard, "%s", buffer);
                        printf("%s", buffer); 
                    }
                }
                //printf("%d\n", wordsRead);
                //printf("%d", wordsRead);
                memset(buffer,0,1024);
            }  
            close(fd);
        }
        else if(strcmp(word.c_str(),"quit") == 0) {
            fd=socket(AF_INET,SOCK_DGRAM,0); // create UDP socket
            if(fd==-1) /*error*/exit(1);

            memset(&hints,0,sizeof hints);
            hints.ai_family=AF_INET; //IPv4
            hints.ai_socktype=SOCK_DGRAM; //UDP socket

            /*
            if(gameActive == 0) {
                printf("There's no game undergoing. If you wish to start a game, type: 'sg (yourID)'\n");
                continue;
            }*/
            
            // Create message 
            std::string messageToSend;
            messageToSend = "QUT " + playerID + "\n";

            // Send message 
            n = sendto(fd, messageToSend.c_str(), messageToSend.length(), 0, res->ai_addr, res->ai_addrlen);
            if (n == -1) /*error*/ exit(1);

            // Empty buffer 
            memset(buffer, 0 , 1024);

            // Receive GS with the status, checking if a game is underway 
            n = recvfrom(fd, buffer, 1024, 0, (struct sockaddr*)&addr, (socklen_t*)&res->ai_addrlen);
            if (n == -1) exit(1);
            buffer[n-1] = '\0';
            
            // Split the buffer information into different words 
            std::vector <std::string> parameters = stringSplit(std::string(buffer), ' ');

            if(strcmp(parameters[1].c_str(), "ERR") == 0){
                printf("Somethign went wrong. Please try again.\n");            }
            else if (strcmp(parameters[1].c_str(), "OK") == 0) {
                printf("You have successfully quit the game.\n");
                //gameActive = 0;
            }
            else {
                exit(1);
            }
            close(fd);
        }
        else if(strcmp(word.c_str(),"exit") == 0) {
            fd=socket(AF_INET,SOCK_DGRAM,0); // create UDP socket
            if(fd==-1) exit(1);

            memset(&hints,0,sizeof hints);
            hints.ai_family=AF_INET; //IPv4
            hints.ai_socktype=SOCK_DGRAM; //UDP socket

            /*
            if(gameActive == 0) {
                printf("There's no game undergoing. If you wish to start a game, type: 'sg (yourID)'\n");
                freeaddrinfo(res);
                close(fd);
                exit(0);
            }
            */

            //Create message
            std::string messageToSend;
            messageToSend = "QUT " + playerID + "\n";

            n = sendto(fd, messageToSend.c_str(), messageToSend.length(), 0, res->ai_addr, res->ai_addrlen);
            if (n == -1) exit(1);

            // Empty buffer 
            memset(buffer, 0 , 1024);

            // Receive GS with the status, checking if a game is underway 
            n = recvfrom(fd, buffer, 1024, 0, (struct sockaddr*)&addr, (socklen_t*)&res->ai_addrlen);
            if (n == -1) exit(1);
            buffer[n-1] = '\0';
            
            // Split the buffer information into different words 
            std::vector <std::string> parameters = stringSplit(std::string(buffer), ' ');

            if(strcmp(parameters[1].c_str(), "ERR") == 0){
                printf("Something went wrong. Please try again.\n");              }
            else if (strcmp(parameters[1].c_str(), "OK") == 0) {
                printf("You have successfully quit the game.\n");
                //gameActive = 0;
                freeaddrinfo(res);
                close(fd);
                exit(0);
            }
            else {
                exit(1);
            }
            close(fd);
        }
        else {
            printf("Invalid command, please try again.\n");
            // Read useless information from line (if there are more than two arguments) and clear buffer
            printf("%s\n", buffer);
            exit(0);
            fgets(buffer, 1024, stdin);
            memset(buffer, 0 , 1024);
            continue;
        }
    }
    //freeaddrinfo(res);
    //close(fd);
    //exit(0);
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

std::string getWordFromBuffer(char* buffer, std::string filename){
    for (int j = 0; j < n; j++) {
        if (buffer[j] == ' ') {
            filename = std::string(buffer).substr(0, j);
            break;
        }
    }
    return filename;
}
