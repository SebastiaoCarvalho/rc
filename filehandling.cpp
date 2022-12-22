#include <string>
#include <cstdio>
#include <iostream>
#include <dirent.h>
#include <vector>
#include <fstream>
#include <sys/stat.h>
#include "filehandling.h"

/* Create Game File for playerID with word */
void createGameFile(std::string playerID, std::string word, std::string hint) {
    std::ofstream file;
    file.open("GAMES/GAME_" + playerID);
    file << word + " " + hint << std::endl;
    file.close();
}

/* Delete file given his name , returning 1 on sucess, 0 otherwise*/
int deleteFile(std::string filename) {
    return remove(filename.c_str()) == 0;
}

/* Return name of all files in directory */
std::vector<std::string> listDirectory(std::string dirName) {
    DIR *dir;
    struct dirent *ent;
    std::vector<std::string> files ;
    if ((dir = opendir (dirName.c_str())) != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) {
            if (ent->d_type != DT_DIR && ent->d_name[0] != '.') {
                files.push_back(ent->d_name);
            }
        }
        closedir (dir);
    }
    return files;
}

/* Check if file with name filename exists */
int verifyExistence(std::string filename) {
    std::ifstream file(filename);
    if (file.good()) {
        file.close();
        return 1;
    }
    else {
        file.close();
        return 0;
    }
}

/* Count number of lines in file */
int getLineNumber(std::string fileName) {
    std::ifstream file(fileName);
    std::string line;
    int line_number = 0;
    while (std::getline(file, line)) {
        line_number++;
    }
    return line_number;
}

/* Get line number lineNumber from file filename */
std::string getLine(std::string filename, int lineNumber) {
    if (! verifyExistence(filename)) {
        return NULL;
    }
    std::ifstream file(filename);
    std::string line;
    int line_number = 1;
    while (std::getline(file, line)) {
        if (line_number == lineNumber) {
            return line;
        }
        line_number++;
    }
    return NULL;
}


/* Get the last line of file filename */
std::string getLastLine(std::string filename) {
    if (! verifyExistence(filename)) {
        return NULL;
    }
    return getLine(filename, getLineNumber(filename));
}

/* Get the full content of file filename */
std::string getContent(std::string filename) {
    if (! verifyExistence(filename)) {
        return NULL;
    }
    std::string file_content = "";
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        file_content += line + "\n";
    }
    return file_content;
}

/* Append text to the end of file filename */
void appendFile(std::string filename, std::string text) {
    std::ofstream file;
    file.open(filename, std::ios::app);
    file << text;
    file.close();
}

/* Move file filename from old directory to newDir and change his name to newName, returning 0 on success, -1 otherwise */
int moveFile(std::string filename, std::string newDir, std::string newName) {
    if (! verifyExistence(newDir)) {
        mkdir(newDir.c_str(), 0777);
    }
    std::string newFilename = newDir + "/" + newName;
    return rename(filename.c_str(), newFilename.c_str());
}
