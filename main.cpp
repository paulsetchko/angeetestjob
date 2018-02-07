#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include "remote.hpp"
#include "curl.hpp"
#include "thread.hpp"

int main(int argc, char *argv[]) {
  const std::string pagefilename("page.out");
  std::string temp;
  std::vector<RemoteFile> remotefiles;
 
  if (argc < 2) {
    printf("Usage: %s <URL>\n", argv[0]);
    return 1;
  }

  temp = argv[1];

  CurlObj *testsubject = new CurlObj(pagefilename, temp);

  testsubject->downloadPages(remotefiles);

  for (auto i = remotefiles.begin(); i != remotefiles.end(); ++i) {
    std::cout << (*i).getUrl() << " " << (*i).getHash() << std::endl;
  }

  delete testsubject;

  return 0;
}

