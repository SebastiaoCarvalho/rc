#include <vector>
#include <string>

/* Get all positions of a letter in a string */
std::vector<int> getPos(std::string str, char letter); 

/* Get the maximum number of errors allowed for a word */
int maxErrors (std::string s);

/* Split a string into a vector of strings using a delimiter */
std::vector<std::string> stringSplit(std::string s, char delimiter);

/* Get a random number between min and max inclusive */
int random(int seed, int min, int max);

/* Get a string resulting of repeating s n times */
std::string repeat(std::string s, int n);

/* Remove \n from strings */
std::string strip(std::string s);

/* Get the current date and time */
std::string getDateFormatted(tm *ltm);

/* Get the current date and time */
int isNumber(std::string s);    

/* Quicksort vector of strings, returning the sorted vector */
std::vector<std::string> sortStringVector(std::vector<std::string> v);

/* Read until size n */
ssize_t readn(int fd, void *buffer, size_t n);
