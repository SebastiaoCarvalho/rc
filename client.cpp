//FPRINTF NÃO ESCREVE '\0'

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "utils.h"


//  LIMITAR NUMERO DE PORTS
// Se criação de socket falhar, repetir automaticamente

// Global variables
int fd;                // Socket file descriptor
struct addrinfo hints,*res; // Socket address info

int main(int argc, char const *argv[]) {
    
    // Functions declaration
    void readFlags(int argc, char const *argv[], std::string *machineIP, std::string *port);
    void handleCtrlC(int exitValue);
    ssize_t rcvMessageUdp(fd_set readfds, timeval tv, int fd, ssize_t n, struct sockaddr_in addr, struct addrinfo* res, char* buffer, std::string messageToSend);
    void sendAndReceiveUdpMessage(fd_set readfds, timeval tv, int fd, ssize_t n, struct sockaddr_in addr, struct addrinfo* res, char* buffer, std::string messageToSend);
    int readStatusMessageHint(std::string status, int wordsRead);
    int readStatusMessageState(std::string status, int wordsRead);
    int readStatusMessageScoreboard(std::string status, int wordsRead);
    void readMessageTcp(int fd, ssize_t n, std::string type);
    void sendTCP(int fd, std::string message);
    void readTcp(fd_set readfds, timeval tv, int fd, ssize_t n, std::string messageToSend, std::string type);
    
    // Variables declaration
    std::string machineIP = "127.0.0.1"; // Deafult IP
    std::string port = "58002";     //Default port

    int fd,errcode;
    struct sigaction act;
    struct addrinfo hints,*res;
    struct sockaddr_in addr;
    char buffer[256];         
    std::string playerID;
    std::string currentWord;
    int gameActive = 0;
    int trial = 1; 
    int maxErrors = 0;
    ssize_t n;
    fd_set readfds;  
    struct timeval tv;
    

    readFlags(argc, argv, &machineIP, &port);
    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET; //IPv4
    hints.ai_socktype=SOCK_DGRAM; //UDP socket
    errcode=getaddrinfo(machineIP.c_str(),port.c_str(),&hints,&res);
    if(errcode!=0) exit(1);
    signal(SIGINT, handleCtrlC);
    memset(&act,0,sizeof act);
    act.sa_handler=SIG_IGN;
    if(sigaction(SIGPIPE,&act,NULL)==-1) exit(1);

    while(1) {
    
        // Read command
        std::string word; 
        std::cin >> word;
        
        if(word == "start" or word == "sg") {
            fd=socket(AF_INET,SOCK_DGRAM,0); // Create UDP socket
            if(fd==-1) {
                printf("Error creating socket. Please try again.\n");
                continue;
            }

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
            if (parameters.size() > 1 && parameters[0] == "RSG") { 
                // If game is good to go, create word, else print error message
                if (parameters[1] == "OK") {
                    // Create empty  word
                    std::string wordSpaces = repeat("_ ", atoi(parameters[2].c_str()));
                    wordSpaces[wordSpaces.length()-1] = '\0';
                    // Save word
                    currentWord = wordSpaces;
                        
                    printf("New game started. Guess a %s letter word: %s. You can miss up to %s times.\n", parameters[2].c_str(), wordSpaces.c_str(), parameters[3].c_str());
                    gameActive = 1;
                    playerID = id;
                    maxErrors = atoi(parameters[3].c_str());
                }
                else if (parameters[1] == "NOK") {
                    printf("You can't start a new game. You have to wait for the current game to finish.\n");
                    gameActive = 0;
                }
                else if (parameters[1] == "ERR") {
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
            n = close(fd);
            if (n == -1) {
                continue;
            }
        }
        else if(word == "play" or word == "pl") {
            fd=socket(AF_INET,SOCK_DGRAM,0); // create UDP socket
            if(fd==-1) {
                printf("Error creating socket. Please try again.\n");
                continue;
            }

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
 
            // If there are at least 2 parameters and the first word is correct
            if (parameters.size() > 1 && parameters[0] == "RLG") {
                // If ok, replace the letters in the given positions 
                if (parameters[1] == "OK") {
                    for (int i=0; i<atoi(parameters[3].c_str()); i++) {
                        int pos;
                        pos = atoi(parameters[4+i].c_str());
                        currentWord[(pos-1)*2] = letter[0];
                    }
                    trial += 1;
                    printf("Word: %s\n", currentWord.c_str());
                }
                else if (parameters[1] == "WIN") {
                    for (int i = 0; i<(int)currentWord.length(); i++) {
                        if (currentWord[i] == '_') {
                            currentWord[i] = letter[0];
                        }
                    }
                    printf("Well done! You've gessed the right word: %s.\n", currentWord.c_str());
                    trial = 1;
                    gameActive = 0;
                }
                else if (parameters[1] == "DUP") {
                    printf("You have already tried that letter. Try another letter please.\n");
                }
                else if (parameters[1] == "NOK") {
                    maxErrors--;
                    printf("The letter %s is not part of the word: %s. You have %d errors left.\n", letter.c_str(), currentWord.c_str(), maxErrors);
                    trial += 1;
                }
                else if (parameters[1] == "OVR") {
                    printf("You have exceeded the maximum number of errors. You have lost the game.\n");
                    trial = 1; 
                    gameActive = 0;
                }
                else if (parameters[1] == "INV") {
                    // If trials is invalid, set trials to the value sent by the server
                    printf("It seems the information you provided is different from the information present on the " \
                    "server. Please try again.\n");
                    trial = atoi(parameters[2].c_str());
                }
                else if (parameters[1] == "ERR") {
                    printf("The data sent is not valid. Please check if your playerID is valid. If you don't " \
                    "have a game ongoing, please start one with the command: 'start (yourID)'.\n");
                }
                else {
                    printf("Something went wrong. Please try again.\n");
                }
            } else {
                printf("Something went wrong. Please try again.\n");
            }
            n = close(fd);
            if (n == -1) {
                continue;
            }
        }
        else if(word == "guess" or word == "gw") {
            fd=socket(AF_INET,SOCK_DGRAM,0); // create UDP socket
            if(fd==-1) {
                printf("Error creating socket. Please try again.\n");
                continue;
            }

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

            // If there are at least 2 parameters and the first word is correct
            if (parameters.size() > 1 && parameters[0] == "RWG") {
                if (strcmp(parameters[1].c_str(), "WIN") == 0) { 
                    printf("Great job! You've gessed the right word.\n");
                    trial = 1;
                    gameActive = 0;
                }
                else if (parameters[1] == "DUP") {
                    printf("You have already tried to guess that word. Try another word please.\n");
                }
                else if (parameters[1] == "NOK") {
                    maxErrors--;
                    printf("The word %s is not the right word. You have %d errors left.\n", guessedWord.c_str(), maxErrors);
                    trial += 1;
                }
                else if (parameters[1] == "OVR") {
                    printf("You have exceeded the maximum number of errors. You have lost the game.\n");
                    trial = 1; 
                    gameActive = 0;
                }
                else if (parameters[1] == "INV") {
                    // If trials is invalid, set trials to the value sent by the server
                    printf("It seems the information you provided is different from the information present on the " \
                    "server. Please try again.\n");
                    trial = atoi(parameters[2].c_str());
                }
                else if (parameters[1] == "ERR") {
                    printf("The data sent is not valid. Please check if your playerID is valid. If you don't " \
                    "have a game ongoing, please start one with the command: 'start (yourID)'.\n");
                }
                else {
                    printf("Something went wrong. Please try again.\n");
                }
            } else {
                printf("Something went wrong. Please try again.\n");
            }  
            n = close(fd);
            if (n == -1) {
                continue;
            }          
        }
        else if(word == "scoreboard" or word == "sb") {
            fd=socket(AF_INET,SOCK_STREAM,0); // create TCP socket
            if(fd==-1) {
                printf("Error creating socket. Please try again.\n");
                continue;
            }

            // Create message  
            std::string messageToSend;
            messageToSend = "GSB\n";

            n = connect(fd, res->ai_addr, res->ai_addrlen);
            if (n == -1) {
                printf("Error connecting to server. Please try again.\n");
                close(fd);
                continue;
            }

            // Send TCP message
            sendTCP(fd, messageToSend);

            // Read TCP message
            readTcp(readfds, tv, fd, n, messageToSend, "scoreboard");

            n = close(fd);
            if (n == -1) {
                continue;
            }
        }
        else if(word == "hint" or word == "h") {
            fd=socket(AF_INET,SOCK_STREAM,0); // create TCP socket
            if(fd==-1) {
                printf("Error creating socket. Please try again.\n");
                continue;
            }
            
            if(gameActive == 0) {
                printf("You have to start a game first.\n");
                continue;
            }

            // Create message
            std::string messageToSend;
            messageToSend = "GHL " + playerID + "\n";

            n = connect(fd, res->ai_addr, res->ai_addrlen);
            if (n == -1) {
                printf("Error connecting to server. Please try again.\n");
                close(fd);
                continue;
            }

            // Send TCP message
            sendTCP(fd, messageToSend);
            
            // Read TCP message
            readTcp(readfds, tv, fd, n, messageToSend, "hint");   
            
            n = close(fd);
            if (n == -1) {
                continue;
            }
        }
        else if(word == "state" or word == "st") {
            fd=socket(AF_INET,SOCK_STREAM,0); // create TCP socket
            if(fd==-1) {
                printf("Error creating socket. Please try again.\n");
                continue;
            }

            /* Create message */
            std::string messageToSend;
            messageToSend = "STA " + playerID + "\n";
            

            n = connect(fd, res->ai_addr, res->ai_addrlen);
            if (n == -1) {
                printf("Error connecting to server. Please try again.\n");
                close(fd);
                continue;
            }
            
            // Send TCP message
            sendTCP(fd, messageToSend);

            // Read TCP message
            readTcp(readfds, tv, fd, n, messageToSend, "state");

            n = close(fd);
            if (n == -1) {
                continue;
            }
        }
        else if(word == "quit") {
            fd=socket(AF_INET,SOCK_DGRAM,0); // Create UDP socket
            if(fd==-1) {
                printf("Error creating socket. Please try again.\n");
                continue;
            }
            
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

                        printf("%s", parameters[1].c_str());
            // If there are at least 2 parameters and the first word is correct
            if (parameters.size() > 1 && parameters[0] == "RQT") { 
                if(parameters[1] == "ERR"){
                    printf("Something went wrong. Please try again.\n");            }
                else if (parameters[1] == "OK") {
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
            n = close(fd);
            if (n == -1) {
                continue;
            }
        }
        else if(word == "exit") {
            fd=socket(AF_INET,SOCK_DGRAM,0); // Create UDP socket
            if(fd==-1) {
                printf("Error creating socket. Please try again.\n");
                continue;
            }
    
            // If there's no game ongoing, just exit the program
            if(gameActive == 0) {
                printf("Closing aplication now...\n");
                sleep(1);
                std::string().swap(machineIP);
                std::string().swap(port);
                std::string().swap(currentWord);
                std::string().swap(playerID);
                freeaddrinfo(res);
                n = close(fd);
                if (n == -1) {
                    continue;
                }
                exit(0);
            }
            
            //Create message
            std::string messageToSend;
            messageToSend = "QUT " + playerID + "\n";

            sendAndReceiveUdpMessage(readfds, tv, fd, n, addr, res, buffer, messageToSend);
            
            // Split the buffer information into different words 
            std::vector <std::string> parameters = stringSplit(std::string(buffer), ' ');

            if (parameters.size() > 1 && parameters[0] == "RQT") {
                if(parameters[1] == "ERR"){
                    printf("Something went wrong. Please try again.\n");              }
                else if (parameters[1] == "OK") {
                    printf("You have successfully quit the game. Closing apllication now...\n");
                    sleep(1);
                    gameActive = 0;
                    trial = 1;
                    std::string().swap(machineIP);
                    std::string().swap(port);
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
            n = close(fd);
            if (n == -1) {
                continue;
            }
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
void readFlags(int argc, char const *argv[], std::string * machineIP, std::string * port) {
    int argn = 1;
    while (argn < argc) {
        if (strcmp(argv[argn],"-n") == 0) {
            if (argn + 1 < argc) {
                *machineIP = argv[argn + 1];
                argn += 2;
            } 
        }
        else if (strcmp(argv[argn],"-p") == 0) {
            if (argn + 1 < argc) {
                *port = argv[argn + 1];
                argn += 2;
            } 
        } else {
            argn += 1;
        }
    }
}

/* Handle Ctrl C Event (SIGINT) */
void handleCtrlC(int exitValue) {
    printf("Caught exitting signal. Exiting...\n");
    freeaddrinfo(res);
    close(fd);
    exit(exitValue);
}

// Recieve Udp message
ssize_t rcvMessageUdp(fd_set readfds, timeval tv, int fd, ssize_t n, struct sockaddr_in addr, struct addrinfo* res, char* buffer, std::string messageToSend) {
    while(1) {
        // Define arguments to set timer
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);
        // Tiemout time
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        // Max number of tries to send message
        int numberOfTries = 0;

        int rv = select(fd+1, &readfds, NULL, NULL, &tv);

        if (rv == 1) {
            // Receive status from GS to check if it is a hit, miss, e.t.c 
            n = recvfrom(fd, buffer, 256, 0, (struct sockaddr*)&addr, (socklen_t*)&res->ai_addrlen);
            if (n == -1) exit(1);
            break;
        } else if (numberOfTries == 5 && rv != 1) {
            printf("The server is not responding. Please try again later.\n");
            break;
        } else {
            // If there's no response from GS, send the message again
            n = sendto(fd, messageToSend.c_str(), messageToSend.length(), 0, res->ai_addr, res->ai_addrlen);
            if (n == -1) exit(1);
            numberOfTries++;
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
    if(status == "NOK") {
        printf("The server was not able to provide you the image. Please try again.\n");
        return wordsRead;
    }
    else if (status == "OK") {
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
    if(status == "NOK") {
        printf("You haven't completed a game yet nor do you have an active game. To play start a game type: 'sg (yourID)'\n");
        return wordsRead;
    }
    else if (status == "FIN" or status == "ACT") {
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
    if(status == "EMPTY") {
        printf("The scoreboard is still empty.\n");
        return wordsRead;
    }
    else if (status == "OK") {
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
    std::string firstWord;
    std::string status;
    std::string filename;
    std::string sizeOfFile;
    char buffer[256];
    // Empty buffer 
    memset(buffer, 0 , 256);
    
    while((n = read(fd, buffer, iterationSize)) != 0) {
        if (n == -1)  exit(1);
        // Reads the first word
        if (wordsRead == 0) { 
            if (strcmp(buffer, " ") != 0) {
                firstWord += buffer;
            } else {
                // Checks if the message received is correct
                if (firstWord == "RSB" or firstWord == "RHL" or firstWord == "RST") {
                    wordsRead+=1;
                } else {
                    printf("Something went wrong. Please try again.\n");  
                    break;
                }
            }
        }
        // Reads the status word
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
        // Reads the filename
        else if (wordsRead == 2) {
            if (strcmp(buffer, " ") != 0) {
                filename += buffer;
                continue;
            } else {
                printf("The file was saved locally with the name: %s.\n", filename.c_str());
                wordsRead+=1;
            }
        }
        // Reads the file size
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
        // Reads the file
        else if (wordsRead == 4) {
            // Last time reading
            if (lastRead == 1 or fSize < iterationSize) {
                // buffer[n-1] = '\0';
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

// Send TCP message
void sendTCP(int fd, std::string message) {
    int nw, i = 0;
    size_t n = message.length();
    // Write the message until n if its size is bigger than n
    while(n>0) {
        if ((nw=write(fd, message.substr(i, n).c_str(),n))<=0) {
            exit(1);
        }
        n -= nw;
        i += nw;
    }
}

void readTcp(fd_set readfds, timeval tv,int fd, ssize_t n, std::string messageToSend, std::string type) {
    while(1) {
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        int numberOfTries = 0;

        int rv = select(fd+1, &readfds, NULL, NULL, &tv);

        if (rv == 1) {
            readMessageTcp(fd, n, type);
            break;
        } else if (numberOfTries == 5 && rv != 1) {
            printf("The server is not responding. Please try again later.\n");
            break;
        } else {
            sendTCP(fd, messageToSend);
            numberOfTries++;
        } 
    }
}
