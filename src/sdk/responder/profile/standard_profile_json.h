#ifndef DSA_SDK_STANDARD_PROFILE_JSON_H
#define DSA_SDK_STANDARD_PROFILE_JSON_H

static const std::string standard_profile = R"({
 "2.0": {
  "Broker": {
   "User": {
    "Remove": {
     "$invokable": "config"
    }
   },
   "Client": {}
  }
 }
})";

#endif //DSA_SDK_STANDARD_PROFILE_JSON_H
