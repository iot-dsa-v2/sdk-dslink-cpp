
#ifndef PROJECT_CLOSE_TOKEN_H
#define PROJECT_CLOSE_TOKEN_H



namespace dsa{

  string_ get_close_token_from_file(string_ path_str=".close_token", bool force_to_generate_one = false){
    try {
      string_ token = string_from_file(path_str);
      if(token.length() != 32) throw std::runtime_error("invalid token length != 32 in file");
      return token;

    } catch (std::exception &e) {
      if(!force_to_generate_one) return "";
      LOG_WARN(Logger::_(),
               LOG << "error loading existing close token " << path_str
                   << " message: " << e.what() << ", generating new key");
    }

    auto new_token = generate_random_string(32);
    string_to_file(new_token, path_str);

    return new_token;
  }

}

#endif //PROJECT_CLOSE_TOKEN_H
