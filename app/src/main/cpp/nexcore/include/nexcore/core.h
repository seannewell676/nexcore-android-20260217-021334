#pragma once
#include <string>

namespace nexcore {

struct ProposeResult {
  std::string json;
  std::string proposal_id;
};

ProposeResult propose(const std::string& user_text, const std::string& base_dir);
std::string execute(const std::string& proposal_id, bool approved, const std::string& base_dir);
std::string audit_tail(int max_entries, const std::string& base_dir);

}  // namespace nexcore
