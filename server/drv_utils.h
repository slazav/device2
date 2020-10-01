#ifndef DRV_UTILS_H
#define DRV_UTILS_H

#include <string>

// Trim substring `trim` from the end of `str` (if it is there).
// Return true if the rimming is done.
bool trim_str(std::string & str, const std::string & trim);


// when do we need to read answer
enum read_cond_t{
  READCOND_ALWAYS,  // always
  READCOND_NEVER,   // never
  READCOND_QMARK,   // if there is a question mark in the message
  READCOND_QMARK1W, // if there is a question mark in the first word
};

// Convert string to read_cond_t: "always" -> READCOND_ALWAYS
read_cond_t str_to_read_cond(const std::string & str);

// Check if the message contains no question marks
bool check_read_cond(const std::string & msg, const int cond);

#endif


