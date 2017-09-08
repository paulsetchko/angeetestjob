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
    const std::string& getUrl();
    const std::string& getHash();

  private:
    std::string url;
    std::string hash;
};

void RemoteFile::setUrl(const std::string& input) {
  url = input;
}

void RemoteFile::setHash(const std::string& input) {
  hash = input;
}

const std::string& RemoteFile::getUrl() {
  return url;
}

const std::string& RemoteFile::getHash() {
  return hash;
}

RemoteFile::RemoteFile() {
}

RemoteFile::RemoteFile(std::string& input): url(input) {
}

RemoteFile::~RemoteFile() {
}

NAMESPACE_END

#endif
