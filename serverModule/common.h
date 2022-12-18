#include "../utils.h"
#include "../filehandling.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string>
#include <vector>
#include <fstream>

/* Safely exit the server, freeing memory used */
void exitServer(int exitCode, int fd, struct addrinfo * res);

/* Get word from filename game file */
std::string getWord(std::string fileName);

/* Check if playerID has game and it is being played (has moves) */
int hasGame(std::string playerId);

/* Get random seed for random word reading */
void getNewSeed(int * seed);