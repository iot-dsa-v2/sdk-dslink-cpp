#include "dsa/util.h"
#include "gtest/gtest.h"

#include <boost/filesystem.hpp>

TEST(SSLTest, GenerateCertificate) {

  dsa::generate_certificate();

  std::vector<std::string> pem_files = { "key.pem",
					 "certificate.pem"
                                       };
  namespace fs = boost::filesystem;
  for (auto pem_file : pem_files) {
    if (fs::exists(pem_file) && fs::is_regular_file(pem_file)) {
        EXPECT_GT(fs::file_size(pem_file), 0);
    } else {
      EXPECT_TRUE(false);
    }
  }
}
