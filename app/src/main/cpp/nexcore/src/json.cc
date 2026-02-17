#include "nexcore/json.h"
#include <sstream>

namespace nexcore::json {

std::string escape(const std::string& s) {
  std::ostringstream o;
  for (char c : s) {
    switch (c) {
      case '\"': o << "\\\""; break;
      case '\\': o << "\\\\"; break;
      case '\b': o << "\\b"; break;
      case '\f': o << "\\f"; break;
      case '\n': o << "\\n"; break;
      case '\r': o << "\\r"; break;
      case '\t': o << "\\t"; break;
      default:
        if (static_cast<unsigned char>(c) < 0x20) o << "\\u" << std::hex << (int)c;
        else o << c;
    }
  }
  return o.str();
}

std::string str(const std::string& s) { return std::string("\"") + escape(s) + "\""; }
std::string num(long long n) { return std::to_string(n); }
std::string boolean(bool b) { return b ? "true" : "false"; }

std::string obj(const std::vector<std::pair<std::string, std::string>>& kv_raw_json) {
  std::ostringstream o;
  o << "{";
  bool first = true;
  for (const auto& kv : kv_raw_json) {
    if (!first) o << ",";
    first = false;
    o << str(kv.first) << ":" << kv.second;
  }
  o << "}";
  return o.str();
}

}  // namespace nexcore::json
