#include <boost/log/common.hpp>
#include <boost/log/core.hpp>
#include <boost/log/exceptions.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/program_options.hpp>
#include <DBwrapper.hpp>

#include "hashData.hpp"
#include "iostream"
namespace po = boost::program_options;

int main(int argc, char* argv[]) {
  po::options_description desc(
      "Usage:\n  dbcs [options] <path/to/input/storage.db>\nMust have options");
  desc.add_options()
  ("help", "produce help message")
  ("log-level", po::value<std::string>(), "info|warning|error \ndefault: error")
  ("thread-count", po::value<int>(), "default: count of logical core")
  ("output",po::value<std::string>(),"<path/to/output/storage.db>\ndefault:"
  " <path/to/input/dbcs-storage.db>");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }

  std::string outPath;
  std::string logLevel;

  int threadNum;
  if (vm["log-level"].empty()) {
    logLevel = "warning";
  } else {
    logLevel = vm["log-level"].as<std::string>();
  }
  if (vm["output"].empty()) {
    outPath = "/tmp/outPut";
  } else {
    outPath = vm["output"].as<std::string>();
  }
  if (vm["thread-count"].empty()) {
    threadNum = std::thread::hardware_concurrency();
  } else {
    threadNum = vm["thread-count"].as<int>();
  };
  srand((int)time(nullptr));

  rocksMapHasher hasher = rocksMapHasher(threadNum);
  rocksdbWrapper db = rocksdbWrapper(10, 10, "/tmp/database", hasher);
  db.createDatabase();
  db.getFamiliesFromBD();
  db.pushData();
  std::map<std::string, std::map<std::string, std::string>> mapa;
  std::vector<std::string> fams = db.getFamilyNum();

  db.migrateDataToMap(logLevel);

  rocksdbWrapper outputDB =
      rocksdbWrapper(hasher.getHashedMap(), outPath, hasher);
  outputDB.createOutputDatabase();
}