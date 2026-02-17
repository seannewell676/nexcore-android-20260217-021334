#include "nexcore/audit.h"
#include "nexcore/filesystem.h"
#include "nexcore/hash.h"
#include <fstream>
#include <sstream>
#include <vector>

namespace nexcore {

static std::string audit_path(const std::string& base_dir) {
  return join(join(base_dir, "audit"), "log.jsonl");
}

static bool read_last_line(const std::string& path, std::string* out) {
  std::ifstream in(path, std::ios::binary);
  if (!in) return false;
  in.seekg(0, std::ios::end);
  auto size = in.tellg();
  if (size <= 0) return false;
  long long i = (long long)size - 1;
  for (; i >= 0; --i) {
    in.seekg(i);
    char c; in.get(c);
    if (c == '\n' && i != (long long)size - 1) { i++; break; }
  }
  if (i < 0) i = 0;
  in.seekg(i);
  std::string line;
  std::getline(in, line);
  *out = line;
  return !line.empty();
}

bool audit_append(const std::string& base_dir, const std::string& json_line) {
  ensure_dir(join(base_dir, "audit"));
  std::string path = audit_path(base_dir);

  std::string prev = "GENESIS";
  std::string last;
  if (read_last_line(path, &last)) prev = sha256_hex(last);

  std::string chained = json_line;
  if (!chained.empty() && chained.back() == '}') {
    chained.pop_back();
    chained += ,\"prev_hash\":\" + prev + """;
    chained += " ,\\"entry_hash\\":\\" + sha256_hex(json_line + prev) + ""}";
  } else {
    chained = "{"raw":" + json_line + ,\"prev_hash\":\" + prev + \",\"entry_hash\":\" + sha256_hex(json_line + prev) + ""}";
  }

  std::ofstream out(path, std::ios::app | std::ios::binary);
  if (!out) return false;
  out << chained << "\n";
  return true;
}

std::string audit_tail(int max_entries, const std::string& base_dir) {
  std::string path = audit_path(base_dir);
  std::ifstream in(path, std::ios::binary);
  if (!in) return "";
  std::vector<std::string> lines;
  std::string line;
  while (std::getline(in, line)) if (!line.empty()) lines.push_back(line);

  std::ostringstream o;
  int start = (int)lines.size() - max_entries;
  if (start < 0) start = 0;
  for (int i = start; i < (int)lines.size(); ++i) o << lines[i] << "\n";
  return o.str();
}

}  // namespace nexcore
