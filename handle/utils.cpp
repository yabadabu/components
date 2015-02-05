#include <Windows.h>
#include <cassert>
#include <cstdio>

// --------------------------------------------------
void dbg(const char *fmt, ...) {
  //std::lock_guard< std::mutex > lock(dbg_mutex);
  char buf[1024];
  va_list ap;
  va_start(ap, fmt);
  int n = _vsnprintf_s(buf, sizeof(buf) - 1, fmt, ap);
  assert(n < sizeof(buf));
  ::OutputDebugStringA(buf);
  printf(buf);
}

bool fatal(const char *fmt, ...) {
  //std::lock_guard< std::mutex > lock(dbg_mutex);
  char buf[1024];
  va_list ap;
  va_start(ap, fmt);
  int n = _vsnprintf_s(buf, sizeof(buf) - 1, fmt, ap);
  assert(n < sizeof(buf));
  ::OutputDebugStringA(buf);
  printf(buf);
  return false;
}