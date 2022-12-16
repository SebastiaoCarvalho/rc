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
// Fix memory leaks (valgrind this b)
// check if send funcs are using string or char *
// comment stuff on other files, variables 
// check after game stored
// test state on finished game
// maybe change state to last summary only return content
// check error cases for makeplay and makeguess
// check if if else order on plays and guesses is right
// review global vars like fileName and wordG
// handle signals
// handle errors
// handle exits freeing stuff
// func remove scores 

// DUVIDAS:
// 1. Qual a função de score?
// 2. Palavra pode ter digitos?
// 3. Quando fazemos quit num jogo sem plays dá erro ou quit normal
// 4. State num jogo sem guesses deve dar
// 5. Devemos sempre dar exit quando ocorre erro ou há erros que devem ser tratados e continuar programa?
// 6. Como dar free do vetor?

struct sigaction act;
int fd, newfd, errcode, seed, lineNumber = 0;
socklen_t addrlen;
struct addrinfo hints,*res;
struct sockaddr_in addr;
std::string fileName;
std::string port = "58002";
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
    void exitServer(int errrorCode);
    void handleCtrlC(int s);
    void sendUDP(int fd, std::string message, struct sockaddr_in addr, size_t addrlen);

    ssize_t n;
    readFlags(argc, argv);
    bootServer();
    printf("Port: %s, File: %s, Verbose: %d\n", port.c_str(), fileName.c_str(), verbose);   
    int pid = fork();
    if (pid == -1) {
        perror("fork");
        exitServer(1);
    }
    signal(SIGINT, handleCtrlC);
    if (pid > 0)
    {

        char buffer[129];
        // child process
        fd=socket(AF_INET,SOCK_DGRAM,0); //UDP socket
        if(fd==-1) /*error*/exitServer(1);
        int iSetOption = 1;
        n = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&iSetOption, sizeof(iSetOption));
        if (n == -1) exitServer(1);
        memset(&hints,0,sizeof hints);
        hints.ai_family=AF_INET; // IPv4
        hints.ai_socktype=SOCK_DGRAM; // UDP socket
        hints.ai_flags=AI_PASSIVE;
        errcode=getaddrinfo(NULL, port.c_str(),&hints,&res);
        if(errcode!=0) /*error*/ exitServer(errcode);
        n=bind(fd,res->ai_addr, res->ai_addrlen);
        printf("%d\n", errno);
        if(n==-1) /*error*/ exitServer(1);
        while (1) {
            addrlen=sizeof(addr);
            printf("Waiting for message...\n");
            memset(buffer,0,128);
            n=recvfrom(fd,buffer, 128, 0, (struct sockaddr*)&addr,&addrlen);
            if(n==-1)/*error*/exitServer(1);
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
                int errCond = playerID.length() != 6  || ! isNumber(playerID) || tokens[2].length() != 1 || ! isNumber(tokens[3]);
                /*Read letter*/
                if (errCond ) {
                    sendUDP(fd, "RLG ERR\n", addr, addrlen);
                    continue;
                }
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
                int errCond = playerID.length() != 6  || ! isNumber(playerID)  || ! isNumber(tokens[3]);
                /*Read letter*/
                if (errCond ) {
                    sendUDP(fd, "RLG ERR\n", addr, addrlen);
                    continue;
                }

                
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
                if (verbose) {
                    printf("Received invalid message from IP address: %s and port: %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
                }
                n=sendto(fd, "ERR\n", 128 ,0 , (struct sockaddr*)&addr, addrlen);
                if (n==-1)/*error*/ exitServer(1);
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
        if(sigaction(SIGCHLD,&act,NULL)==-1)/*error*/exitServer(1);
        if((fd=socket(AF_INET,SOCK_STREAM,0))==-1)exitServer(1);//error
        int iSetOption = 1;
        n = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&iSetOption, sizeof(iSetOption));
        if (n == -1) exitServer(1);
        memset(&hints,0,sizeof hints);
        hints.ai_family=AF_INET;//IPv4
        hints.ai_socktype=SOCK_STREAM;//TCP socket
        hints.ai_flags=AI_PASSIVE;
        if ((ret=getaddrinfo(NULL, port.c_str(), &hints, &res))!=0)/*error*/exitServer(1);
        if (bind(fd,res->ai_addr,res->ai_addrlen)==-1)/*error*/exitServer(1);
        if (listen(fd,5)==-1)/*error*/exitServer(1);

        while(1) {
            printf("Waiting for connection...\n");
            memset(buffer,0,128);
            addrlen=sizeof(addr);
            do newfd=accept(fd,(struct sockaddr*)&addr,&addrlen);//wait for a connection
            while(newfd==-1&&errno==EINTR);
            if(newfd==-1)/*error*/exitServer(1);
            if((pid=fork())==-1)/*error*/exitServer(1);
            else if(pid==0) { // child process
                
                n = read(newfd, buffer, 128);
                if (n == -1) /*error*/ exitServer(1);
                buffer[128] = '\0';
                std::string message = buffer;
                message = strip(message);
                std::vector<std::string> tokens = stringSplit(message, ' ');
                if (strncmp(buffer, "GSB", 3) == 0) {
                    if (verbose) {
                        printf("Received GSB from IP address: %s and port: %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
                    }
                    sendScoreBoard(newfd);
                }
                else if (strncmp(buffer, "GHL", 3) == 0) {
                    std::string playerID = tokens[1];
                    if (verbose) {
                        printf("Received GHL with playerID: %s from IP address: %s and port: %d\n", playerID.c_str(), inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
                    }                    
                    sendHint(newfd, playerID);
                }
                else if (strncmp(buffer, "STA", 3) == 0) {
                    std::string playerID = tokens[1];
                    if (verbose) {
                        printf("Received STA with playerID: %s from IP address: %s and port: %d\n", playerID.c_str(), inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
                    }
                    sendState(newfd, playerID);
                }
                else {
                    message = "ERR\n";
                    if (verbose) {
                        printf("Received invalid message from IP address: %s and port: %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
                    }
                    int nw = 0;
                    int i = 0;
                    while(n>0) {
                        if ((nw=write(newfd, message.substr(i, n).c_str(),n))<=0) exitServer(1);
                        printf("%s", message.substr(i, nw).c_str());
                        n -= nw;
                        i += nw;
                    }
                }
                close(newfd); 
                exit(0);
            }
            // parent process
            do ret=close(newfd);while(ret==-1&&errno==EINTR);
            if(ret==-1)/*error*/exitServer(1);
        }
    }
}

/* Safely exit the server, freeing memory used */
void exitServer(int exitCode) {
    freeaddrinfo(res);
    close(fd);
    exit(exitCode);
}

/* Send UDP message */
void sendUDP(int fd, std::string message, struct sockaddr_in addr, size_t addrlen) {
    int n = sendto(fd, message.c_str(), message.length() ,0 , (struct sockaddr*)&addr, addrlen);
    if (n==-1)/*error*/ exitServer(1);
}

/* Send TCP message */
void sendTCP(int fd, std::string message) {
    int nw, i = 0;
    size_t n = message.length();
    while(n>0) {
        if ((nw=write(fd, message.substr(i, n).c_str(),n))<=0) exitServer(1);
        n -= nw;
        i += nw;
    }
}

/* Handle Ctrl C Event (SIGINT) */
void handleCtrlC(int s){
    printf("Caught exitting signal.\n Exiting...\n");
    exitServer(s);
}

/* Read flags from command line */
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

/* Get random seed for random word reading */
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

/* Read random word from file */
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

/* Read sequential word from file */
void readSequentialWord(std::string fileName, std::string * word) {
    std::ifstream file(fileName);
    std::string line;
    std::vector<std::string> lines;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    
    *word = stringSplit(lines[lineNumber], ' ')[0];
    std::cout << *word << std::endl;
    file.close();
    lineNumber = (lineNumber + 1) % lines.size();
}

/* Create Game File for playerID with word */
void createGameFile(std::string playerID, std::string word) {
    std::ofstream file;
    file.open("GAMES/GAME_" + playerID);
    file << word + " " + word + ".png" << std::endl;
    file.close();
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
    files.clear();
    std::vector<std::string>().swap(files); // free memory
}

/* Check if playerID has game and it is being played (has moves) */
int hasGame(std::string playerId) {
    return verifyExistence("GAMES/GAME_" + playerId) && getLineNumber("GAMES/GAME_" + playerId) > 1;
}

/* Start a game for playerID*/
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
            readSequentialWord(fileName, &word); 
            createGameFile(playerID, word);
        }
        else { // get word from game file    
            word = stringSplit(getLine("GAMES/GAME_" + playerID, 1), ' ')[0]; 
        }
        std::string letterNumber = std::to_string(word.length());
        int errorsN = maxErrors(word);
        std::string maxErrors = std::to_string(errorsN);
        message = "RSG " + status + " " + letterNumber + " " + maxErrors + "\n";
    }
    if (verbose) printf("Replying with : %s", message.c_str());
    sendUDP(fd, message.c_str(), addr, addrlen); // TODO : check if sends using n = 
}

/* Check if last line is the same as the new play (repeated message case) */
int isRepeated(std::string playerID, std::string play) {
    std::string line = getLastLine("GAMES/GAME_" + playerID);
    printf("line: %s\n", line.c_str());
    printf("play: %s\n", play.c_str());
    return play == line + "\n";
}

/* Save play in game file */
void savePlay(std::string playerID, std::string status, std::string hit, std::string play, int missing) {
    std::string filename = "GAMES/GAME_" + playerID;
    appendFile(filename, status + " " + hit + " " + play + " " + std::to_string(missing) + "\n");
}

/* Get how many errors playerID made on his game */
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

/* Get how many letters are missing from playerId's game */
int getMissingNumber(std::string playerId) {
    if (! hasGame(playerId)) {
        return -1;
    }
    std::string line = getLine("GAMES/GAME_" + playerId, getLineNumber("GAMES/GAME_" + playerId));
    std::string missing = stringSplit(line, ' ')[3];
    return atoi(missing.c_str());
}

/* Store Game on his directory */
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

int scoreCalc(int errors, int errorsMade, int trials) {
    int score;
    if (trials < 6)
        score = 100 - trials * 15;
    else
        score = 100 - trials * 10;
    return score > 0 ? score : 0;
}

/* Save score file for playerID's game */
void saveScore(std::string playerID) {
    std::string filename = "GAMES/GAME_" + playerID;
    std::string word = sortStringVector(stringSplit(getLine(filename, 1), ' '))[0];
    int errors = maxErrors(word);
    int trials = getLineNumber(filename) - 1;
    int errorsMade = getErrorsMade(playerID);
    int succ = trials - errorsMade;
    //int score = (errors - errorsMade) * 100 / errors;
    int score = scoreCalc(errors, errorsMade, trials);
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

/* Exit playerID's game */
void quitGame(std::string playerID) {
    std::string status;
    if (verifyExistence("GAMES/GAME_" + playerID)) {
        status = "OK";
        storeGame(playerID, "Q");
    }
    else {
        status = "ERR";
    }
    std::string message = "RQT " + status + "\n";
    if (verbose) printf("Replying with : %s", message.c_str());
    sendUDP(fd, message.c_str(), addr, addrlen);
}

/* Check if trial number is valid for playerID's game */
int isTrialValid(std::string playerID, int trial) {
    int line_number = getLineNumber("GAMES/GAME_" + playerID);
    printf("line_number: %d\n", line_number);
    return trial == line_number;
}

/* Checks if play is duplicate for playerID's game */
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

/* Get word from filename game file */
std::string getWord(std::string fileName) {
    std::string line = getLine(fileName, 1);
    std::vector<std::string> words = stringSplit(line, ' ');
    return words[0];
}

/* Get the top 10 games */
std::string getScoreBoard() {
    std::vector<std::string> files = listDirectory("SCORES");
    if (files.size() == 0) {
        return "";
    }
    std::string scoreboard = "\n----------------------------- TOP " + 
    std::to_string(files.size()) + 
    " SCORES -----------------------------\n";
    scoreboard += "\n";
    scoreboard += "SCORE PLAYER     WORD                      GOOD TRIALS  TOTAL TRIALS\n\n";
    ssize_t size = files.size();
    for (ssize_t i = size - 1; i >= 0 && i >= size - 10; i--) {
        printf("File: %s, i: %d", files[i].c_str(), i);
        std::string line = getLine("SCORES/" + files[i], 1);
        std::vector<std::string> words = stringSplit(line, ' ');
        std::string word = words[2];
        while (word.length() < 32) {
            word += " ";
        }
        scoreboard += " " + words[0] + "  " + words[1] + "  " + word + " " + 
        words[3] + repeat(" ", 13) + words[4] + "\n";
    }
    return scoreboard;
}

/* Send ScoreBoard to the player application */
void sendScoreBoard(int newfd) {
    std::string scoreboard = getScoreBoard();
    std::string message;
    if (scoreboard == "") {
        message = "RSB EMPTY\n";
    }
    else {
        printf("Scoreboard: %s", scoreboard.c_str());
        message = "RSB OK scoreboard.txt " + std::to_string(scoreboard.size()) + " " +  scoreboard + "\n";
    }
    if (verbose) {
        printf("Replying with scoreboard\n");
    }
    sendTCP(newfd, message.c_str());
}

/* Read image from filename and return content */
std::string readImage(std::string filename) {
    // std::ifstream file(filename, std::ios::binary);
    // std::ostringstream ss;
    std::string image = "";
    if (!verifyExistence(filename)) {
        return image;
    }
    // ss << file.rdbuf();
    // image = ss.str();
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        //printf("Line: %s", line.c_str());
        image += line + "\n";
    }
    // printf("Image size: %ld", strlen(image.c_str()));
    return image;
}

void readImage2(std::string filename, char ** content, size_t * size) {
    FILE * file = fopen(filename.c_str(), "r");
    unsigned char character;
    std::vector<unsigned char> image;
    while (!feof(file)) {
       character = getc(file);
       image.push_back(character);
    }
    printf("read\n");
    fclose(file);
    *size = image.size();
    *content = new char[*size + 1];
    printf("created\n");
    for (int i = 0; i < image.size(); i++) {
        printf("i: %d, image[i]: %d\n", i, image[i]);
        (*content)[i] = image[i];
    }
    printf("copied\n");
    (*content)[*size] = '\0';
    printf("Content %s\n", content);
}

/* Send hint image content from playerID's game to player application */
void sendHint(int newfd, std::string playerID) {
    std::string message;
    size_t size;
    char * content = NULL;
    printf("a\n");
    if (! verifyExistence("GAMES/GAME_" + playerID)) {
        message = "RHL NOK\n";
    }
    else {
        std::string word = stringSplit(getLine("GAMES/GAME_" + playerID, 1), ' ')[0];
        std::string image = readImage("images/transfer.jpg");
        if (image == "") {
            message = "RHL NOK\n";
        }
        else {
            /* char *cstr = new char[image.size() + 1];
            memset(cstr, 0, image.size() + 1);
            printf("cstr size %ld | %ld\n", sizeof(char *) / sizeof(cstr[0]), image.size());
            copyString(cstr, image);
            content = cstr; */
            readImage2("images/transfer.jpg", &content, &size);
            if (content == NULL) {
                printf("Content is null\n");
            }
            else {
                printf("Content is not null\n");
                //store content in file

            }
            message = "RHL OK hint.txt " + std::to_string(size) + " ";
            // store message in file
            /* std::ofstream file("hint.jpg");
            file << image;
            file.close(); */
        }
    }
    if (verbose) {
        printf("Replying with hint\n");
    }
    printf("Message: %s", message.c_str());
    sendTCP(newfd, message.c_str());
    if (content != NULL) {
        FILE * file = fopen("bhint.jpg", "w");
        int nw, n = size;
        while(n>0) {
            if ((nw=write(newfd, content,n))<=0) exitServer(1);
            fwrite(content, sizeof(char), nw, file);
            n -= nw;
            content += nw;
        }
        fclose(file);
        delete [] content;
    }
}

/* Get game summary from file filename */
std::string getSummary(std::string filename) {
    int trials = getLineNumber(filename) - 1;
    std::string word = getWord(filename);
    std::string game = repeat("-", word.length());
    std::string summary;
    if (trials > 0) summary = std::to_string(trials) + " transactions found:\n";
    else summary = "No transactions found\n";
    for (int i = 1; i <= trials; i++) {
        std::string line = getLine(filename, i + 1);
        printf("line: %s\n", line.c_str());
        std::vector<std::string> words = stringSplit(line, ' ');
        if (words[0] == "T") {
            summary += "Letter trial: " + words[2] + "\n";
            size_t len = word.length();
            for (size_t j = 0; j < len; j++) {
                if (word[j] == words[2][0]) {
                    game[j] = words[2][0];
                }
            }
        }
        else {
            summary += "Word trial: " + words[2] + "\n";
        }
        summary += "Solved so far: " + game + "\n";
    }
    return summary;
}

/* Get summary from last game played by playerID */
std::string getLastGameSummary(std::string playerID) {
    std::vector<std::string> files = sortStringVector(listDirectory("GAMES/" + playerID));
    ssize_t size = files.size();
    std::string filename = "GAMES/" + playerID + "/" + files[size - 1];
    std::string file_content = getSummary(filename);
    std::string res = "STATE_" + playerID + ".txt " + std::to_string(file_content.size()) + " " 
    + "Finished game found for player " + playerID + "\n" + file_content;
    printf("res: %s", res.c_str());
    return res;
}

/* Send state from playerID's games to player application */
void sendState(int newfd, std::string playerID) {
    std::string message, file_content = "";
    if (verifyExistence("GAMES/GAME_" + playerID)) {
        message = "RST ACT ";
        file_content = "Active game found for player " + playerID + "\n" + getSummary("GAMES/GAME_" + playerID);
        message += "STATE_" + playerID + ".txt " + std::to_string(file_content.size()) + " " + file_content + "\n";
    }
    else if (listDirectory("GAMES/" + playerID).size() != 0 && playerID.length() == 6) { // find bug here
        file_content = getLastGameSummary(playerID);
        message = "RST FIN ";
        message += file_content + "\n";
    }
    else {
        message = "RST NOK\n";
    }
    if (verbose) {
        printf("Replying with state\n");
    }
    sendTCP(newfd, message.c_str());
}

/* Try a letter play for playerID's game */
void makePlay(std::string playerID, char letter, int trial) {
    std::string status, message, word;
    std::vector<int> pos;
    int maxErrorsN, errorsMade, missing;
    if (! verifyExistence("GAMES/GAME_" + playerID) ) {
        message = "RLG ERR\n";
        sendUDP(fd, message.c_str(), addr, addrlen);
        return;
    }
    word = getWord("GAMES/GAME_" +  playerID);
    pos = getPos(word, letter);
    maxErrorsN = maxErrors(word);
    errorsMade = getErrorsMade(playerID);
    missing = getMissingNumber(playerID) > 0 ? getMissingNumber(playerID) : word.length();
    printf("missing: %d\n", missing);
    if (! isTrialValid(playerID, trial) && ! isRepeated(playerID, "T H " + std::string(1, letter) + " " + std::to_string(missing) + "\n") && ! isRepeated(playerID, "T M " + std::string(1, letter) + " " + std::to_string(missing) + "\n")) {
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
    if (verbose) {
        printf("Replying with: %s\n", message.c_str());
    }    
    sendUDP(fd,message.c_str(), addr, addrlen);
}

/* Make a word guess for playerID's game */
void makeGuess(std::string playerID, std::string guess, int trial) {
    std::string status, word, message;
    int maxErrorsN, errorsMade, missing;
    if (! verifyExistence("GAMES/GAME_" + playerID)) {
        message = "RWG ERR\n";
        sendUDP(fd, message.c_str(), addr, addrlen);
        return;
    }
    maxErrorsN = maxErrors(word);
    errorsMade = getErrorsMade(playerID);
    missing = getMissingNumber(playerID) > 0 ? getMissingNumber(playerID) : word.length();
    word = getWord("GAMES/GAME_" + playerID);
    if (! isTrialValid(playerID, trial) && ! isRepeated(playerID,  "G H " + guess + " " + std::to_string(missing) + "\n") && ! isRepeated(playerID, status + "G M " + guess + " " + std::to_string(missing) + "\n")) {
        status = "INV";
    }
    else if (guess == word) {
        status = "WIN";
    }
    else if (errorsMade > maxErrorsN) {
        status = "OVR";
    }
    else {
        status = "NOK";
    }
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
    if (verbose) {
        printf("Replying with: %s\n", message.c_str());
    }
    sendUDP(fd, message.c_str(), addr, addrlen);
}

