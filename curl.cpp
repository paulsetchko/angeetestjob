#include <pthread.h>
#include <vector>
#include <sys/stat.h>
#include <fstream>
#include <limits>
#include <curl/curl.h>
#include <iostream>
#include <sstream>
#include <crypto++/zlib.h>
#include <crypto++/files.h>
#include <crypto++/hex.h>
#include <crypto++/filters.h>
#include <utility>
#include "curl.hpp"

CurlObj::CurlObj(const std::string& name, const std::string& url):
              core(url, name),
              min_index(0),
              max_index(0) {
  curl_global_init(CURL_GLOBAL_ALL);
}

CurlObj::~CurlObj() {
/*  std::cout << std::endl << "The biggest file is:" << std::endl
            << remotefiles[max_index].getUrl() << std::endl
            << std::endl << "The smallest file is:" << std::endl
            << remotefiles[min_index].getUrl() << std::endl;
*/
  curl_global_cleanup();
}

size_t CurlObj::writeData(void *ptr, size_t size, size_t nmemb, void *stream) {
  size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}

void *CurlObj::savePage(void *args) {
  CURL *curl_handle;
  thread_args *temp = (thread_args *)args;
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
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, CurlObj::writeData);

    /* write the page body to this file handle */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, desc);

    /* get it! */
    curl_easy_perform(curl_handle);

    /* close the header file */
    fclose(desc);

    /* cleanup curl stuff */
    curl_easy_cleanup(curl_handle);

    delete (thread_args *)args;
  }

  return NULL;
}

void CurlObj::fillUrlVector(std::vector<RemoteFile>& files) {
  std::ifstream myfile(core.filename);
  std::vector<std::string> urls;
  std::string temp, line;
  std::string url(core.url);

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
    RemoteFile tempfile(*i);
    files.push_back(tempfile);
  }
}

void CurlObj::getAdler32(const std::string& filename, std::string& sink) {
  CryptoPP::Adler32 hashAdler32;

  CryptoPP::FileSource(filename.c_str(),
            true,
            new CryptoPP::HashFilter(hashAdler32,
            new CryptoPP::HexEncoder(new CryptoPP::StringSink(sink))));
}

void CurlObj::getFilesize(RemoteFile& unit, const std::string& filename) {
  struct stat st;
  stat(filename.c_str(), &st);
  unit.setSize(st.st_size);
}

void CurlObj::downloadPages(std::vector<RemoteFile>& remotefiles) {
  std::string temp;
  int error, j = 0;
  int max = 0, min = std::numeric_limits<int>::max();
  savePage((void *)(&core));
  fillUrlVector(remotefiles);
  
  pthread_t tid[remotefiles.size()];

  for (auto i = remotefiles.begin(); i != remotefiles.end(); ++i, ++j) {
    std::string tempname(std::to_string(j));
    std::string tempurl((*i).getUrl());
    thread_args *args = new thread_args(tempurl, tempname);

    error = pthread_create(&tid[j],
                           NULL,
                           CurlObj::savePage,
                           (void *)args);
    if (0 != error) {
      delete args;
    }
  }

  for (uint16_t i = 0; i < remotefiles.size(); ++i) {
    pthread_join(tid[i], NULL);
    getFilesize(remotefiles[i], std::to_string(i));
  }

  j = 0;

  for (auto i = remotefiles.begin(); i != remotefiles.end(); ++i, ++j) {
    if (min > (*i).getSize()) {
      min_index = j;
      min = (*i).getSize();
    }
    if (max < (*i).getSize()) {
      max_index = j;
      max = (*i).getSize();
    }
  }

  j = 0;

  for (auto i = remotefiles.begin(); i != remotefiles.end(); ++i, ++j) {
    getAdler32(std::to_string(j), temp);
    (*i).setHash(temp);
    temp.erase();
    std::cout << (*i).getUrl() << " " << (*i).getHash() << std::endl;
  }
}

