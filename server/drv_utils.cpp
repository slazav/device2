#include "drv_utils.h"

bool
trim_str(std::string & str, const std::string & trim){
  if (trim.size()>0 &&
      str.size() >= trim.size() &&
      str.substr(str.size()-trim.size()) == trim){
      str.resize(str.size()-trim.size());
      return true;
  }
  return false;
}


bool
no_question(const std::string & msg){
  return msg.find('?') == std::string::npos;
}
