#ifndef THREAD
#define THREAD

#include <string>

struct thread_args {
  thread_args(const std::string& url_arg, const std::string& file_arg):
              url(url_arg),
              filename(file_arg) {}
  std::string url;
  std::string filename;
};

#endif
