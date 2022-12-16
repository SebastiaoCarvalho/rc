#include "../utils.h"
#include "../filehandling.h"
#include "common.h"
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

/* Send TCP message */
void sendTCP(int fd, const char * message, size_t len);

/* Get the top 10 games */
std::string getScoreBoard();

/* Send ScoreBoard to the player application */
void sendScoreBoard(int newfd, bool verbose);

/* Get the name of the file that is the hint for playerID's game*/
std::string getImageFilename(std::string playerID, std::string wordsFileName);

/* Read image from filename into char buffer, saving its size */
void readImage(std::string filename, char ** content, size_t * size);

/* Send hint image content from playerID's game to player application */
void sendHint(int newfd, std::string playerID, std::string wordsFileName, bool verbose);

/* Get game summary from file filename */
std::string getSummary(std::string filename);

/* Get summary from last game played by playerID */
std::string getLastGameSummary(std::string playerID);

/* Send state from playerID's games to player application */
void sendState(int newfd, std::string playerID, bool verbose);


