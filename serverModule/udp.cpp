#include "udp.h"


/* Send UDP message */
void sendUDP(int fd, const char * message, size_t len, struct sockaddr_in addr, size_t addrlen) {
    int max_send = 5;
    int n = sendto(fd, message, len ,0 , (struct sockaddr*)&addr, addrlen);
    while (n == -1 && max_send > 0) {
        n = sendto(fd, message, len ,0 , (struct sockaddr*)&addr, addrlen);
        max_send--;
    }
    return;
}

/* Read random word from file */
void readWord(std::string fileName, std::string * word, std::string * hint, int * seed) {
    std::ifstream file(fileName);
    std::string line;
    std::vector<std::string> lines;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    getNewSeed(seed);
    int lineNumber = random(*seed, 0, lines.size() - 1);
    std::vector<std::string> split = stringSplit(lines[lineNumber], ' ');
    *word = split[0];
    *hint = split[1];
    std::cout << *word << std::endl;
    file.close();
}

/* Read sequential word from file */
void readSequentialWord(std::string fileName, std::string * word, std::string * hint, int * lineNumber) {
    std::ifstream file(fileName);
    std::string line;
    std::vector<std::string> lines;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    
    std::vector<std::string> split = stringSplit(lines[ *lineNumber], ' ');
    *word = split[0];
    *hint = split[1];
    std::cout << *word << std::endl;
    file.close();
    * lineNumber = (* lineNumber + 1) % lines.size();
}

/* Check if last line is the same as the new play (repeated message case) */
int isRepeated(std::string playerID, std::string play) {
    std::string line = getLastLine("GAMES/GAME_" + playerID);
    printf("line: %s\n", line.c_str());
    printf("play: %s\n", play.c_str());
    return play == line + "\n";
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

/* Start a game for playerID, wherre sequentialRead is true if read is sequential, else is random. 
    ReadArgNumber is line number if read is sequential, else is seed. 
*/
void startGame(std::string playerID, bool sequentialRead, int * readArgNumber, std::string fileName, int fd, 
    struct sockaddr_in addr, size_t addrlen, bool verbose) {
    std::string word;
    std::string hint;
    std::string status;
    std::string message;
    if (hasGame(playerID)) {
        status = "NOK";
        message = "RSG " + status + "\n";
    }
    else {
        status = "OK";
        if (! verifyExistence("GAMES/GAME_" + playerID)) { // get new word from word file
            if (sequentialRead) readSequentialWord(fileName, &word, &hint, readArgNumber); // read A
            else readWord(fileName, &word, &hint, readArgNumber);
            createGameFile(playerID, word, hint);
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
    sendUDP(fd, message.c_str(), message.size(), addr, addrlen); // TODO : check if sends using n = 
}

/* Exit playerID's game */
void quitGame(std::string playerID, int fd, struct sockaddr_in addr, size_t addrlen, bool verbose) {
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
    sendUDP(fd, message.c_str(), message.size(), addr, addrlen);
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

/* Save play in game file */
void savePlay(std::string playerID, std::string status, std::string hit, std::string play, int missing) {
    std::string filename = "GAMES/GAME_" + playerID;
    appendFile(filename, status + " " + hit + " " + play + " " + std::to_string(missing) + "\n");
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

/* Calculate the score for a game given his statistics  */
int scoreCalc(int errorsMax, int errorsMade, int trials) {
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

/* Try a letter play for playerID's game */
void makePlay(std::string playerID, char letter, int trial, int fd, struct sockaddr_in addr, size_t addrlen, bool verbose) {
    std::string status, message, word;
    std::vector<int> pos;
    int maxErrorsN, errorsMade, missing;
    if (! verifyExistence("GAMES/GAME_" + playerID) ) {
        message = "RLG ERR\n";
        sendUDP(fd, message.c_str(), message.size(), addr, addrlen);
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
    sendUDP(fd, message.c_str(), message.size(), addr, addrlen);
}

/* Make a word guess for playerID's game */
void makeGuess(std::string playerID, std::string guess, int trial, int fd, struct sockaddr_in addr, size_t addrlen, bool verbose) {
    std::string status, word, message;
    int maxErrorsN, errorsMade, missing;
    if (! verifyExistence("GAMES/GAME_" + playerID)) {
        message = "RWG ERR\n";
        sendUDP(fd, message.c_str(), message.size(), addr, addrlen);
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
    sendUDP(fd, message.c_str(), message.size(), addr, addrlen);
}