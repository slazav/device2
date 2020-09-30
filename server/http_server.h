#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <microhttpd.h>
#include "dev_manager.h"

/*************************************************/
// Microhttpd-related functions.
// Requests from users are transferred into DevManager.
// Each connection in a separate thread.

class HTTP_Server{
  void *d;

public:
  HTTP_Server(
      const std::string & addr,
      const int port,
      bool test, // test mode with single connection
      DevManager * dm);

  ~HTTP_Server();
};

#endif
