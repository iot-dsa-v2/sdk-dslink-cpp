#include "dsa_common.h"

#include "console_logger.h"

#include <iostream>

namespace dsa {

void ConsoleLogger::log(const std::string& str) { std::cout << str; }
}
