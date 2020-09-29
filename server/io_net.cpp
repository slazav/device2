#include "io_net.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

IONet::IONet(const Opt & opts) {
// based on the example in
// https://beej.us/guide/bgnet/html//index.html#a-simple-stream-client

  int res;

  //prefix for error messages
  errpref = opts.get("errpref", "IONet: ");

  // address and port (mandatory settings)
  std::string addr = opts.get("addr", "");
  if (addr == "") throw Err() << errpref
    << "Parameter -addr is empty or missing";

  std::string port = opts.get("port", "");
  if (port == "") throw Err() << errpref
    << "Parameter -port is empty or missing";

  errpref += addr + ":" + port + ": ";

  // fill hints structure and do getaddrinfo
  struct addrinfo hints, *servinfo, *p;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  res = getaddrinfo(addr.c_str(), port.c_str(), &hints, &servinfo);
  if (res != 0) throw Err() << errpref
    << "getaddrinfo: " << gai_strerror(res);

  // loop through all the results and connect to the first we can
  int e;
  for(p = servinfo; p != NULL; p = p->ai_next) {
    sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (sockfd == -1) {e = errno; continue; }
    res = connect(sockfd, p->ai_addr, p->ai_addrlen);
    if (res == -1) {e = errno; continue; }
    break;
  }
  if (p == NULL) {
    freeaddrinfo(servinfo);
    throw Err() << errpref
      << "can't connect: " << strerror(e);
  }
  freeaddrinfo(servinfo);

  // set bufsize
  bufsize = opts.get("bufsize", 4096);

  // set timeout
  timeout = opts.get("timeout", 5.0);

}

IONet::~IONet() {
  ::close(sockfd);
}

std::string
IONet::read() {
  char buf[bufsize];

  // Reading with timeout.
  if (timeout > 0) {
    // Prepare timeout structure and fd_set
    struct timespec timeout_s;
    timeout_s.tv_sec = int(timeout);
    timeout_s.tv_nsec = (timeout - int(timeout))*1e9;
    fd_set set;
    FD_ZERO(&set); // clear the set
    FD_SET(sockfd, &set);

    // Wait for data.
    int res = pselect(sockfd+1, &set, NULL, NULL, &timeout_s, NULL);
    if (res == -1) throw Err() << errpref << "select error: " << strerror(errno);
    if (res == 0)  throw Err() << errpref << "read timeout";
  }

  // Read data
  int fl=0;
  ssize_t ret = ::recv(sockfd, buf, sizeof(buf), fl);
  if (ret<0) throw Err() << errpref << "read error: " << strerror(errno);
  return std::string(buf, buf+ret);
}

void
IONet::write(const std::string & msg) {
  int fl = MSG_NOSIGNAL;
  ssize_t ret = ::send(sockfd, msg.data(), msg.size(), fl);
  if (ret<0) throw Err() << errpref << "write error: " << strerror(errno);
}

