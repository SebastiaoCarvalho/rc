#include <string>

int deleteFile(std::string filename);
std::vector<std::string> listDirectory(std::string dirName);
int getLineNumber(std::string fileName);
int verifyExistence(std::string filename);
std::string getLine(std::string filename, int lineNumber);