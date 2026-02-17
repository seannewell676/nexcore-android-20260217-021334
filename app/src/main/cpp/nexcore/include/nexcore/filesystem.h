#pragma once
#include <string>
namespace nexcore {
std::string join(const std::string& a, const std::string& b);
bool ensure_dir(const std::string& path);
bool write_text_file(const std::string& path, const std::string& content);
bool read_text_file(const std::string& path, std::string* out);
bool path_exists(const std::string& path);
bool remove_all(const std::string& path);
bool copy_dir_recursive(const std::string& src, const std::string& dst);
}
