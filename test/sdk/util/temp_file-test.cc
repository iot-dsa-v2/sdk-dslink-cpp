#include "dsa/util.h"
#include <gtest/gtest.h>
#include "util/temp_file.h"

using namespace dsa;

TEST(TempFileTest, CreateTempFile) {

  auto tmp = std::make_shared<TempFile>();
  tmp->init("temp123");
  // Get temporary file
  boost::filesystem::path temp_path = tmp->get();
  // Check if file is created and file path is correct
  EXPECT_NE(temp_path.string().find("temp123"), std::string::npos);
  // Get second temporary file
  boost::filesystem::path temp_path1 = tmp->get();
  // Check if two seperate tempfiles where created
  EXPECT_EQ(std::to_string(temp_path.string().back()), "48"); // ASCII(48) -> '0'
  EXPECT_EQ(std::to_string(temp_path1.string().back()), "49"); // ASCII(48) -> '1'

}

