#include "dsa_common.h"

#include "string.h"

#include <boost/filesystem.hpp>
#include "openssl/rand.h"
#include "module/default/simple_storage.h"
using namespace std;
namespace fs = boost::filesystem;

namespace dsa {

string_ string_from_file(string_ file_path) {
  SimpleStorage simple_storage;
  std::unique_ptr<StorageBucket> storage_bucket;
  string_ data;
  storage_bucket = simple_storage.get_bucket(file_path);
  auto read_callback = [&](std::string storage_key, std::vector<uint8_t> vec) {
    string_ content(vec.begin(), vec.end());
    data = content;
  };
  storage_bucket->read(file_path, read_callback);

  return data;
}

void string_to_file(string_ data, string_ file_path) {
  SimpleStorage simple_storage;
  std::unique_ptr<StorageBucket> storage_bucket;
  storage_bucket = simple_storage.get_bucket(file_path);
  auto content = new RefCountBytes(&data.c_str()[0], &data.c_str()[strlen(data.c_str())]);

  storage_bucket->write(file_path, std::forward<RefCountBytes*>(content));
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

string_ get_close_token_from_file(string_ path_str,
                                  bool force_to_generate_one) {
  try {
    string_ token = string_from_file(path_str);
    if (token.length() != 32)
      throw std::runtime_error("invalid token length != 32 in file");
    return token;

  } catch (std::exception &e) {
    if (!force_to_generate_one) return "";
  }

  auto new_token = generate_random_string(32);
  string_to_file(new_token, path_str);

  return new_token;
}
}