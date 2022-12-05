#include <string>
#include <cstdio>
#include <iostream>
#include <dirent.h>
#include <vector>
#include <fstream>
#include <sys/stat.h>
#include "filehandling.h"

// delete file given his name
int deleteFile(std::string filename) {
    return remove(filename.c_str()) == 0;
}

// return name of all files in directory
std::vector<std::string> listDirectory(std::string dirName) {
    DIR *dir;
    struct dirent *ent;
    std::vector<std::string> files;
    if ((dir = opendir (dirName.c_str())) != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) {
            if (ent->d_type != DT_DIR) {
                files.push_back(ent->d_name);
            }
        }
    }
    return files;
}

// count number of lines in file
int getLineNumber(std::string fileName) {
    std::ifstream file(fileName);
    std::string line;
    int line_number = 0;
    while (std::getline(file, line)) {
        line_number++;
    }
    return line_number;
}

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

std::string getLine(std::string filename, int lineNumber) {
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

std::string getContent(std::string filename) {
    std::string file_content = "";
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        file_content += line + "\n";
    }
    return file_content;
}

void appendFile(std::string filename, std::string text) {
    std::ofstream file;
    file.open(filename, std::ios::app);
    file << text;
    file.close();
}

int moveFile(std::string filename, std::string newDir, std::string newName) {
    if (! verifyExistence(newDir)) {
        mkdir(newDir.c_str(), 0777);
    }
    std::string newFilename = newDir + "/" + newName;
    return rename(filename.c_str(), newFilename.c_str()) == 0;
}
