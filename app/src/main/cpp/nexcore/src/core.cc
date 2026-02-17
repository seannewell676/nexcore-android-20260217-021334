#include "nexcore/core.h"
#include "nexcore/json.h"
#include "nexcore/filesystem.h"
#include "nexcore/audit.h"
#include "nexcore/hash.h"
#include <ctime>
#include <fstream>

namespace nexcore {

static long long now_ms() {
  timespec ts{};
  clock_gettime(CLOCK_REALTIME, &ts);
  return (long long)ts.tv_sec * 1000LL + ts.tv_nsec / 1000000LL;
}

static std::string make_id(const std::string& seed) {
  return sha256_hex(seed + std::to_string(now_ms())).substr(0, 16);
}

static std::string workspace_dir(const std::string& base_dir) { return join(base_dir, "workspace"); }
static std::string snapshots_dir(const std::string& base_dir) { return join(base_dir, "snapshots"); }
static std::string proposal_store_path(const std::string& base_dir) { return join(join(base_dir, "state"), "proposals.jsonl"); }

static bool store_proposal(const std::string& base_dir, const std::string& proposal_json) {
  ensure_dir(join(base_dir, "state"));
  std::ofstream out(proposal_store_path(base_dir), std::ios::app | std::ios::binary);
  if (!out) return false;
  out << proposal_json << "\n";
  return true;
}

static bool load_proposal(const std::string& base_dir, const std::string& proposal_id, std::string* out_json) {
  std::ifstream in(proposal_store_path(base_dir), std::ios::binary);
  if (!in) return false;
  std::string line;
  while (std::getline(in, line)) {
    if (line.find("\"proposal_id\":\"" + proposal_id + "\"") != std::string::npos) {
      *out_json = line;
      return true;
    }
  }
  return false;
}

static std::string infer_intent(const std::string& user_text) {
  std::string low = user_text;
  for (auto& c : low) c = (char)tolower(c);
  if (low.find("draft") != std::string::npos || low.find("message") != std::string::npos) return "DRAFT_MESSAGE";
  if (low.find("summarize") != std::string::npos || low.find("summary") != std::string::npos) return "SUMMARIZE_DOCS";
  return "WRITE_NOTE";
}

static void ensure_runtime_dirs(const std::string& base_dir) {
  ensure_dir(base_dir);
  ensure_dir(workspace_dir(base_dir));
  ensure_dir(snapshots_dir(base_dir));
  ensure_dir(join(base_dir, "audit"));
  ensure_dir(join(base_dir, "state"));
}

ProposeResult propose(const std::string& user_text, const std::string& base_dir) {
  ensure_runtime_dirs(base_dir);

  const std::string intent = infer_intent(user_text);
  const std::string proposal_id = make_id(user_text);
  const bool is_write = (intent == "WRITE_NOTE");
  const bool reversible = is_write;
  const std::string decision = is_write ? "CONFIRM" : "ALLOW";

  const std::string evidence = json::obj({
    {"input_digest", json::str(sha256_hex(user_text))},
    {"sources", json::obj({{"type", json::str("user_text")}})},
    {"assumptions", json::str(is_write ? "Writing only inside sandbox workspace" : "Read-only output")}
  });

  const std::string risk = json::obj({
    {"is_write", json::boolean(is_write)},
    {"reversible", json::boolean(reversible)},
    {"sensitivity", json::str(is_write ? "filesystem_sandbox" : "none")},
    {"ambiguity_score", json::num(is_write ? 10 : 1)}
  });

  std::string steps = (intent == "DRAFT_MESSAGE")
    ? json::str("Generate a draft artifact only. No sending.")
    : (intent == "SUMMARIZE_DOCS")
        ? json::str("Summarize provided text. No writes.")
        : json::str("Create a note file in workspace and verify it.");

  const std::string proposal = json::obj({
    {"proposal_id", json::str(proposal_id)},
    {"intent", json::str(intent)},
    {"user_text", json::str(user_text)},
    {"steps", steps},
    {"policy_decision", json::str(decision)},
    {"evidence", evidence},
    {"risk", risk},
    {"timestamp_ms", json::num(now_ms())}
  });

  store_proposal(base_dir, proposal);
  audit_append(base_dir, json::obj({
    {"event", json::str("PROPOSE")},
    {"proposal_id", json::str(proposal_id)},
    {"intent", json::str(intent)},
    {"decision", json::str(decision)},
    {"timestamp_ms", json::num(now_ms())}
  }));

  return {proposal, proposal_id};
}

static std::string snapshot_path(const std::string& base_dir, const std::string& snapshot_id) {
  return join(snapshots_dir(base_dir), "ws_" + snapshot_id);
}

static void snapshot_take(const std::string& base_dir, const std::string& snapshot_id) {
  std::string snap = snapshot_path(base_dir, snapshot_id);
  remove_all(snap);
  ensure_dir(snap);
  copy_dir_recursive(workspace_dir(base_dir), snap);
}

static bool snapshot_restore(const std::string& base_dir, const std::string& snapshot_id) {
  std::string snap = snapshot_path(base_dir, snapshot_id);
  if (!path_exists(snap)) return false;
  remove_all(workspace_dir(base_dir));
  ensure_dir(workspace_dir(base_dir));
  return copy_dir_recursive(snap, workspace_dir(base_dir));
}

std::string execute(const std::string& proposal_id, bool approved, const std::string& base_dir) {
  ensure_runtime_dirs(base_dir);

  if (!approved) return json::obj({{"error", json::str("Not approved")}, {"proposal_id", json::str(proposal_id)}});

  std::string proposal_json;
  if (!load_proposal(base_dir, proposal_id, &proposal_json)) {
    return json::obj({{"error", json::str("Unknown proposal_id")}, {"proposal_id", json::str(proposal_id)}});
  }

  std::string intent = "UNKNOWN";
  if (proposal_json.find("\"intent\":\"DRAFT_MESSAGE\"") != std::string::npos) intent = "DRAFT_MESSAGE";
  else if (proposal_json.find("\"intent\":\"SUMMARIZE_DOCS\"") != std::string::npos) intent = "SUMMARIZE_DOCS";
  else if (proposal_json.find("\"intent\":\"WRITE_NOTE\"") != std::string::npos) intent = "WRITE_NOTE";

  long long ts = now_ms();

  if (intent == "DRAFT_MESSAGE") {
    std::string draft = "Draft: " + proposal_id + "\n\n(This is a draft artifact. No sending.)";
    auto receipt = json::obj({
      {"event", json::str("EXECUTE_DRAFT")},
      {"proposal_id", json::str(proposal_id)},
      {"ok", json::boolean(true)},
      {"artifact", json::str(draft)},
      {"timestamp_ms", json::num(ts)}
    });
    audit_append(base_dir, receipt);
    return receipt;
  }

  if (intent == "SUMMARIZE_DOCS") {
    std::string summary = "Summary (demo): " + proposal_json.substr(0, 200) + "...";
    auto receipt = json::obj({
      {"event", json::str("EXECUTE_SUMMARY")},
      {"proposal_id", json::str(proposal_id)},
      {"ok", json::boolean(true)},
      {"summary", json::str(summary)},
      {"timestamp_ms", json::num(ts)}
    });
    audit_append(base_dir, receipt);
    return receipt;
  }

  // WRITE_NOTE transactional
  std::string snap_id = make_id(proposal_id + "_snap");
  snapshot_take(base_dir, snap_id);

  std::string note_path = join(workspace_dir(base_dir), "note_" + proposal_id + ".txt");
  std::string note_content = "Nex note\nproposal_id=" + proposal_id + "\nts=" + std::to_string(ts) + "\n";

  bool wrote = write_text_file(note_path, note_content);
  bool verified = wrote && path_exists(note_path);

  bool rolled_back = false;
  if (!verified) {
    snapshot_restore(base_dir, snap_id);
    rolled_back = true;
  }

  auto receipt = json::obj({
    {"event", json::str("EXECUTE_WRITE_NOTE")},
    {"proposal_id", json::str(proposal_id)},
    {"ok", json::boolean(verified)},
    {"note_path", json::str(note_path)},
    {"rolled_back", json::boolean(rolled_back)},
    {"timestamp_ms", json::num(ts)}
  });
  audit_append(base_dir, receipt);
  return receipt;
}

}  // namespace nexcore
