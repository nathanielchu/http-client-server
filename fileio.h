#ifndef FILE_IO
#define FILE_IO

#include <string>

int read_file(const std::string dir, const char *path_cstr, std::string &body);
int write_file(const std::string path, const char *contents, bool is_head);

#endif // FILE_IO
