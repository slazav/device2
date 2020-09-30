#ifndef DRV_UTILS_H
#define DRV_UTILS_H

#include <string>

// Trim substring `trim` from the end of `str` (if it is there).
// Return true if the rimming is done.
bool trim_str(std::string & str, const std::string & trim);


// Check if the message contains no question marks
bool no_question(const std::string & msg);

#endif


