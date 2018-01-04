#include "string.h"
#include <boost/filesystem.hpp>
#include "openssl/rand.h"

using namespace std;
namespace fs = boost::filesystem;

namespace dsa{

string_ string_from_file(string_ file_path){
  string_ data;
  if (fs::is_regular_file(file_path)) {
    std::ifstream my_file(file_path, std::ios::in | std::ios::binary);
    if (my_file.is_open()) {
      my_file >> data;
    } else {
      throw std::runtime_error("cannot open file to read");
    }
  }
  else{
    throw std::runtime_error("there is no file");
  }

  return data;
}

void string_to_file(string_ data, string_ file_path){
  std::ofstream my_file(file_path,
                        std::ios::out | std::ios::binary | std::ios::trunc);
  if (my_file.is_open()) {
    my_file<<data;
  } else {
    throw std::runtime_error("Unable to open file to write");
  }
}

std::vector<unsigned char> get_random_byte_array(int len) {
  if(!IS_RAND_INITIALIZED) {
    RAND_poll();
    IS_RAND_INITIALIZED = 1;
  }
  std::vector<unsigned char> buffer(len);
  RAND_bytes(buffer.data(), len);
  return buffer;
}
static
unsigned char get_random_char() {
  while(1) {
    unsigned char n = (unsigned char)(get_random_byte_array(1)[0] & 0x7F);
    if ((n >= '0' && n <= '9') ||
        (n >= 'A' && n <= 'Z') ||
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

string_ get_close_token_from_file(string_ path_str, bool force_to_generate_one){
  try {
    string_ token = string_from_file(path_str);
    if(token.length() != 32) throw std::runtime_error("invalid token length != 32 in file");
    return token;

  } catch (std::exception &e) {
    if(!force_to_generate_one) return "";
  }

  auto new_token = generate_random_string(32);
  string_to_file(new_token, path_str);

  return new_token;
}


}