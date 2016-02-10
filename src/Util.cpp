#include "Util.h"
#include <algorithm> // count

bool string_contains(const std::string & a, const std::string & b) {
  return (a.find(b) != std::string::npos);
}

bool string_beginsWith(const std::string & a, const std::string & b) {
  return (a.find(b) == 0);
}

bool string_endsWith(const std::string & a, const std::string & b) {
  if (b.size() > a.size()) return false;
  return std::equal(b.rbegin(), b.rend(), a.rbegin());
}

bool core_subset(const std::vector<int> & a, const std::vector<int> & b) {
  for (int t : a) {
    if (!std::count(b.begin(), b.end(), t)) return false;
  }
  return true;
}

std::string trim(const std::string & s) {
  size_t start = s.find_first_not_of(" \t\n");
  size_t end = s.find_last_not_of(" \t\n");
  return s.substr(start, end + 1);
}

void log(const int level, const char * fmt, ...) {
  if (level > 0) return;
  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
}

void condLog(const bool cond, const int level, const char * fmt, ...) {
  if (level > 0) return;
  if (!cond) return;
  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
}

void terminate(const int code, const char * fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
  exit(code);
}

void logVec(const int level, std::vector<int> & core) {
  if (level > 0) return;
  printf("c");
  for (int i : core) printf(" %d", i);
  printf("\n");
}

void condTerminate(const bool cond, const int code, const char * fmt, ...) {
  if (!cond) return;
  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
  exit(code);
}