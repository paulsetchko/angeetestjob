#ifndef CURLOBJ
#define CURLOBJ

#include "thread.hpp"
#include "remote.hpp"

class CurlObj {
  public:
    CurlObj(const std::string&, const std::string&);
    ~CurlObj();
    void downloadPages(std::vector<RemoteFile>&);

  private:
    struct thread_args core;
    int min_index, max_index;
    void fillUrlVector(std::vector<RemoteFile>&);
    void getAdler32(const std::string&, std::string&);
    void getFilesize(RemoteFile&, const std::string&);
    static void *savePage(void *);
    static size_t writeData(void *, size_t, size_t, void *);
    static void corePage(void *);
};

#endif
