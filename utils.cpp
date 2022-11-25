#include <vector>
#include <unistd.h>
#include <string.h>
#include "utils.h"
#include <stdio.h>
#include <string>

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