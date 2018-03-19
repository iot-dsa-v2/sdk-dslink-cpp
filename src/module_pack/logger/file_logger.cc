#include "dsa_common.h"

#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>

#include "module/module.h"
#include "module/logger.h"
#include "util/date_time.h"

#include "module/storage.h"
#include "module/authorizer.h"
#include "module/client_manager.h"

using namespace std;

namespace bf = boost::filesystem;

namespace dsa {
class FileLogger : public Logger {
 private:
  bf::path _log_path;
  ofstream log_file;

 public:
  uint8_t filter = 0xFF;

  FileLogger(bf::path log_path){
    _log_path = log_path;
    log_file.open(_log_path.c_str());
  }

  ~FileLogger(){
    log_file.close();
  }

  void write_meta(std::ostream& stream, const char* level) override{
    stream << '[' << level << DateTime::get_ts() << "]  ";
  };


  void log(const string_& str, uint8_t lvl) override{
    if ((lvl & filter) || level == Logger::ALL___) {
      log_file << str;
      log_file.flush();
    }
  };
};


class FileLoggerModule: public Module{
 protected:
  shared_ptr_<Logger> create_logger(App& app, const LinkStrandRef &strand) override{
    return make_shared_<FileLogger>("file_logger_logs.txt");
  }
};

#include <boost/config.hpp>
extern "C" BOOST_SYMBOL_EXPORT FileLoggerModule module;
FileLoggerModule module;

}