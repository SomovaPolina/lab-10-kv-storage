//
// Created by polinka on 20.04.2021.
//
#include "hashData.hpp"

#include <boost/log/common.hpp>
#include <boost/log/core.hpp>
#include <boost/log/exceptions.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>

#include "iostream"
#include "picosha2.h"

boost::log ::trivial::severity_level whatIsLevel(std::string logLevel) {
  if (logLevel.empty()) {
    return boost::log::trivial::severity_level::error;
  } else if (logLevel == "warning") {
    return boost::log::trivial::severity_level::warning;
  } else if (logLevel == "info") {
    return boost::log::trivial::severity_level::info;
  } else
    throw "invalid log level";
}
void rocksMapHasher::hashStorage(std::string familyName,
                                 std::map<std::string, std::string> kvStorage,
                                 std::string logLevel) {
  std::map<std::string, std::string> hashed;
  for (auto const& kv : kvStorage) {
    std::string hash_hex_str;
    picosha2::hash256_hex_string(kv.first + kv.second, hash_hex_str);
    hashed[kv.first] = hash_hex_str;

    boost::log::core::get()->set_filter(boost::log::trivial::severity >=
                                        whatIsLevel(logLevel));

    BOOST_LOG_TRIVIAL(info) << "Family " << familyName << "->"

                            << kv.first << "  hashed";
  }
  mutex.lock();
  hashedMap_[familyName] = hashed;
  hashed.clear();
  mutex.unlock();
}

void rocksMapHasher::startHashing(std::string familyName,
                                  std::map<std::string, std::string> kvStorage,
                                  std::string logLevel) {
  familyPool_.enqueue([this, familyName, kvStorage, logLevel]() {
    this->hashStorage(familyName, kvStorage, logLevel);
  });
}