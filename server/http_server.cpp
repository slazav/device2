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


  //get connection number
  auto info = MHD_get_connection_info(
    connection, MHD_CONNECTION_INFO_SOCKET_CONTEXT);
  uint64_t cnum = *(uint64_t*)info->socket_context;

  static int dummy;
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

  DevManager * dm = (DevManager*)cls;
  struct MHD_Response * response;
  int ret;
  try {
    std::string msg = dm->run(url, cnum);
    response = MHD_create_response_from_buffer(
        msg.length(), (void*)msg.data(), MHD_RESPMEM_MUST_COPY);
    ret = MHD_queue_response(connection, 200, response);
    MHD_destroy_response(response);
  }
  catch (Err e) {
    response = MHD_create_response_from_buffer(
        e.str().length(), (void*)e.str().data(), MHD_RESPMEM_MUST_COPY);
    MHD_add_response_header(response, "Error", e.str().c_str());
    ret = MHD_queue_response(connection, 400, response);
    MHD_destroy_response(response);
  }

  return ret;
}

// global connection counter
uint64_t cmax = 0;

// Callback for opening/closing a connection
void ConnFunc (void *cls,
               struct MHD_Connection *connection,
               void **socket_context,
               enum MHD_ConnectionNotificationCode toe){

  DevManager * dm = (DevManager*)cls;

  dm->lock();
  switch (toe){
  case MHD_CONNECTION_NOTIFY_STARTED:
    MHD_set_connection_option(connection, MHD_CONNECTION_OPTION_TIMEOUT, 10);
    *socket_context = new uint64_t;
    *(uint64_t*)*socket_context = cmax++; // set connection number
    dm->conn_open(*(uint64_t*)*socket_context);
    break;
  case MHD_CONNECTION_NOTIFY_CLOSED:
    dm->conn_close(*(uint64_t*)*socket_context);
    delete (uint64_t*)*socket_context;
    break;
  }
  dm->unlock();
}

HTTP_Server::HTTP_Server(
      const int port,
      DevManager * dm) {

  d = MHD_start_daemon(
        MHD_USE_THREAD_PER_CONNECTION,
        port, NULL, NULL, &ProcessRequest, dm,
        MHD_OPTION_NOTIFY_CONNECTION, &ConnFunc, dm,
        MHD_OPTION_END);

  if (d == NULL)
    throw Err() << "Can't start http server";
}

HTTP_Server::~HTTP_Server(){
  MHD_stop_daemon((MHD_Daemon*)d);
}


