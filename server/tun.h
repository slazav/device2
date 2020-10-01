#ifndef TUN_H
#define TUN_H

#include "opt/opt.h"

// find a free port
int find_free_port (void);

// Create a tunnel using ssh.
// Options needed: via, server, port and optional via_cmd
// Return value: server:port for the connection
std::string create_tunnel(const Opt & opts);

#endif