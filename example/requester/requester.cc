#include <dsa/requester.h>

int main() {
  App app;

  client;

  requester = client->session()->requester();


  stream = requester.subscribe("/abc", func);
  stream.close();
}


void callback(ValueUpdate, ResponseStatus)