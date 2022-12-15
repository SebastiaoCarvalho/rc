#include <string>

/* Delete file given his name */
int deleteFile(std::string filename);

/* Return name of all files in directory */
std::vector<std::string> listDirectory(std::string dirName);

/* Count number of lines in file */
int getLineNumber(std::string fileName);

/* Check if file with name filename exists */
int verifyExistence(std::string filename);

/* Get line number lineNumber from file filename */
std::string getLine(std::string filename, int lineNumber);

/* Get last line of file */
std::string getLastLine(std::string filename);

/* Get content of file */
void appendFile(std::string filename, std::string text);

/* Move file to new directory */
int moveFile(std::string filename, std::string newDir, std::string newName);

/* Get content of file */
std::string getContent(std::string filename);
