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
    "Role": {
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
       "name": "Role",
       "type": "string"
      },
      {
       "name": "Max_Session",
       "type": "number"
      }
     ]
    }
   },
   "Permission_Role": {
    "$type": "string",
    "$writable": "config",
    "$editor": "enum[,none,list,read,write,config]",
    "Remove": {
     "$invokable": "config"
    },
    "Add_Rule": {
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
    "Fallback": {
     "$type": "string",
     "$writable": "config"
    }
   },
   "Permission_Rule": {
    "$type": "string",
    "$writable": "config",
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
    "Role": {
     "$type": "string",
     "$writable": "config"
    },
    "Time_Range": {
     "$type": "string",
     "$editor": "daterange",
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
   },
   "Upstream_Connection": {
    "Remove": {
     "$invokable": "config"
    },
    "Enabled": {
     "$type": "bool",
     "$writable": "config"
    },
    "Url": {
     "$type": "string",
     "$writable": "config"
    },
    "Token": {
     "$type": "string",
     "$writable": "config"
    },
    "Role": {
     "$type": "string",
     "$writable": "config"
    },
    "Status": {
     "$type": "string"
    },
    "Remote_Id": {
     "$type": "string"
    },
    "Remote_Path": {
     "$type": "string"
    }
   }
  }
 }
})";

#endif //DSA_SDK_STANDARD_PROFILE_JSON_H
