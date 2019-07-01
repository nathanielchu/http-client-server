#include <string.h>

#include <string>
#include <fstream>
#include <sstream>

// read file into data, returns filelength
std::string read_file(const std::string dir, const char *path_cstr) 
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
        return buffer.str();
    }

    return std::string();
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

