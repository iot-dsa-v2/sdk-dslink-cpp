#include "dslink.h"

using namespace dsa;

int main(int argc, const char *argv[]) {

  ConfigLoader config(argc, argv, "mydslink", "1.0.0");
  Link link(config);
  return 0;
}
