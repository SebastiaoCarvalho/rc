// Fix imports
#include "tcp.h"

/* Send TCP message */
void sendTCP(int fd, const char * message, size_t len) {
    int nw, max_send = 5;
    size_t n = len;
    while(n>0) {
        while ((nw=write(fd, message, n))<=0 && max_send > 0) {
            nw=write(fd, message,n);
            max_send--;
        }
        n -= nw;
        message += nw;
    }
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
void sendScoreBoard(int newfd, bool verbose) {
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
    sendTCP(newfd, message.c_str(), message.size());
}

/* Get the name of the file that is the hint for playerID's game*/
std::string getImageFilename(std::string playerID, std::string wordsFileName) {
    if (!verifyExistence("GAMES/GAME_" + playerID)) {
        return "";
    }
    std::string filename = "GAMES/GAME_" + playerID;
    printf("Filename: %s", filename.c_str());
    std::string line = getLine(wordsFileName, 1);
    std::vector<std::string> words = stringSplit(line, ' ');
    printf("Image filename: %s", words[1].c_str());
    return words[1];
}

// TODO : REMOVE THIS FUNCTION

// /* Read image from filename and return content */
// std::string readImage(std::string filename) {
//     // std::ifstream file(filename, std::ios::binary);
//     // std::ostringstream ss;
//     std::string image = "";
//     if (!verifyExistence(filename)) {
//         return image;
//     }
//     // ss << file.rdbuf();
//     // image = ss.str();
//     std::ifstream file(filename);
//     std::string line;
//     while (std::getline(file, line)) {
//         //printf("Line: %s", line.c_str());
//         image += line + "\n";
//     }
//     // printf("Image size: %ld", strlen(image.c_str()));
//     return image;
// }

/* Read image from filename into char buffer, saving its size */
void readImage(std::string filename, char ** content, size_t * size) {
    filename = "images/" + filename; 
    FILE * file = fopen(filename.c_str(), "r"); // search file in images directory
    unsigned char character;
    std::vector<unsigned char> image;
    while (!feof(file)) {
       character = getc(file);
       image.push_back(character);
    }
    fclose(file);
    *size = image.size();
    *content = new char[*size + 1];
    for (size_t i = 0; i < image.size(); i++) {
        (*content)[i] = image[i];
    }
    (*content)[*size] = '\0';
}

/* Send hint image content from playerID's game to player application */
void sendHint(int newfd, std::string playerID, std::string wordsFileName, bool verbose) {
    std::string message;
    size_t size;
    char * content = NULL;
    if (! verifyExistence("GAMES/GAME_" + playerID)) {
        message = "RHL NOK\n";
    }
    else {
        std::string word = stringSplit(getLine("GAMES/GAME_" + playerID, 1), ' ')[0];
        std::string imageName = getImageFilename(playerID, wordsFileName);
        if (imageName == "") {
            message = "RHL NOK\n";
        }
        else {
            readImage(imageName , &content, &size);
            if (content == NULL) {
                printf("Content is null\n");
            }
            else {
                printf("Content is not null\n");
            }
            message = "RHL OK " + imageName + " " + std::to_string(size) + " ";

        }
    }
    if (verbose) {
        printf("Replying with hint\n");
    }
    printf("Message: %s", message.c_str());
    sendTCP(newfd, message.c_str(), message.size());
    if (content != NULL) {
        sendTCP(newfd, content, size);
        //delete content ;
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
void sendState(int newfd, std::string playerID, bool verbose) {
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
    sendTCP(newfd, message.c_str(), message.size());
}