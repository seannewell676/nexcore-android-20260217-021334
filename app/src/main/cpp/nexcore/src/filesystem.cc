#include "nexcore/filesystem.h"
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fstream>
#include <cstring>

namespace nexcore {

std::string join(const std::string& a, const std::string& b) {
  if (a.empty()) return b;
  if (a.back() == '/') return a + b;
  return a + "/" + b;
}

static bool is_dir(const std::string& path) {
  struct stat st{};
  if (stat(path.c_str(), &st) != 0) return false;
  return S_ISDIR(st.st_mode);
}

bool ensure_dir(const std::string& path) {
  if (path.empty()) return false;
  if (is_dir(path)) return true;
  std::string cur;
  for (size_t i = 0; i < path.size(); ++i) {
    cur.push_back(path[i]);
    if (path[i] == '/' || i == path.size() - 1) {
      if (!cur.empty() && cur.back() == '/') cur.pop_back();
      if (!cur.empty() && !is_dir(cur)) {
        if (mkdir(cur.c_str(), 0700) != 0 && errno != EEXIST) { /* ignore */ }
      }
      if (i != path.size() - 1) cur.push_back('/');
    }
  }
  return is_dir(path);
}

bool write_text_file(const std::string& path, const std::string& content) {
  std::ofstream out(path, std::ios::binary);
  if (!out) return false;
  out.write(content.data(), (std::streamsize)content.size());
  return (bool)out;
}

bool read_text_file(const std::string& path, std::string* out) {
  std::ifstream in(path, std::ios::binary);
  if (!in) return false;
  std::string data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  *out = std::move(data);
  return true;
}

bool path_exists(const std::string& path) { return access(path.c_str(), F_OK) == 0; }

static bool remove_all_impl(const std::string& path) {
  struct stat st{};
  if (lstat(path.c_str(), &st) != 0) return true;
  if (S_ISDIR(st.st_mode)) {
    DIR* dir = opendir(path.c_str());
    if (!dir) return false;
    struct dirent* ent;
    while ((ent = readdir(dir)) != nullptr) {
      if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
      std::string child = join(path, ent->d_name);
      if (!remove_all_impl(child)) { closedir(dir); return false; }
    }
    closedir(dir);
    return rmdir(path.c_str()) == 0;
  }
  return unlink(path.c_str()) == 0;
}

bool remove_all(const std::string& path) { return remove_all_impl(path); }

static bool copy_file(const std::string& src, const std::string& dst) {
  std::ifstream in(src, std::ios::binary);
  if (!in) return false;
  std::ofstream out(dst, std::ios::binary);
  if (!out) return false;
  out << in.rdbuf();
  return (bool)out;
}

bool copy_dir_recursive(const std::string& src, const std::string& dst) {
  struct stat st{};
  if (stat(src.c_str(), &st) != 0) return false;
  if (!S_ISDIR(st.st_mode)) return false;
  ensure_dir(dst);

  DIR* dir = opendir(src.c_str());
  if (!dir) return false;
  struct dirent* ent;
  while ((ent = readdir(dir)) != nullptr) {
    if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
    std::string s = join(src, ent->d_name);
    std::string d = join(dst, ent->d_name);
    struct stat st2{};
    if (stat(s.c_str(), &st2) != 0) { closedir(dir); return false; }
    if (S_ISDIR(st2.st_mode)) {
      if (!copy_dir_recursive(s, d)) { closedir(dir); return false; }
    } else {
      if (!copy_file(s, d)) { closedir(dir); return false; }
    }
  }
  closedir(dir);
  return true;
}

}  // namespace nexcore
