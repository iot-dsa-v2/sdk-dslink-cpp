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
   "Client": {
    "Remove": {
     "$invokable": "config"
    },
    "Group": {
     "$type": "string",
     "$writable": "config"
    },
    "Path": {
     "$type": "string",
     "$writable": "config"
    },
    "Max_Session": {
     "$type": "number"
    },
    "Current_Session": {
     "$type": "number"
    },
    "Default_Token": {
     "$type": "string",
     "$writable": "config"
    }
   }
  }
 }
})";

#endif //DSA_SDK_STANDARD_PROFILE_JSON_H
