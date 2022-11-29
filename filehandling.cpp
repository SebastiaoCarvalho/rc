#include <string>
#include <cstdio>
#include <iostream>
#include <dirent.h>
#include <vector>
#include "filehandling.h"

// delete file given his name
int deleteFile(std::string filename) {
    return remove(filename.c_str()) == 0;
}

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

