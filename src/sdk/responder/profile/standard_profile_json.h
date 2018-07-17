#ifndef DSA_SDK_STANDARD_PROFILE_JSON_H
#define DSA_SDK_STANDARD_PROFILE_JSON_H

static const std::string standard_profile = R"({
 "2.0": {
  "broker": {
   "user": {
    "remove": {
     "$invokable": "config"
    }
   },
   "client": {
    "remove": {
     "$invokable": "config"
    },
    "detach-token": {
     "$invokable": "config"
    },
    "role": {
     "$type": "string",
     "$writable": "config"
    },
    "path": {
     "$type": "string",
     "$writable": "config"
    },
    "max-session": {
     "$type": "number",
     "$writable": "config"
    },
    "current-session": {
     "$type": "number"
    },
    "from-token": {
     "$type": "string"
    }
   },
   "quarantine-client": {
    "authorize": {
     "$invokable": "config",
     "$params": [
      {
       "name": "Path",
       "type": "string"
      },
      {
       "name": "Role",
       "type": "string"
      },
      {
       "name": "Max Session",
       "type": "number"
      }
     ]
    }
   },
   "permission-role": {
    "$type": "string",
    "$writable": "config",
    "$editor": "enum[,none,list,read,write,config]",
    "remove": {
     "$invokable": "config"
    },
    "add-rule": {
     "$invokable": "config",
     "$params": [
      {
       "name": "Path",
       "type": "string"
      },
      {
       "name": "Permission",
       "type": "string",
       "editor": "enum[none,list,read,write,config]"
      }
     ]
    },
    "fallback": {
     "$type": "string",
     "$writable": "config"
    }
   },
   "permission-rule": {
    "$type": "string",
    "$writable": "config",
    "$editor": "enum[none,list,read,write,config]",
    "remove": {
     "$invokable": "config"
    }
   },
   "token": {
    "remove": {
     "$invokable": "config"
    },
    "remove-all-clients": {
     "$invokable": "config"
    },
    "regenerate": {
     "$invokable": "config"
    },
    "role": {
     "$type": "string",
     "$writable": "config"
    },
    "time-range": {
     "$type": "string",
     "$editor": "daterange",
     "$writable": "config"
    },
    "count": {
     "$type": "number",
     "$writable": "config"
    },
    "max-session": {
     "$type": "number",
     "$writable": "config"
    },
    "managed": {
     "$type": "bool",
     "$writable": "config"
    },
    "token": {
     "$type": "string"
    }
   },
   "upstream-connection": {
    "remove": {
     "$invokable": "config"
    },
    "enabled": {
     "$type": "bool",
     "$writable": "config"
    },
    "url": {
     "$type": "string",
     "$writable": "config"
    },
    "token": {
     "$type": "string",
     "$writable": "config"
    },
    "role": {
     "$type": "string",
     "$writable": "config"
    },
    "status": {
     "$type": "string"
    },
    "remote-id": {
     "$type": "string"
    },
    "remote-path": {
     "$type": "string"
    }
   }
  }
 }
})";

#endif //DSA_SDK_STANDARD_PROFILE_JSON_H
