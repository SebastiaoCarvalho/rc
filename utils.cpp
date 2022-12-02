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

std::string repeat(std::string s, int n) {
  std::string repeat;

  for (int i = 0; i < n; i++)
    repeat += s;

  return repeat;
}

// remove \n from strings
std::string strip(std::string s) {
  std::string stripped;

  for (size_t i = 0; i < s.length(); i++)
    if (s[i] != '\n')
      stripped += s[i];

  return stripped;
}

std::string getDateFormatted(tm *ltm) {
    std::string year = std::to_string(1900 + ltm->tm_year);
    std::string month = std::to_string(1 + ltm->tm_mon);
    if (month.length() == 1) {
        month = "0" + month;
    }
    std::string day = std::to_string(ltm->tm_mday);
    if (day.length() == 1) {
        day = "0" + day;
    }
    std::string hour = std::to_string(ltm->tm_hour);
    if (hour.length() == 1) {
        hour = "0" + hour;
    }
    std::string min = std::to_string(ltm->tm_min);
    if (min.length() == 1) {
        min = "0" + min;
    }
    std::string sec = std::to_string(ltm->tm_sec);
    if (sec.length() == 1) {
        sec = "0" + sec;
    }
    return year + month + day + "_" + hour + min + sec;
}