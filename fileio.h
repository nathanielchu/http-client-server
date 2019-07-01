#ifndef FILE_IO
#define FILE_IO

#include <string>

std::string read_file(const std::string dir, const char *path_cstr);
int write_file(const std::string path, const char *contents, bool is_head);

#endif // FILE_IO
