#include "drv_net.h"
#include "drv_utils.h"

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

// based on the example in
// https://beej.us/guide/bgnet/html//index.html#a-simple-stream-client

Driver_net::Driver_net(const Opt & opts) {
  opts.check_unknown({"addr","port","timeout","bufsize","errpref","idn",
    "read_cond", "add_str", "trim_str"});
  int res;

  //prefix for error messages
  errpref = opts.get("errpref", "Driver_net: ");

  // address and port (mandatory settings)
  std::string addr = opts.get("addr", "");
  if (addr == "") throw Err() << errpref
    << "Parameter -addr is empty or missing";

  std::string port = opts.get("port", "5025");

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

  bufsize = opts.get("bufsize", 4096);
  timeout = opts.get("timeout", 5.0);
  add     = opts.get("add_str",  "\n");
  trim    = opts.get("trim_str", "\n");
  idn     = opts.get("idn", "");
  read_cond = str_to_read_cond(opts.get("read_cond", "qmark1w"));
}

Driver_net::~Driver_net() {
  ::close(sockfd);
}

std::string
Driver_net::read() {
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
    auto res = pselect(sockfd+1, &set, NULL, NULL, &timeout_s, NULL);
    if (res == -1) throw Err() << errpref << "select error: " << strerror(errno);
    if (res == 0)  throw Err() << errpref << "read timeout";
  }

  // Read data
  int fl=0;
  auto res = ::recv(sockfd, buf, sizeof(buf), fl);
  if (res<0) throw Err() << errpref
    << "read error: " << strerror(errno);

  auto ret = std::string(buf, buf+res);

  trim_str(ret,trim); // -trim option
  return ret;
}

void
Driver_net::write(const std::string & msg) {

  std::string m = msg;
  if (add.size()>0) m+=add;

  int fl = MSG_NOSIGNAL;
  ssize_t ret = ::send(sockfd, m.data(), m.size(), fl);
  if (ret<0) throw Err() << errpref
    << "write error: " << strerror(errno);
}

std::string
Driver_net::ask(const std::string & msg) {

  if (idn.size() && strcasecmp(msg.c_str(),"*idn?")==0) return idn;

  write(msg);

  // if there is no '?' in the message no answer is needed.
  if (!check_read_cond(msg, read_cond)) return std::string();

  return read();
}
