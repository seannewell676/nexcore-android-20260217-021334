#include <jni.h>
#include <string>
#include "nexcore/core.h"

static std::string to_str(JNIEnv* env, jstring s) {
  if (!s) return "";
  const char* c = env->GetStringUTFChars(s, nullptr);
  std::string out = c ? c : "";
  if (c) env->ReleaseStringUTFChars(s, c);
  return out;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_nex_core_NexCoreBridge_propose(JNIEnv* env, jobject, jstring userText, jstring baseDir) {
  auto res = nexcore::propose(to_str(env, userText), to_str(env, baseDir));
  return env->NewStringUTF(res.json.c_str());
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_nex_core_NexCoreBridge_execute(JNIEnv* env, jobject, jstring proposalId, jboolean approved, jstring baseDir) {
  auto out = nexcore::execute(to_str(env, proposalId), approved == JNI_TRUE, to_str(env, baseDir));
  return env->NewStringUTF(out.c_str());
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_nex_core_NexCoreBridge_auditTail(JNIEnv* env, jobject, jint maxEntries, jstring baseDir) {
  auto out = nexcore::audit_tail((int)maxEntries, to_str(env, baseDir));
  return env->NewStringUTF(out.c_str());
}
