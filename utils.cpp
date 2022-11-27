#include <vector>
#include <unistd.h>
#include <string.h>
#include "utils.h"
#include <stdio.h>
#include <string>
#include <sstream>
#include <cstdlib>

// get all positions of a letter in a string
std::vector<int> getPos(char * string, char letter) {
    ssize_t len = strlen(string);
    std::vector<int> pos;
    for (int i = 0; i < len; i++) {
        if (string[i] == letter) {
            pos.push_back(i);
        }
    }
    return pos;
}

// get the maximum number of errors allowed for a word
int maxErrors (std::string s) {
    size_t len = s.length();
    if (len <= 6) {
        return 7;
    }
    else if (len <= 10) {
        return 8;
    }
    else {
        return 9;
    }
}

// split a string into a vector of strings using a delimiter
std::vector<std::string> stringSplit(std::string s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::stringstream ss(s);
    while (!ss.eof()) {
        std::getline(ss, token, delimiter);
        tokens.push_back(token);
    }
    return tokens;
}

// get a random number between min and max
int random(int min, int max) {
    return min + (rand() % (max - min + 1));
}