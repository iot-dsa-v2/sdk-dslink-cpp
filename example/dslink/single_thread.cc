#include "dslink.h"

using namespace dsa;

int main(int argc, const char *argv[]) {
  App app(1);
  ConfigLoader config(argc, argv);
  Link link(config);
  return 0;
}
