#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <curl/curl.h>
#include <sstream>
#include <vector>
#include <algorithm>
#include <string>
#include <crypto++/zlib.h>
#include <crypto++/files.h>
#include <crypto++/hex.h>
#include <crypto++/filters.h>
#include "remote.hpp"

CURL *curl_handle;

void get_adler32(const std::string filename, std::string& sink) {
  CryptoPP::Adler32 hashAdler32;

  CryptoPP::FileSource(filename.c_str(),
            true,
            new CryptoPP::HashFilter(hashAdler32,
            new CryptoPP::HexEncoder(new CryptoPP::StringSink(sink))));
}

size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
  size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}

void curl_setup() {
  curl_global_init(CURL_GLOBAL_ALL);

  /* init the curl session */
  curl_handle = curl_easy_init();

  /* tell libcurl to follow redirection */
  curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);

  /* Switch on full protocol/debug output while testing */
  //curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);

  /* disable progress meter, set to 0L to enable and disable debug output */
  curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
}

int curl_getfile(const char *filename) {
  int ret = 1;
  FILE *desc = fopen(filename, "wb");
  if (desc) {
    /* write the page body to this file handle */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, desc);

    /* get it! */
    curl_easy_perform(curl_handle);

    /* close the header file */
    fclose(desc);

    ret = 0;
  }

  if (ret) {
    std::cout << "error with getting a file" << std::endl;
  }

  return ret;
}

void fill_url_vector(const char *filename,
                     const char *addr,
                     std::vector<RemoteFile::RemoteFile>& files) {
  std::ifstream myfile(filename);
  std::vector<std::string> urls;
  std::string temp, line;
  std::string url(addr);

  /* reading the file line by line */
  while (std::getline(myfile, line)) {
    int start = 0, end = 0;
    if (std::string::npos != line.find("src=\"")) {
      start = line.find("src=\"") + 5;
      end = line.find("\"", start);
    } else if (std::string::npos != line.find("src = \"")) {
      start = line.find("src = \"") + 7;
      end = line.find("\"", start);
    } else if (std::string::npos != line.find("src='")) {
      start = line.find("src='") + 5;
      end = line.find("'", start);
    } else if (std::string::npos != line.find("href=\"") &&
               std::string::npos != line.find("<link")) {
      start = line.find("href=\"") + 6;
      end = line.find("\"", start);
    } else if (std::string::npos != line.find("content=\"") &&
               (std::string::npos != line.find(".jpg") ||
                std::string::npos != line.find(".png"))) {
      start = line.find("content=\"") + 9;
      end = line.find("\"", start);
    } else if (std::string::npos != line.find("'script','//")) {
      start = line.find("'script','//") + 12;
      end = line.find("'", start);
    }

    if (start) {
      temp = line.substr(start, end - start);
      if ('/' == temp[1]) {
        temp.erase(0, 2);
      } else if ('/' == temp[0]) {
        if ('/' != url.back()) {
          temp = url + temp;
        } else {
          temp = url + temp.erase(0, 1);
        }
      }

      urls.push_back(temp);
    }
  }

  /* remove the dublicate files */
  std::sort(urls.begin(), urls.end());
  urls.erase(std::unique(urls.begin(), urls.end()), urls.end());

  for (auto i = urls.begin(); i != urls.end(); ++i) {
    RemoteFile::RemoteFile tempfile(*i);
    files.push_back(tempfile);
  }
}
 
int main(int argc, char *argv[]) {
  const char *pagefilename = "page.out";
  int j = 0;
  std::string line;
  std::ifstream myfile(pagefilename);
  std::string temp;
  std::vector<std::string> urls;
  std::vector<RemoteFile::RemoteFile> remotefiles;
 
  if (argc < 2) {
    printf("Usage: %s <URL>\n", argv[0]);
    return 1;
  }

  curl_setup();
 
  /* set URL to get here */ 
  curl_easy_setopt(curl_handle, CURLOPT_URL, argv[1]);

  /* send all data to this function  */ 
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);

  if (curl_getfile(pagefilename)) {
    curl_easy_cleanup(curl_handle);
    return 1;
  }

  /* fill up the files urls and their adler32 hashes*/
  fill_url_vector(pagefilename, argv[1], remotefiles);

  for (auto i = remotefiles.begin(); i != remotefiles.end(); ++i, ++j) {
    curl_easy_setopt(curl_handle, CURLOPT_URL, (*i).getUrl().c_str());
    curl_getfile(std::to_string(j).c_str());
    get_adler32(std::to_string(j), temp);
    (*i).setHash(temp);
    temp.erase();
    std::cout << (*i).getHash() << std::endl;
  }

  /* cleanup curl stuff */
  curl_easy_cleanup(curl_handle);
 
  return 0;
}

