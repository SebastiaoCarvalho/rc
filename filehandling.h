#include <string>

int deleteFile(std::string filename);
std::vector<std::string> listDirectory(std::string dirName);
int getLineNumber(std::string fileName);
int verifyExistence(std::string filename);
std::string getLine(std::string filename, int lineNumber);
std::string getLastLine(std::string filename);
void appendFile(std::string filename, std::string text);
int moveFile(std::string filename, std::string newDir, std::string newName);
std::string getContent(std::string filename);
