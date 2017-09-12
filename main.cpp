#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
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

void *curl_getfile(void *args) {
  CURL *curl_handle;
  RemoteFile::thread_args *temp = (RemoteFile::thread_args *)args;
  FILE *desc = fopen(temp->filename.c_str(), "wb");

  if (desc) {
    /* init the curl session */
    curl_handle = curl_easy_init();

    /* tell libcurl to follow redirection */
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);

    /* Switch on full protocol/debug output while testing */
    //curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);

    /* disable progress meter, set to 0L to enable and disable debug output */
    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);

    /* set URL to get here */
    curl_easy_setopt(curl_handle, CURLOPT_URL, temp->url.c_str());

    /* send all data to this function  */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);

    /* write the page body to this file handle */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, desc);

    /* get it! */
    curl_easy_perform(curl_handle);

    /* close the header file */
    fclose(desc);

    /* cleanup curl stuff */
    curl_easy_cleanup(curl_handle);

    delete (RemoteFile::thread_args *)args;
  }

  return NULL;
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
  int error, j = 0;
  std::string line;
  std::ifstream myfile(pagefilename);
  std::string temp;
  std::vector<std::string> urls;
  std::vector<RemoteFile::RemoteFile> remotefiles;
 
  if (argc < 2) {
    printf("Usage: %s <URL>\n", argv[0]);
    return 1;
  }

  curl_global_init(CURL_GLOBAL_ALL);

  RemoteFile::thread_args *core = new RemoteFile::thread_args();
  core->url = argv[1];
  core->filename = pagefilename;

  curl_getfile((void *)core);

  /* fill up the files urls and their adler32 hashes*/
  fill_url_vector(pagefilename, argv[1], remotefiles);

  pthread_t tid[remotefiles.size()];

  for (auto i = remotefiles.begin(); i != remotefiles.end(); ++i, ++j) {
    RemoteFile::thread_args *args = new RemoteFile::thread_args();

    args->url = (*i).getUrl();
    args->filename = std::to_string(j);
    
    error = pthread_create(&tid[j],
                           NULL,
                           curl_getfile,
                           (void *)args);
    if (0 != error) {
      delete args;
    }
  }

  for (uint16_t i = 0; i < remotefiles.size(); ++i) {
    error = pthread_join(tid[i], NULL);
  }

  j = 0;

  for (auto i = remotefiles.begin(); i != remotefiles.end(); ++i, ++j) {
    get_adler32(std::to_string(j), temp);
    (*i).setHash(temp);
    temp.erase();
    std::cout << (*i).getHash() << std::endl;
  }

  return 0;
}

