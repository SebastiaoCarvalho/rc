//FPRINTF NÃO ESCREVE '\0'

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
#include <fcntl.h>
#include "utils.h"

int fd,errcode;
socklen_t addrlen;
struct addrinfo hints,*res;
struct sockaddr_in addr;
std::string machineIP = "127.0.0.1"; //O default é o IP da máquina -> DESKTOP-8NS8GE1 ou 127.0.0.1
std::string port="58002";     //O default devia ser 58002


//  LIMITAR NUMERO DE PORTS

int main(int argc, char const *argv[]) {
    
    void readFlags(int argc, char const *argv[]);
    void sendTCP(int fd, std::string message);
    ssize_t rcvMessageUdp(fd_set readfds, timeval tv, int fd, ssize_t n, struct sockaddr_in addr, struct addrinfo* res, char* buffer, std::string messageToSend);
    void sendAndReceiveUdpMessage(fd_set readfds, timeval tv, int fd, ssize_t n, struct sockaddr_in addr, struct addrinfo* res, char* buffer, std::string messageToSend);
    int readStatusMessageHint(std::string status, int wordsRead);
    int readStatusMessageState(std::string status, int wordsRead);
    int readStatusMessageScoreboard(std::string status, int wordsRead);
    void readMessageTcp(int fd, ssize_t n, std::string type);
    void readTcp(fd_set readfds, timeval tv, int fd, ssize_t n, std::string messageToSend, std::string type);

    char buffer[256];         
    std::string playerID;
    std::string currentWord;
    int gameActive = 0;
    int trial = 1; 
    ssize_t n;
    fd_set readfds;  
    struct timeval tv;

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

            // Set socket to non-blocking
            fcntl(fd, F_SETFL, O_NONBLOCK);
            
            // Read playerID 
            std::string id;
            std::cin >> id;

            // Create message 
            std::string messageToSend;
            messageToSend = "SNG " + id + "\n";

            sendAndReceiveUdpMessage(readfds, tv, fd, n, addr, res, buffer, messageToSend);

            // Split the buffer information into different words 
            std::vector <std::string> parameters = stringSplit(std::string(buffer), ' ');

            // If there are at least 2 parameters and the first word is correct
            if (parameters.size() > 1 && strcmp("RSG", parameters[0].c_str()) == 0) { 
                // If game is good to go, create word, else print error message
                if (strcmp(parameters[1].c_str(), "OK") == 0) {
                    // Create empty  word
                    std::string wordSpaces = repeat("_ ", atoi(parameters[2].c_str()));
                    wordSpaces[wordSpaces.length()-1] = '\0';
                    // Save word
                    currentWord = wordSpaces;
                        
                    printf("New game started. Guess a %s letter word: %s. You can miss up to %s times.\n", parameters[2].c_str(), wordSpaces.c_str(), parameters[3].c_str());
                    gameActive = 1;
                    playerID = id;
                }
                else if (strcmp(parameters[1].c_str(), "NOK") == 0) {
                    printf("You can't start a new game. You have to wait for the current game to finish.\n");
                }
                else if (strcmp(parameters[1].c_str(), "ERR") == 0) {
                    printf("The player ID you provided is wrong. Please make sure your player ID is 6 digits long.\n");
                }
                // If the message recieved is for example: "RSG BOLA"
                else {
                    printf("Something went wrong. Please try again.\n");
                }
            }
            // If the message recieved is for example: "RLL"
            else {
                printf("Something went wrong. Please check if your player ID is correct and try again. \n");
            }
            close(fd);
        }
        else if(strcmp(word.c_str(),"play") == 0 or strcmp(word.c_str(),"pl") ==0) {
            fd=socket(AF_INET,SOCK_DGRAM,0); // create UDP socket
            if(fd==-1) exit(1);

            // Set socket to non-blocking
            fcntl(fd, F_SETFL, O_NONBLOCK);

            std::string letter;
            std::cin >> letter;

            if(gameActive == 0) {
                printf("You have to start a game before playing. You can do that by writing the command: 'start (yourID)'.\n");
                continue;
            }

            if (letter.length() != 1) {
                printf("Invalid letter. Please make sure that you only play one letter at a time.\n");
                continue;
            }
            
            // Create message to send
            std::string messageToSend;
            messageToSend = "PLG " + playerID + " " + letter + " " + std::to_string(trial) + "\n";

            sendAndReceiveUdpMessage(readfds, tv, fd, n, addr, res, buffer, messageToSend);
            
            // Split the buffer information into different words 
            std::vector <std::string> parameters = stringSplit(std::string(buffer), ' ');
 
            // If there are at least 2 parameters
            if (parameters.size() > 1 && strcmp("RLG", parameters[0].c_str()) == 0) {
                // If ok, replace the letters in the given positions 
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
                    gameActive = 0;
                }
                else if (strcmp(parameters[1].c_str(), "DUP") == 0) {
                    printf("You have already tried that letter. Try another letter please.\n");
                }
                else if (strcmp(parameters[1].c_str(), "NOK") == 0) {
                    printf("The letter %s is not part of the word: %s.\n", letter.c_str(), currentWord.c_str());
                    trial += 1;
                }
                else if (strcmp(parameters[1].c_str(), "OVR") == 0) {
                    printf("You have exceeded the maximum number of errors. You have lost the game.\n");
                    trial = 1; 
                    gameActive = 0;
                }
                else if (strcmp(parameters[1].c_str(), "INV") == 0) {
                    // If trials is invalid, set trials to the value sent by the server
                    printf("It seems the information you provided is different from the information present on the " \
                    "server. Please try again.\n");
                    trial = atoi(parameters[2].c_str());
                }
                else if (strcmp(parameters[1].c_str(), "ERR") == 0) {
                    printf("The data sent is not valid. Please check if your playerID is valid. If you don't " \
                    "have a game ongoing, please start one with the command: 'start (yourID)'.\n");
                }
                else {
                    printf("Something went wrong. Please try again.\n");
                }
            } else {
                printf("Something went wrong. Please try again.\n");
            }
            close(fd);
        }
        else if(strcmp(word.c_str(),"guess") == 0 or strcmp(word.c_str(),"gw") ==0) {
            fd=socket(AF_INET,SOCK_DGRAM,0); // create UDP socket
            if(fd==-1) exit(1);

            // Read word to be guessed 
            std::string guessedWord;
            std::cin >> guessedWord;

            if(gameActive == 0) {
                printf("You have to start a game before trying to guess the word.\n");
                continue;
            }

            if(guessedWord.length()<3 or guessedWord.length()>30) {
                printf("Invalid word. Please make sure that your word has between 3 and 30 characters.\n");
                continue;
            }
    
            //Create message
            std::string messageToSend;
            messageToSend = "PWG " + playerID + " " + guessedWord + " " + std::to_string(trial) + "\n";
            
            sendAndReceiveUdpMessage(readfds, tv, fd, n, addr, res, buffer, messageToSend);
            
            // Split the buffer information into different words 
            std::vector <std::string> parameters = stringSplit(std::string(buffer), ' ');

            // If there are at least 2 parameters
            if (parameters.size() > 1 && strcmp("RWG", parameters[0].c_str()) == 0) {
                if (strcmp(parameters[1].c_str(), "WIN") == 0) { 
                    printf("Great job! You've gessed the right word.\n");
                    trial = 1;
                    gameActive = 0;
                }
                else if (strcmp(parameters[1].c_str(), "NOK") == 0) {
                    printf("The word %s is not the right word.\n", guessedWord.c_str());
                    trial += 1;
                }
                else if (strcmp(parameters[1].c_str(), "OVR") == 0) {
                    printf("You have exceeded the maximum number of errors. You have lost the game.\n");
                    trial = 1; 
                    gameActive = 0;
                }
                else if (strcmp(parameters[1].c_str(), "INV") == 0) {
                    // If trials is invalid, set trials to the value sent by the server
                    printf("It seems the information you provided is different from the information present on the " \
                    "server. Please try again.\n");
                    trial = atoi(parameters[2].c_str());
                }
                else if (strcmp(parameters[1].c_str(), "ERR") == 0) {
                    printf("The data sent is not valid. Please check if your playerID is valid. If you don't " \
                    "have a game ongoing, please start one with the command: 'start (yourID)'.\n");
                }
                else {
                    printf("Something went wrong. Please try again.\n");
                }
            } else {
                printf("Something went wrong. Please try again.\n");
            }  
            close(fd);          
        }
        else if(strcmp(word.c_str(),"scoreboard") == 0 or strcmp(word.c_str(),"sb") ==0) {
            fd=socket(AF_INET,SOCK_STREAM,0); // create TCP socket
            if(fd==-1) /*error*/exit(1);

            // Create message  
            std::string messageToSend;
            messageToSend = "GSB\n";

            n = connect(fd, res->ai_addr, res->ai_addrlen);
            if (n == -1) /*error*/ exit(1);

            // Send TCP message
            sendTCP(fd, messageToSend);

            // Read TCP message
            readTcp(readfds, tv, fd, n, messageToSend, "scoreboard");

            close(fd);
        }
        else if(strcmp(word.c_str(),"hint") == 0 or strcmp(word.c_str(),"h") ==0) {
            fd=socket(AF_INET,SOCK_STREAM,0); // create TCP socket
            if(fd==-1) exit(1);
            
            if(gameActive == 0) {
                printf("You have to start a game first.\n");
                continue;
            }

            // Create message
            std::string messageToSend;
            messageToSend = "GHL " + playerID + "\n";

            n = connect(fd, res->ai_addr, res->ai_addrlen);
            if (n == -1) /*error*/ exit(1);

            // Send TCP message
            sendTCP(fd, messageToSend);
            
            // Read TCP message
            readTcp(readfds, tv, fd, n, messageToSend, "hint");   
            
            close(fd);
        }
        //FALTA VER CASOS DE EXIT
        else if(strcmp(word.c_str(),"state") == 0 or strcmp(word.c_str(),"st") ==0) {
            fd=socket(AF_INET,SOCK_STREAM,0); // create TCP socket
            if(fd==-1) /*error*/exit(1);
            
            if(playerID.length() == 0) {
                printf("You have to start a game first.\n");
                continue;
            }

            /* Create message */
            std::string messageToSend;
            messageToSend = "STA " + playerID + "\n";
            

            n = connect(fd, res->ai_addr, res->ai_addrlen);
            if (n == -1) /*error*/ exit(1);
            
            // Send TCP message
            sendTCP(fd, messageToSend);

            // Read TCP message
            readTcp(readfds, tv, fd, n, messageToSend, "state");

            close(fd);
        }
        else if(strcmp(word.c_str(),"quit") == 0) {
            fd=socket(AF_INET,SOCK_DGRAM,0); // create UDP socket
            if(fd==-1) /*error*/exit(1);
            
            if(gameActive == 0) {
                printf("There's no game undergoing. If you wish to start a game, type: 'sg (yourID)'\n");
                continue;
            }

            // Create message 
            std::string messageToSend;
            messageToSend = "QUT " + playerID + "\n";

            sendAndReceiveUdpMessage(readfds, tv, fd, n, addr, res, buffer, messageToSend);
            
            // Split the buffer information into different words 
            std::vector <std::string> parameters = stringSplit(std::string(buffer), ' ');

            // If there are at least 2 parameters and the first word is correct
            if (parameters.size() > 1 && strcmp("RQT", parameters[0].c_str()) == 0) { 
                if(strcmp(parameters[1].c_str(), "ERR") == 0){
                    printf("Something went wrong. Please try again.\n");            }
                else if (strcmp(parameters[1].c_str(), "OK") == 0) {
                    printf("You have successfully quit the game.\n");
                    gameActive = 0;
                    trial = 1;
                }
                else {
                    printf("Something went wrong. Please try again.\n");
                }
            } else {
                printf("Something went wrong. Please try again.\n");
            }
            close(fd);
        }
        else if(strcmp(word.c_str(),"exit") == 0) {
            fd=socket(AF_INET,SOCK_DGRAM,0); // create UDP socket
            if(fd==-1) exit(1);
    
            if(gameActive == 0) {
                printf("Closing aplication now...\n");
                sleep(0.7);
                std::string().swap(currentWord);
                std::string().swap(playerID);
                freeaddrinfo(res);
                close(fd);
                exit(0);
            }
            
            //Create message
            std::string messageToSend;
            messageToSend = "QUT " + playerID + "\n";

            sendAndReceiveUdpMessage(readfds, tv, fd, n, addr, res, buffer, messageToSend);
            
            // Split the buffer information into different words 
            std::vector <std::string> parameters = stringSplit(std::string(buffer), ' ');

            if (parameters.size() > 1 && strcmp("RQT", parameters[0].c_str()) == 0) {
                if(strcmp(parameters[1].c_str(), "ERR") == 0){
                    printf("Something went wrong. Please try again.\n");              }
                else if (strcmp(parameters[1].c_str(), "OK") == 0) {
                    printf("You have successfully quit the game. Closing apllication now...\n");
                    sleep(0.7);
                    gameActive = 0;
                    trial = 1;
                    std::string().swap(currentWord);
                    std::string().swap(playerID);
                    std::vector<std::string>().swap(parameters);
                    freeaddrinfo(res);
                    close(fd);
                    exit(0);
                }
                else {
                    printf("Something went wrong. Please try again.\n");
                }
            }
            else {
                printf("Something went wrong. Please try again.\n");
            }
            close(fd);
        }
        else {
            printf("Invalid command, please try again.\n");
            // Read useless information from line (if there are more than two arguments) and clear buffer
            fgets(buffer, sizeof(buffer), stdin);
            memset(buffer, 0 , sizeof(buffer));
            continue;
        }
    }
}


// Read input optional flags 
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

// Send TCP message
void sendTCP(int fd, std::string message) {
    int nw, i = 0;
    size_t n = message.length();
    while(n>0) {
        if ((nw=write(fd, message.substr(i, n).c_str(),n))<=0) {
            exit(1);
        }
        n -= nw;
        i += nw;
    }
}

// Recieve Udp message
ssize_t rcvMessageUdp(fd_set readfds, timeval tv, int fd, ssize_t n, struct sockaddr_in addr, struct addrinfo* res, char* buffer, std::string messageToSend) {
    while(1) {
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int rv = select(fd+1, &readfds, NULL, NULL, &tv);

        if (rv == 1) {
            // Receive status from GS to check if it is a hit, miss, e.t.c 
            n = recvfrom(fd, buffer, 256, 0, (struct sockaddr*)&addr, (socklen_t*)&res->ai_addrlen);
            if (n == -1) exit(1);
            break;
        } else {
            n = sendto(fd, messageToSend.c_str(), messageToSend.length(), 0, res->ai_addr, res->ai_addrlen);
            if (n == -1) exit(1);
        }
    }
    return n;
}

// Send and receive UDP message
void sendAndReceiveUdpMessage(fd_set readfds, timeval tv, int fd, ssize_t n, struct sockaddr_in addr, struct addrinfo* res, char* buffer, std::string messageToSend) {
    n = sendto(fd, messageToSend.c_str(), messageToSend.length(), 0, res->ai_addr, res->ai_addrlen);
    if (n == -1) exit(1);

    // Empty buffer 
    memset(buffer, 0 , 256);

    n = rcvMessageUdp(readfds, tv, fd, n, addr, res, buffer, messageToSend);

    buffer[n-1] = '\0';
}

int readStatusMessageHint(std::string status, int wordsRead) {
    if(strcmp(status.c_str(),"NOK") == 0) {
        printf("Something went wrong. Please try again.\n");
        return wordsRead;
    }
    else if (strcmp(status.c_str(), "OK") == 0) {
        printf("The image file is being loaded...\n");
        wordsRead+=1;
        return wordsRead;
    }
    else {
        printf("Something went wrong. Please try again.\n");
        return wordsRead;
    }
    return wordsRead;
}

int readStatusMessageState(std::string status, int wordsRead) {
    if(strcmp(status.c_str(),"NOK") == 0) {
        printf("You haven't completed a game yet nor do you have an active game. To play start a game type: 'sg (yourID)'\n");
        return wordsRead;
    }
    else if (strcmp(status.c_str(),"FIN") == 0 or strcmp(status.c_str(),"ACT") == 0) {
        wordsRead+=1;
        return wordsRead;
    }
    else {
        printf("Something went wrong. Please try again.\n");
        return wordsRead;
    }
    return wordsRead;
}

int readStatusMessageScoreboard(std::string status, int wordsRead) {
    if(strcmp(status.c_str(),"EMPTY") == 0) {
        printf("The scoreboard is still empty.\n");
        return wordsRead;
    }
    else if (strcmp(status.c_str(), "OK") == 0) {
        wordsRead+=1;
        return wordsRead;
    }
    else {
        printf("Something went wrong. Please try again.\n");
        return wordsRead;
    }
    return wordsRead;
}
void readMessageTcp(int fd, ssize_t n, std::string type) {
    FILE* file;
    int fSize = 0;
    int iterationSize = 1;
    int wordsRead = 0;
    int lastRead = 0;
    std::string status;
    std::string filename;
    std::string sizeOfFile;
    char buffer[256];
    // Empty buffer 
    memset(buffer, 0 , 256);
    
    while((n = read(fd, buffer, iterationSize)) != 0) {
        if (n == -1)  exit(1);
        //Da primeira vez lê apenas "RSB "
        if (wordsRead == 0) { 
            if (strcmp(buffer, " ") == 0) {
            wordsRead+=1;
            }
        }
        //Ler palavra status
        else if (wordsRead == 1) {
            if (strcmp(buffer, " ") != 0 && strcmp(buffer, "\n") != 0){
                status += buffer;
            }
            else {
                if (type == "scoreboard") {
                    wordsRead = readStatusMessageScoreboard(status, wordsRead);
                }
                else if (type == "hint") { 
                    wordsRead = readStatusMessageHint(status, wordsRead);
                }
                else if (type == "state") {
                    wordsRead = readStatusMessageState(status, wordsRead);
                }
                else if (wordsRead == 1) {
                    printf("Something went wrong. Please try again.\n");  
                    break;
                }
            }
        }
        // Ler nome do ficheiro
        else if (wordsRead == 2) {
            if (strcmp(buffer, " ") != 0) {
                filename += buffer;
                continue;
            } else {
                printf("The file will be saved locally with the name: %s.\n", filename.c_str());
                wordsRead+=1;
            }
        }
        // Ler tamanho do ficheiro
        else if (wordsRead == 3) {
            if (strcmp(buffer, " ") != 0) {
                sizeOfFile += buffer;
                continue;
            } else {
                wordsRead+=1;
                fSize = atoi(sizeOfFile.c_str());
                iterationSize = 128;
                file = fopen(filename.c_str(), "w");
            }   
        }
        // Ler o ficheiro
        else if (wordsRead == 4) {
            //Última leitura ou se o tamanho for menor que a primeira iteração
            if (lastRead == 1 or fSize < iterationSize) {
                fwrite(buffer, sizeof(char), n, file);
                if (type != "hint") {
                    printf("%s", buffer); 
                }
                fclose(file);  
                break;
            } else {
                fSize -= n;
                if (fSize <= iterationSize) {
                    iterationSize = fSize;
                    lastRead = 1;
                }        
                fwrite(buffer, sizeof(char), n, file);
                if (type != "hint") {
                    printf("%s", buffer); 
                }
            }
        }
        memset(buffer,0,256);
    }
}

void readTcp(fd_set readfds, timeval tv,int fd, ssize_t n, std::string messageToSend, std::string type) {
    while(1) {
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int rv = select(fd+1, &readfds, NULL, NULL, &tv);

        if (rv == 1) {
            readMessageTcp(fd, n, type);
            break;
        } else {
            sendTCP(fd, messageToSend);
        }
    }
}
