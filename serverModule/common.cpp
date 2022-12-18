#include "common.h"

/* Safely exit the server, freeing memory used */
void exitServer(int exitCode, int fd, struct addrinfo * res) {
    freeaddrinfo(res);
    close(fd);
    exit(exitCode);
}

/* Get word from filename game file */
std::string getWord(std::string fileName) {
    std::string line = getLine(fileName, 1);
    std::vector<std::string> words = stringSplit(line, ' ');
    return words[0];
}

/* Check if playerID has game and it is being played (has moves) */
int hasGame(std::string playerId) {
    return verifyExistence("GAMES/GAME_" + playerId) && getLineNumber("GAMES/GAME_" + playerId) > 1;
}

/* Get random seed for random word reading */
void getNewSeed(int * seed) {
    if ( ! verifyExistence("seed.txt") ) {
        std::ofstream file("seed.txt");
        file << 0;
        file.close();
        * seed = 0;
        return;
    }
    std::ifstream file("seed.txt");
    std::string line;
    std::getline(file, line);
    * seed = std::stoi(line);
    * seed = random(* seed, 0, 1000);
    file.close();
    std::ofstream file2("seed.txt");
    file2 << * seed;
    file2.close();
}