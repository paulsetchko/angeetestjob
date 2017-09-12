#ifndef REMOTEFILE
#define REMOTEFILE

#include <string>

NAMESPACE_BEGIN(RemoteFile)

class RemoteFile {
  public:
    RemoteFile();
    RemoteFile(std::string& input);
    ~RemoteFile();

    void setUrl(const std::string& input);
    void setHash(const std::string& input);
    void setSize(const int& input);
    const std::string& getUrl();
    const std::string& getHash();
    const int& getSize();

  private:
    std::string url;
    std::string hash;
    int size;
};

void RemoteFile::setUrl(const std::string& input) {
  url = input;
}

void RemoteFile::setHash(const std::string& input) {
  hash = input;
}

void RemoteFile::setSize(const int& input) {
  size = input;
}


const std::string& RemoteFile::getUrl() {
  return url;
}

const std::string& RemoteFile::getHash() {
  return hash;
}

const int& RemoteFile::getSize() {
  return size;
}

RemoteFile::RemoteFile() {
}

RemoteFile::RemoteFile(std::string& input): url(input) {
}

RemoteFile::~RemoteFile() {
}

struct thread_args {
  std::string url;
  std::string filename;
};

NAMESPACE_END

#endif
