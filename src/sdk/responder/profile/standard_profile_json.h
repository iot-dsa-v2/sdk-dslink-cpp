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
    "Detach_Token": {
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
     "$type": "number",
     "$writable": "config"
    },
    "Current_Session": {
     "$type": "number"
    },
    "From_Token": {
     "$type": "string"
    }
   },
   "Quarantine_Client": {
    "Authorize": {
     "$invokable": "config",
     "$params": [
      {
       "name": "Path",
       "type": "string"
      },
      {
       "name": "Group",
       "type": "string"
      },
      {
       "name": "Max_Session",
       "type": "number"
      }
     ]
    }
   },
   "Permission_Group": {
    "Add_Rule": {
     "$invokable": "config",
     "$params": [
      {
       "name": "Path",
       "type": "string"
      },
      {
       "name": "Permission",
       "type": "string"
      }
     ]
    },
    "Fallback": {
     "$type": "number",
     "$writable": "config"
    }
   },
   "Permission_Rule": {
    "$type": "string",
    "$editor": "enum[none,list,read,write,config]",
    "Remove": {
     "$invokable": "config"
    }
   },
   "Token": {
    "Remove": {
     "$invokable": "config"
    },
    "Remove_All_Clients": {
     "$invokable": "config"
    },
    "Regenerate": {
     "$invokable": "config"
    },
    "Time_Range": {
     "$type": "string",
     "$editor": "datarange",
     "$writable": "config"
    },
    "Count": {
     "$type": "number",
     "$writable": "config"
    },
    "Max_Session": {
     "$type": "number",
     "$writable": "config"
    },
    "Managed": {
     "$type": "bool",
     "$writable": "config"
    },
    "Token": {
     "$type": "string"
    }
   }
  }
 }
})";

#endif //DSA_SDK_STANDARD_PROFILE_JSON_H
