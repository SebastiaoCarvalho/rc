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
#include "filehandling.h"
#include <string>
#include <signal.h>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <sstream>

// TODO : 
// add verification of n != -1 on sends at udp protocols
// check error cases for makeplay and makeguess
// change quit to save game before quitting
// check if if else order on plays and guesses is right
// create message makers 
// review global vars like fileName and wordG
// implement verbose
// Socket response
// handle signals
// handle errors
// handle exits freeing stuff
// wordG remove
// write write to tcp function

struct sigaction act;
int fd, newfd, errcode, seed;
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
    void bootServer();
    void sendScoreBoard(int newfd);
    void sendHint(int newfd, std::string playerID);
    void sendState(int newfd, std::string playerID);
    void quitGame(std::string playerID);

    ssize_t n;
    readFlags(argc, argv);
    bootServer();
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
        // set socket timer
        /* struct timeval tv;
        tv.tv_sec = 10;
        tv.tv_usec = 0;
        if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
            perror("Error");
        } */
        memset(&hints,0,sizeof hints);
        hints.ai_family=AF_INET; // IPv4
        hints.ai_socktype=SOCK_DGRAM; // UDP socket
        hints.ai_flags=AI_PASSIVE;
        errcode=getaddrinfo(NULL, port.c_str(),&hints,&res);
        if(errcode!=0) /*error*/ exit(1);
        n=bind(fd,res->ai_addr, res->ai_addrlen);
        printf("%d\n", errno);
        if(n==-1) /*error*/ exit(1);
        while (1) {
            addrlen=sizeof(addr);
            printf("Waiting for message...\n");
            memset(buffer,0,128);
            n=recvfrom(fd,buffer, 128, 0, (struct sockaddr*)&addr,&addrlen);
            if(n==-1)/*error*/exit(1);
            buffer[128] = '\0';
            std::string message = buffer;
            message = strip(message);
            std::vector<std::string> tokens = stringSplit(message, ' ');
            if (strncmp(buffer, "SNG", 3) == 0) {
                /* Read PlayerID*/
                std::string playerID = tokens[1];
                printf("PlayerID: %s\n", playerID.c_str());
                if (verbose) {
                    printf("Received SNG with playerID: %s from IP address: %s and port: %d\n", playerID.c_str(), inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
                }
                startGame(playerID);
            }
            else if (strncmp(buffer, "PLG", 3) == 0) {
                /* Read PlayerID*/
                std::string playerID = tokens[1];
                printf("%s\n", playerID.c_str());
                /*Read letter*/
                char letter = tokens[2][0];
                printf("%c\n", letter);
                /*Read Trial*/
                std::string trial = tokens[3];
                if (verbose) {
                    printf("Received PLG with playerID: %s, letter: %c, trial: %s from IP address: %s and port: %d\n", playerID.c_str(), letter, trial.c_str(), inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
                }
                printf("%s\n", trial.c_str());
                int trial_int = atoi(trial.c_str());
                printf("trial_int: %d\n", trial_int);
                makePlay(playerID, letter, trial_int);

            }
            else if (strncmp(buffer, "PWG", 3) == 0) {
                /* Read PlayerID*/
                std::string playerID = tokens[1];
                printf("%s\n", playerID.c_str());
                std::string word = tokens[2];
                printf("%s\n", word.c_str());
                /*Read Trial*/
                std::string trial = tokens[3];
                if (verbose) {
                    printf("Received PWG with playerID: %s, word: %s, trial: %s from IP address: %s and port: %d\n", playerID.c_str(), word.c_str(), trial.c_str(), inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
                }
                printf("%s\n", trial.c_str());
                int trial_int = atoi(trial.c_str());
                makeGuess(playerID, word, trial_int);

            }
            else if (strncmp(buffer, "QUT", 3) == 0) {
                /* Read PlayerID*/
                std::string playerID = tokens[1];
                printf("%s\n", playerID.c_str());
                message = "OK " + playerID + "\n";
                if (verbose) {
                    printf("Received QUT with playerID: %s from IP address: %s and port: %d\n", playerID.c_str(), inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
                }
                quitGame(playerID);

            }      
            else if (strncmp(buffer, "REV", 3) == 0) {
                
            }
            else if (strncmp(buffer, "RRV", 3) == 0) {
                
            }
            else {
                n=sendto(fd, "ERR\n", 128 ,0 , (struct sockaddr*)&addr, addrlen);
                if (n==-1)/*error*/ exit(1);
            }
        }
        freeaddrinfo(res);
        close(fd);
        return 0;
    }
    else {

        char buffer[129];
        int ret;
        act.sa_handler=SIG_IGN;
        if(sigaction(SIGCHLD,&act,NULL)==-1)/*error*/exit(1);
        if((fd=socket(AF_INET,SOCK_STREAM,0))==-1)exit(1);//error
        memset(&hints,0,sizeof hints);
        hints.ai_family=AF_INET;//IPv4
        hints.ai_socktype=SOCK_STREAM;//TCP socket
        hints.ai_flags=AI_PASSIVE;
        if ((ret=getaddrinfo(NULL, port.c_str(), &hints, &res))!=0)/*error*/exit(1);
        if (bind(fd,res->ai_addr,res->ai_addrlen)==-1)/*error*/exit(1);
        if (listen(fd,5)==-1)/*error*/exit(1);
        freeaddrinfo(res);//frees the memory allocated by getaddrinfo (no longer needed)

        while(1) {
            printf("Waiting for connection...\n");
            memset(buffer,0,128);
            addrlen=sizeof(addr);
            do newfd=accept(fd,(struct sockaddr*)&addr,&addrlen);//wait for a connection
            while(newfd==-1&&errno==EINTR);
            if(newfd==-1)/*error*/exit(1);
            if((pid=fork())==-1)/*error*/exit(1);
            else if(pid==0) { // child process
                close(fd);
                while((n=read(newfd,buffer,128))!=0) {
                    if(n==-1)/*error*/exit(1);
                }
                buffer[128] = '\0';
                std::string message = buffer;
                message = strip(message);
                std::vector<std::string> tokens = stringSplit(message, ' ');
                if (strncmp(buffer, "GSB", 3) == 0) {
                    sendScoreBoard(newfd);
                }
                else if (strncmp(buffer, "GHL", 3) == 0) {
                    std::string playerID = tokens[1];
                    printf("%s\n", playerID.c_str());
                    sendHint(newfd, playerID);
                }
                else if (strncmp(buffer, "STA", 3) == 0) {
                    std::string playerID = tokens[1];
                    printf("%s\n", playerID.c_str());
                    sendState(newfd, playerID);
                }
                else {
                    message = "ERR\n";
                    int nw = 0;
                    int i = 0;
                    while(n>0) {
                        if ((nw=write(newfd, message.substr(i, n).c_str(),n))<=0) exit(1);
                        printf("%s", message.substr(i, nw).c_str());
                        n -= nw;
                        i += nw;
                    }
                }
            close(newfd); exit(0);
            }
            // parent process
            do ret=close(newfd);while(ret==-1&&errno==EINTR);
            if(ret==-1)/*error*/exit(1);
        }
    }
}


/* void handleCtrlC(int s){
    freeaddrinfo(res);
    close(fd);
} */

void readFlags(int argc, char const *argv[]) {
    if (argc == 1) {
        printf("No file name specified. Using default file name: words.txt\n");
        fileName = "words.txt";
    }
    else {
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

void getNewSeed() {
    if ( ! verifyExistence("seed.txt") ) {
        std::ofstream file("seed.txt");
        file << 0;
        file.close();
        seed = 0;
        return;
    }
    std::ifstream file("seed.txt");
    std::string line;
    std::getline(file, line);
    seed = std::stoi(line);
    seed = random(seed, 0, 1000);
    file.close();
    std::ofstream file2("seed.txt");
    file2 << seed;
    file2.close();
}

void readWord(std::string fileName, std::string * word) {
    std::ifstream file(fileName);
    std::string line;
    std::vector<std::string> lines;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    getNewSeed();
    int lineNumber = random(seed, 0, lines.size() - 1);
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

void bootServer() {
    std::vector<std::string> files = listDirectory("GAMES");
    size_t size = files.size();
    for (size_t i = 0; i < size; i++) {
        int res = deleteFile("GAMES/" + files[i]);
        if (res == 0) {
            std::cout << "Error deleting file " << files[i] << std::endl;
        }
    }
}

int hasGame(std::string playerId) {
    return verifyExistence("GAMES/GAME_" + playerId) && getLineNumber("GAMES/GAME_" + playerId) > 1;
}
void startGame(std::string playerID) {
    std::string word;
    std::string status;
    std::string message;
    if (hasGame(playerID)) {
        status = "NOK";
        message = "RSG " + status + "\n";
    }
    else {
        status = "OK";
        if (! verifyExistence("GAMES/GAME_" + playerID)) { // get new word from word file
            readWord(fileName, &word); 
            createGameFile(playerID, word);
        }
        else { // get word from game file    
            word = stringSplit(getLine("GAMES/GAME_" + playerID, 1), ' ')[0]; 
        }
        wordG = word;
        std::string letterNumber = std::to_string(word.length());
        int errorsN = maxErrors(wordG);
        std::string maxErrors = std::to_string(errorsN);
        message = "RSG " + status + " " + letterNumber + " " + maxErrors + "\n";
    }
    sendto(fd, message.c_str(), message.length(), 0, (struct sockaddr*)&addr, addrlen); // TODO : check if sends using n = 
}

int isRepeated(std::string playerID, std::string play) {
    std::string line = getLastLine("GAMES/GAME_" + playerID);
    printf("line: %s\n", line.c_str());
    printf("play: %s\n", play.c_str());
    return play == line + "\n";
}

void savePlay(std::string playerID, std::string status, std::string hit, std::string play, int missing) {
    std::string filename = "GAMES/GAME_" + playerID;
    appendFile(filename, status + " " + hit + " " + play + " " + std::to_string(missing) + "\n");
}

int getErrorsMade(std::string playerID) {
    std::ifstream file("GAMES/GAME_" + playerID);
    std::string line;
    std::getline(file, line); // skip first line
    int errors = 0;
    while (std::getline(file, line)) {
        std::vector<std::string> words = stringSplit(line, ' ');
        if (words[1] == "M") {
            errors++;
        }
    }
    return errors;
}

int getMissingNumber(std::string playerId) {
    if (! hasGame(playerId)) {
        return -1;
    }
    std::string line = getLine("GAMES/GAME_" + playerId, getLineNumber("GAMES/GAME_" + playerId));
    std::string missing = stringSplit(line, ' ')[3];
    return atoi(missing.c_str());
}

void storeGame(std::string playerID, std::string status) {
    time_t now = time(0);
    tm *ltm = localtime(&now);
    std::string timeStamp = getDateFormatted(ltm);
    std::string filename = timeStamp + "_" + status;
    int n = moveFile("GAMES/GAME_" + playerID, "GAMES/" + playerID, filename);
    if (n != 0) {
        std::cout << "Error renaming file" << std::endl;
        std::cout << "Error code: " << errno << std::endl;
    }
}

void saveScore(std::string playerID) {
    std::string filename = "GAMES/GAME_" + playerID;
    std::string word = stringSplit(getLine(filename, 1), ' ')[0];
    int errors = maxErrors(word);
    int trials = getLineNumber(filename) - 1;
    int errorsMade = getErrorsMade(playerID);
    int succ = trials - errorsMade;
    //int score = (errors - errorsMade) * 100 / errors;
    int score = (int) ((errors/trials) * 11);
    printf("Score: %d\n", score);
    std::string scoreS = std::to_string(score);
    while (scoreS.length() < 3) {
        scoreS = "0" + scoreS;
    }
    std::ofstream file;
    time_t now = time(0);
    tm *ltm = localtime(&now);
    std:: string scoreFilename = scoreS+ "_" + playerID + "_" + getDateFormatted(ltm);
    file.open("SCORES/" + scoreFilename);
    file << scoreS << " " << playerID << " " << word << " " << succ << " " << trials << std::endl;
}

void quitGame(std::string playerID) {
    std::string status;
    if (hasGame(playerID)) {
        status = "OK";
        storeGame(playerID, "Q");
    }
    else {
        status = "ERR";
    }
    std::string message = "RQT " + status + "\n";
    sendto(fd, message.c_str(), message.length(), 0, (struct sockaddr*)&addr, addrlen);
}

int isTrialValid(std::string playerID, int trial) {
    int line_number = getLineNumber("GAMES/GAME_" + playerID);
    printf("line_number: %d\n", line_number);
    return trial == line_number;
}


int isDup(std::string playerID, std::string play) {
    std::ifstream file("GAMES/GAME_" + playerID);
    std::string line;
    std::getline(file, line); // skip first line
    while (std::getline(file, line)) {
        std::vector<std::string> words = stringSplit(line, ' ');
        if (words[2] == play) {
            return 1;
        }
    }
    return 0;
}

std::string getScoreBoard() {
    std::string scoreboard = "";
    std::vector<std::string> files = listDirectory("SCORES");
    ssize_t size = files.size();
    for (ssize_t i = size - 1; i >= 0 && i >= size - 10; i--) {
        std::string line = getLine("SCORES/" + files[i], 1);
        std::vector<std::string> words = stringSplit(line, ' ');
        scoreboard += words[1] + " " + std::to_string(words[2].length()) + " " + words[4] + "\n";
    }
    return scoreboard;
}

void sendScoreBoard(int newfd) {
    std::string scoreboard = getScoreBoard();
    std::string message;
    if (scoreboard == "") {
        message = "RSB EMPTY\n";
    }
    else {
        message = "RSB OK scoreboard.txt " + std::to_string(scoreboard.size()) + " " +  scoreboard + "\n";
    }
    size_t n = message.length();
    int nw = 0;
    int i = 0;
    while(n>0) {
        if ((nw=write(newfd, message.substr(i, n).c_str(),n))<=0) exit(1);
        printf("%s", message.substr(i, nw).c_str());
        n -= nw;
        i += nw;
    }
}

std::string readImage(std::string filename) {
    // std::ifstream file(filename, std::ios::binary);
    // std::ostringstream ss;
    std::string image = "";
    if (!verifyExistence(filename)) {
        return image;
    }
    // ss << file.rdbuf();
    // image = ss.str();
    std::ifstream file(filename, std::ios::binary);
    std::string line;
    while (std::getline(file, line)) {
        image += line + "\n";
    }
    return image;
}

void sendHint(int newfd, std::string playerID) {
    std::string message;
    if (! verifyExistence("GAMES/GAME_" + playerID)) {
        message = "RHL NOK\n";
    }
    else {
        std::string word = stringSplit(getLine("GAMES/GAME_" + playerID, 1), ' ')[0];
        std::string image = readImage("images/" + word + ".jpg");
        if (image == "") {
            message = "RHL NOK\n";
        }
        else {
            message = "RHL OK " + word + ".jpg " + std::to_string(image.size()) + " " + image + "\n";
        }
        size_t n = message.length();
        int nw = 0;
        int i = 0;
        while(n>0) {
            if ((nw=write(newfd, message.substr(i, n).c_str(),n))<=0) exit(1);
            printf("%s", message.substr(i, nw).c_str());
            n -= nw;
            i += nw;
        }
    }
    
}

std::string getLastGame(std::string playerID) {
    std::vector<std::string> files = listDirectory("GAMES/" + playerID);
    ssize_t size = files.size();
    std::string filename = "GAMES/" + playerID + "/" + files[size - 1];
    std::string file_content = getContent(filename);
    std::string res = files[size - 1] + " " + std::to_string(file_content.size()) + " " + file_content;
    return res;
}

void sendState(int newfd, std::string playerID) {
    std::string message;
    std::string file_content = "";
    if (verifyExistence("GAMES/GAME_" + playerID)) {
        message = "RST ACT ";
        file_content = getContent("GAMES/GAME_" + playerID);
        message += "GAME_" + playerID + " " + std::to_string(file_content.size()) + " " + file_content + "\n";
    }
    else if (listDirectory("GAMES/" + playerID).size() != 0) {
        file_content = getLastGame(playerID);
        message = "RST FIN ";
        message += file_content + "\n";
    }
    else {
        message = "RST NOK\n";
    }
    size_t n = message.length();
    int nw = 0;
    int i = 0;
    while(n>0) {
        if ((nw=write(newfd, message.substr(i, n).c_str(),n))<=0) exit(1);
        printf("%s", message.substr(i, nw).c_str());
        n -= nw;
        i += nw;
    }
    printf("%s", message.c_str());
}

void makePlay(std::string playerID, char letter, int trial) {
    std::string status;
    char word[30]; // TODO : maybe change this to std::string
    strcpy(word, wordG.c_str());
    printf("%s\n", word);
    std::vector<int> pos = getPos(word, letter);
    int maxErrorsN = maxErrors(wordG);
    int errorsMade = getErrorsMade(playerID);
    int missing = getMissingNumber(playerID) > 0 ? getMissingNumber(playerID) : wordG.length();
    printf("missing: %d\n", missing);
    if (! verifyExistence("GAMES/GAME_" + playerID)) {
        status = "ERR";
    }
    else if (! isTrialValid(playerID, trial) && ! isRepeated(playerID, "T H " + std::string(1, letter) + " " + std::to_string(missing) + "\n") && ! isRepeated(playerID, "T M " + std::string(1, letter) + " " + std::to_string(missing) + "\n")) {
        status = "INV";
    }
    else if (isDup(playerID, std::string(1, letter)) && ! isRepeated(playerID, "T H " + std::string(1, letter) + " " + std::to_string(missing) + "\n") && ! isRepeated(playerID, "T M " + std::string(1, letter) + " " + std::to_string(missing) + "\n")) {
        status = "DUP";
    }
    else if (errorsMade + 1 > maxErrorsN) { 
        status = "OVR";
    }
    else if (missing - pos.size() == 0) {
        status = "WIN";
    }
    else if (pos.size() > 0) {
        status = "OK";
    }
    else {
        status = "NOK";
    }
    std::string message;
    message = "RLG " + status + " " + std::to_string(trial);
    size_t len = pos.size();
    if (status == "OK") {
        message += " " + std::to_string(pos.size());
        for (size_t i = 0; i < len; i++) {
            message += " " + std::to_string(pos[i] + 1);
        }
    }
    message += "\n";
    if ((status == "OK" || status == "WIN") && ! isRepeated(playerID, "T H " + std::string(1, letter) + " " + std::to_string(missing) + "\n")) {
        savePlay(playerID, "T", "H", std::string(1, letter), missing - pos.size());
    }
    else if (status == "NOK" && ! isRepeated(playerID, "T M " + std::string(1, letter) + " " + std::to_string(missing) + "\n")) {
        savePlay(playerID, "T", "M", std::string(1, letter), missing - pos.size());
    }
    if (status == "WIN") {
        saveScore(playerID);
        storeGame(playerID, "W");
    }
    else if (status == "OVR") {
        storeGame(playerID, "F");
    }
    printf("%s", message.c_str());
    sendto(fd,message.c_str(), message.length() ,0 , (struct sockaddr*)&addr, addrlen); // TODO : check if sends using n = 
}

void makeGuess(std::string playerID, std::string guess, int trial) {
    std::string status;
    int maxErrorsN = maxErrors(wordG);
    int errorsMade = getErrorsMade(playerID);
    int missing = getMissingNumber(playerID) > 0 ? getMissingNumber(playerID) : wordG.length();
    if (! verifyExistence("GAMES/GAME_" + playerID)) {
        status = "ERR";
    }
    else if (! isTrialValid(playerID, trial) && ! isRepeated(playerID,  "G H " + guess + " " + std::to_string(missing) + "\n") && ! isRepeated(playerID, status + "G M " + guess + " " + std::to_string(missing) + "\n")) {
        status = "INV";
    }
    else if (guess == wordG) {
        status = "WIN";
    }
    else if (errorsMade > maxErrorsN) {
        status = "OVR";
    }
    else {
        status = "NOK";
    }
    std::string message;
    message = "RWG " + status + " " + std::to_string(trial) + "\n";
    if (status == "WIN" && ! isRepeated(playerID, "G H " + guess + " " + std::to_string(missing) + "\n")) {
        savePlay(playerID, "G", "H", guess, 0);
        saveScore(playerID);
        storeGame(playerID, "W");
    }
    else if (status == "NOK" && ! isRepeated(playerID, "G M " + guess + " " + std::to_string(missing) + "\n") ) {
        savePlay(playerID, "G", "M", guess, missing);
    }
    else if (status == "OVR") {
        storeGame(playerID, "F");
    }
    sendto(fd, message.c_str(), message.length() , 0, (struct sockaddr*)&addr, addrlen); // TODO : check if sends using n =
}

