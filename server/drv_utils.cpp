#include "drv_utils.h"
#include "err/err.h"

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

read_cond_t
str_to_read_cond(const std::string & str){
  if (str == "always")  return READCOND_ALWAYS;
  if (str == "never")   return READCOND_NEVER;
  if (str == "qmark")   return READCOND_QMARK;
  if (str == "qmark1w") return READCOND_QMARK1W;
  throw Err() << "unknown -read_cond value: " << str;
}

bool
check_read_cond(const std::string & msg, const int cond){
  switch (cond){
    case READCOND_ALWAYS: return true;
    case READCOND_NEVER:  return false;
    case READCOND_QMARK:
      return msg.find('?') != std::string::npos;
    case READCOND_QMARK1W:
      return msg.find('?') < msg.find(' ');
  }
  throw Err() << "bad read_cond: " << cond;
}
