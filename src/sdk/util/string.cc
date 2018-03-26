#include "dsa_common.h"

#include "module/logger.h"
#include "string.h"

#include <boost/asio/io_service.hpp>
#include <boost/filesystem.hpp>
#include "module/default/simple_storage.h"
#include "module/storage.h"
#include "openssl/rand.h"
#include "util/string_encode.h"
using namespace std;
namespace fs = boost::filesystem;

namespace dsa {

string_ string_from_file(const string_ &file_path) {
  std::vector<uint8_t> vec{};

  if (fs::exists(file_path) && fs::is_regular_file(file_path)) {
    size_t size = fs::file_size(file_path);

    if (size) {
      fs::ifstream ifs(file_path, std::ios::in | std::ios::binary);
      if (ifs) {
        vec.resize(static_cast<size_t>(size));
        ifs.read(reinterpret_cast<char *>(&vec.front()),
                 static_cast<size_t>(size));
        ifs.close();
      } else {
        LOG_ERROR(__FILENAME__,
                  LOG << "Unable to open " << file_path << " file to read");
      }
    }
  } else {
    LOG_FINE(__FILENAME__, LOG << "there is no file to read " << file_path);
  }

  string_ content(vec.begin(), vec.end());

  return content;
}

void string_to_file(const string_ &data, const string_ &file_path) {
  try {
    fs::ofstream ofs(file_path,
                     std::ios::out | std::ios::trunc | std::ios::binary);
    if (ofs) {
      ofs << data;
      ofs.close();
    } else {
      LOG_ERROR(__FILENAME__,
                LOG << "Unable to open " << file_path << " file to write");
    }
  } catch (const fs::filesystem_error &ex) {
    LOG_ERROR(__FILENAME__, LOG << "Write failed for " << file_path << " file");
  }
}

string_ string_from_storage(const string_ &key, StorageBucket &storage_bucket) {
  string_ data;
  bool callback_called = false;
  auto read_callback = [&](string_ storage_key, std::vector<uint8_t> vec,
                           BucketReadStatus read_status) {
    string_ content(vec.begin(), vec.end());
    data = content;
    callback_called = true;
  };
  storage_bucket.read(key, read_callback);
  if (!callback_called) {
    LOG_FATAL(__FILENAME__,
              LOG << "Storage does not support synchronize reading");
  }
  return data;
}
void string_to_storage(const string_ &data, const string_ &key,
                       StorageBucket &storage_bucket) {
  auto content =
      new RefCountBytes(&data.c_str()[0], &data.c_str()[strlen(data.c_str())]);

  storage_bucket.write(key, std::forward<RefCountBytes *>(content));
}

std::vector<unsigned char> get_random_byte_array(int len) {
  if (!IS_RAND_INITIALIZED) {
    RAND_poll();
    IS_RAND_INITIALIZED = 1;
  }
  std::vector<unsigned char> buffer(len);
  RAND_bytes(buffer.data(), len);
  return buffer;
}
static unsigned char get_random_char() {
  while (1) {
    unsigned char n = (unsigned char)(get_random_byte_array(1)[0] & 0x7F);
    if ((n >= '0' && n <= '9') || (n >= 'A' && n <= 'Z') ||
        (n >= 'a' && n <= 'z')) {
      return n;
    }
  }
}
string_ generate_random_string(int len) {
  string_ randStr;

  for (int i = 0; i < len; ++i) {
    randStr += get_random_char();
  }

  return randStr;
}

string_ get_master_token_from_storage(StorageBucket &storage_bucket,
                                      const string_ &key,
                                      bool force_to_generate_one) {
  string_ token = string_from_storage(key, storage_bucket);
  if (token.length() == 32) return token;

  if (force_to_generate_one) {
    token = generate_random_string(32);
    string_to_storage(token, key, storage_bucket);
  } else
    token = "";
  return token;
}
}