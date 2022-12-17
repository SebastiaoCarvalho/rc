#include <vector>
#include <unistd.h>
#include <string.h>
#include "utils.h"
#include <stdio.h>
#include <string>
#include <sstream>
#include <cstdlib>

/* Get all positions of a letter in a string */
std::vector<int> getPos(std::string str, char letter) {
    ssize_t len = str.length();
    std::vector<int> pos;
    for (int i = 0; i < len; i++) {
        if (str[i] == letter) {
            pos.push_back(i);
        }
    }
    return pos;
}

/* Get the maximum number of errors allowed for a word */
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

/* Split a string into a vector of strings using a delimiter */
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

/* Get a random number between min and max inclusive */
int random(int seed, int min, int max) {
    srand(seed);
    return min + (rand() % (max - min + 1));
}

/* Get a string resulting of repeating s n times*/
std::string repeat(std::string s, int n) {
  std::string repeat;

  for (int i = 0; i < n; i++)
    repeat += s;

  return repeat;
}

/* Remove \n from strings*/
std::string strip(std::string s) {
  std::string stripped;

  for (size_t i = 0; i < s.length(); i++)
    if (s[i] != '\n')
      stripped += s[i];

  return stripped;
}

/* Return date format used for file naming */
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

/* return 1 if s is a number, 0 otherwise*/
int isNumber(std::string s) {
    for (size_t i = 0; i < s.length(); i++) {
        if (!isdigit(s[i])) {
            return 0;
        }
    }
    return 1;
}

/* Quicksort vector of strings */
std::vector<std::string> sortStringVector(std::vector<std::string> v) {
    if (v.size() <= 1) {
        return v;
    }
    std::vector<std::string> left;
    std::vector<std::string> right;
    std::vector<std::string> sorted;
    std::string pivot = v[v.size() / 2];
    v.erase(v.begin() + v.size() / 2);
    for (size_t i = 0; i < v.size(); i++) {
        if (v[i] < pivot) {
            left.push_back(v[i]);
        }
        else {
            right.push_back(v[i]);
        }
    }
    left = sortStringVector(left);
    right = sortStringVector(right);
    sorted.insert(sorted.end(), left.begin(), left.end());
    sorted.push_back(pivot);
    sorted.insert(sorted.end(), right.begin(), right.end());
    return sorted;
}

void copyString(char *p, std::string s) {
    size_t i;
    for (i = 0; i < s.length(); i++) {
        p[i] = s[i];
    }
    p[i] = '\0';
}

ssize_t readn(int fd, void *buffer, size_t n)
{
    ssize_t numRead;
    size_t totalRead;
    char *buf;
    
    buf = (char*)buffer;                       
    for (totalRead = 0; totalRead < n; ) {
        numRead = read(fd, buf, n - totalRead);

        if (numRead == 0)  // EOF
            return totalRead;             
        if (numRead == -1) {
            if (errno == EINTR)
                continue;               
            else
                return -1;              
        }
        totalRead += numRead;
        buf += numRead;
    }
    return totalRead;                     
}