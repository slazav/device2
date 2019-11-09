#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <microhttpd.h>
#include "device_manager.h"

/*************************************************/
// Microhttpd-related functions.
// Requests from users are transferred into DeviceManager.
// Each connection in a separate thread.

class HTTP_Server{
  void *d;
public:
  HTTP_Server(
      const int port,
      DeviceManager & dm);

  ~HTTP_Server();
};

#endif
