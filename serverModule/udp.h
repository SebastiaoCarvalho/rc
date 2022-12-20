#include "common.h"
#include "../utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <vector>
#include <fstream>

/* Send UDP message */
void sendUDP(int fd, const char * message, size_t len, struct sockaddr_in addr, size_t addrlen);

/* Read random word from file */
void readWord(std::string fileName, std::string * word, std::string * hint, int * seed);

/* Read sequential word from file */
void readSequentialWord(std::string fileName, std::string * word, std::string * hint, int * lineNumber);

/* Check if last line is the same as the new play (repeated message case) */
int isRepeated(std::string playerID, std::string play, int trial);

/* Get how many errors playerID made on his game */
int getErrorsMade(std::string playerID);

/* Get how many letters are missing from playerId's game */
int getMissingNumber(std::string playerId);

/* Start a game for playerID*/
void startGame(std::string playerID, bool sequentialRead, int * readArgNumber, std::string fileName, int fd, struct sockaddr_in addr, size_t addrlen, bool verbose);

/* Exit playerID's game */
void quitGame(std::string playerID, int fd, struct sockaddr_in addr, size_t addrlen, bool verbose);

/* Check if trial number is valid for playerID's game */
int isTrialValid(std::string playerID, int trial);

/* Checks if play is duplicate for playerID's game */
int isDup(std::string playerID, std::string play);

/* Save play in game file */
void savePlay(std::string playerID, std::string status, std::string hit, std::string play, int missing);

/* Store Game on his directory */
void storeGame(std::string playerID, std::string status);

/* Calculate the score for a game given his statistics  */
int scoreCalc(int errorsMax, int errorsMade, int trials);

/* Save score file for playerID's game */
void saveScore(std::string playerID);

/* Try a letter play for playerID's game */
void makePlay(std::string playerID, char letter, int trial, int fd, struct sockaddr_in addr, size_t addrlen, bool verbose);

/* Make a word guess for playerID's game */
void makeGuess(std::string playerID, std::string guess, int trial, int fd, struct sockaddr_in addr, size_t addrlen, bool verbose);