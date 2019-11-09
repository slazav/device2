#include <iostream>
#include <fstream>
#include <string>

//#include <cstdlib>
//#include <cstring>

#include "err/err.h"
#include "http_server.h"

// callback for microhttpd
int ProcessRequest(void * cls,
      struct MHD_Connection * connection,
      const char * url,
      const char * method,
                    const char * version,
      const char * upload_data,
      size_t * upload_data_size,
                    void ** ptr) {

  static int dummy;
  DevManager * dm = (DevManager*)cls;
  struct MHD_Response * response;
  int ret;

  if (0 != strcmp(method, "GET"))
    return MHD_NO; /* unexpected method */
  if (&dummy != *ptr)
    {
      /* The first time only the headers are valid,
         do not respond in the first round... */
      *ptr = &dummy;
      return MHD_YES;
    }
  if (0 != *upload_data_size)
    return MHD_NO; /* upload data in a GET!? */
  *ptr = NULL; /* clear context pointer */

  std::string msg = dm->run(url);

  response = MHD_create_response_from_buffer(
        msg.length(), (void*)msg.data(), MHD_RESPMEM_MUST_COPY);

  ret = MHD_queue_response(connection,
      MHD_HTTP_OK,
      response);
  MHD_destroy_response(response);
  return ret;
}


HTTP_Server::HTTP_Server(
      const int port,
      DevManager & dm){

  d = MHD_start_daemon(
        MHD_USE_THREAD_PER_CONNECTION,
        port, NULL, NULL, &ProcessRequest, &dm,
        MHD_OPTION_END);

  if (d == NULL)
    throw Err() << "Can't start http server";
}

HTTP_Server::~HTTP_Server(){
  MHD_stop_daemon((MHD_Daemon*)d);
}


