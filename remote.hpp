#ifndef REMOTEFILE
#define REMOTEFILE

#include <string>

class RemoteFile {
  public:
//    RemoteFile();
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

#endif
