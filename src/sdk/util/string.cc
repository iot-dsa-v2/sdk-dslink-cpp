#include "dsa_common.h"

#include "string.h"

#include <boost/asio/io_service.hpp>
#include <boost/filesystem.hpp>
#include "module/default/simple_storage.h"
#include "openssl/rand.h"

using namespace std;
namespace fs = boost::filesystem;

namespace dsa {

std::string url_encode(const std::string & s_src)
{
  if(s_src.empty())
    return "";
  const unsigned char dec_to_hex[16 + 1] = "0123456789ABCDEF";
  const unsigned char * p_src = (const unsigned char *)s_src.c_str();
  const auto src_len = s_src.length();
  unsigned char * const p_start = new unsigned char[src_len * 3];
  unsigned char * p_end = p_start;
  const unsigned char * const src_end = p_src + src_len;

  for (; p_src < src_end; ++p_src)
  {
    switch (*p_src) {
      case '%':
      case '=':
      case '&':
      case '\n':
        // escape this char
        *p_end++ = '%';
        *p_end++ = dec_to_hex[*p_src >> 4];
        *p_end++ = dec_to_hex[*p_src & 0x0F];
        break;
      default:
        *p_end++ = *p_src;
        break;
    }
  }

  std::string sResult((char *)p_start, (char *)p_end);
  delete [] p_start;
  return sResult;
}

string_ string_from_file(string_ file_path) {
  SimpleStorage simple_storage;
  std::unique_ptr<StorageBucket> storage_bucket;
  string_ data;
  storage_bucket = simple_storage.get_bucket("");
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
  storage_bucket = simple_storage.get_bucket("");
  auto content =
      new RefCountBytes(&data.c_str()[0], &data.c_str()[strlen(data.c_str())]);

  storage_bucket->write(file_path, std::forward<RefCountBytes *>(content));
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
  string_ token = string_from_file(path_str);
  if (token.length() == 32)
    return token;

  if (force_to_generate_one) {
    token = generate_random_string(32);
    string_to_file(token, path_str);
  } else
    token = "";
  return token;
}
}