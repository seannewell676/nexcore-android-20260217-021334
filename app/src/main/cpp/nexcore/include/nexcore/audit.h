#pragma once
#include <string>
namespace nexcore {
bool audit_append(const std::string& base_dir, const std::string& json_line);
std::string audit_tail(int max_entries, const std::string& base_dir);
}
