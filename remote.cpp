#include "remote.hpp"

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
