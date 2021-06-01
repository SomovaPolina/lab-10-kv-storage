//
// Created by polinka on 20.04.2021.
//
#ifndef INCLUDE_HTMLDOWNLOADER_HPP_
#define INCLUDE_HTMLDOWNLOADER_HPP_
#include <boost/log/common.hpp>
#include <boost/log/core.hpp>
#include <boost/log/exceptions.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <map>

#include "ThreadPool.h"
#include "iostream"
#include "map"
#include "mutex"
#include "picosha2.h"
#include "utility"
class rocksMapHasher {
 private:
  std::map<std::string, std::map<std::string, std::string>> hashedMap_;
  ThreadPool familyPool_;
  std::mutex mutex;

 public:
  std::map<std::string, std::map<std::string, std::string>> getHashedMap() {
    return hashedMap_;
  }
  void startHashing(std::string fimilyName,
                    std::map<std::string, std::string> kvStorage,
                    std::string logLevel);
  explicit rocksMapHasher(int threadNum) : familyPool_(threadNum) {}
  void hashStorage(std::string familyName,
                   std::map<std::string, std::string> kvStore,
                   std::string logLevel);
};
#endif  // INCLUDE_HTMLDOWNLOADER_HPP_