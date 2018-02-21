#include "dsa_common.h"

#include "string.h"

#include <boost/asio/io_service.hpp>
#include <boost/filesystem.hpp>
#include "module/default/simple_storage.h"
#include "module/storage.h"
#include "openssl/rand.h"

using namespace std;
namespace fs = boost::filesystem;

namespace dsa {

string_ string_from_file(const string_ &file_path) {
  SimpleStorage simple_storage;
  std::unique_ptr<StorageBucket> storage_bucket;
  storage_bucket = simple_storage.get_bucket("");

  return string_from_bucket(file_path, *storage_bucket);
}

void string_to_file(const string_ &data, const string_ &file_path) {
  SimpleStorage simple_storage;
  std::unique_ptr<StorageBucket> storage_bucket;
  storage_bucket = simple_storage.get_bucket("");
  string_to_bucket(data, file_path, *storage_bucket);
}

string_ string_from_bucket(const string_ &key, StorageBucket &storage_bucket) {
  string_ data;
  auto read_callback = [&](std::string storage_key, std::vector<uint8_t> vec,
                           BucketReadStatus read_status) {
    string_ content(vec.begin(), vec.end());
    data = content;
  };
  storage_bucket.read(key, read_callback);
  return data;
}
void string_to_bucket(const string_ &data, const string_ &key,
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

string_ get_close_token_from_bucket(StorageBucket &storage_bucket,
                                    const string_ &key,
                                    bool force_to_generate_one) {
  string_ token = string_from_bucket(key, storage_bucket);
  if (token.length() == 32) return token;

  if (force_to_generate_one) {
    token = generate_random_string(32);
    string_to_bucket(token, key, storage_bucket);
  } else
    token = "";
  return token;
}
}