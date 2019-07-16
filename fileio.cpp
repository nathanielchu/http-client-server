#include <string.h>

#include <string>
#include <fstream>
#include <sstream>

#include <iostream>

// read file into data, returns filelength
int read_file(const std::string dir, const char *path_cstr, std::string &body) 
{
    std::string path(path_cstr);
    char filename[dir.length() + path.length() + 2] = {'\0'};
    if (dir.substr(0,1) != ".")
        strcpy(filename, ".");
    strcat(filename, dir.c_str());
    strcat(filename, "/");
    strcat(filename, path.c_str());
    
    std::ifstream ifs(filename, std::ios::binary);
    if (ifs.is_open()) {
        std::stringstream buffer;
        buffer << ifs.rdbuf();
        ifs.close();
        body = buffer.str();
        std::cout << body << std::endl;
        return 0;
    }

    return -1;
}

// write file
int write_file(const std::string path, const char *contents, bool is_head) {
    std::ios_base::openmode flags = (is_head) ? std::ios::binary : (std::ios::binary | std::ios::app);
    std::ofstream ofs(path.c_str(), flags);
    if(ofs.is_open()) {
        ofs.write(contents, strlen(contents));
        ofs.close();
        return 0;
    }

    return -1;
}

