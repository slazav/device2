#include "log.h"

std::ostream *Log::log = &std::cout;
std::ofstream Log::flog;
int Log::log_level = 0;
