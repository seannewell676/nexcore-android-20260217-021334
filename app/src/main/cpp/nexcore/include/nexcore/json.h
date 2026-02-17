#pragma once
#include <string>
#include <vector>
#include <utility>

namespace nexcore::json {
std::string escape(const std::string& s);
std::string obj(const std::vector<std::pair<std::string, std::string>>& kv_raw_json);
std::string str(const std::string& s);
std::string num(long long n);
std::string boolean(bool b);
}  // namespace nexcore::json
