#include "tun.h"

#include "err/err.h"
#include <string>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

// based on tigervnc/common/network/TcpSocket.cxx
int
find_free_port (void) {
  int sock;
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;

  if ((sock = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    throw Err() << "FindFreePort: unable to create socket: " << strerror(errno);

  addr.sin_port = 0;
  if (bind (sock, (struct sockaddr *)&addr, sizeof (addr)) < 0)
    throw Err() << "FindFreePort: unable find free port: " << strerror(errno);

  socklen_t n = sizeof(addr);
  if (getsockname (sock, (struct sockaddr *)&addr, &n) < 0)
    throw Err() << "FindFreePort: unable to get port number: " << strerror(errno);

  close(sock);
  return ntohs(addr.sin_port);
}

// based on tigervnc/vncviewer/vncviewer.cxx
std::string
create_tunnel(const Opt & opts) {
  auto cmd = opts.get("via_cmd",
    "/usr/bin/ssh -f -L \"$L\":\"$H\":\"$R\" \"$G\" sleep 20");

  auto via = opts.get("via");
  auto srv = opts.get("server", "localhost");
  auto rport = opts.get("port", 8082);
  auto lport = find_free_port();

  auto srport = type_to_str(rport);
  auto slport = type_to_str(lport);

  setenv("G", via.c_str(), 1);
  setenv("H", srv.c_str(), 1);
  setenv("R", srport.c_str(), 1);
  setenv("L", slport.c_str(), 1);

  //std::replace(cmd.begin(),cmd.end(), '%', '$')
  auto res = system(cmd.c_str());
  if (res == -1) throw Err() << "can't make -via tunnel: " << strerror(errno);
  if (res != 0) throw Err() << "can't make -via tunnel";

  return srv + ":" + slport;
}
