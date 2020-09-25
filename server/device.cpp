#include <iostream>
#include <fstream>
#include <unistd.h>

#include "err/err.h"
#include "log/log.h"
#include "read_words/read_words.h"
#include "drivers.h"
#include "device.h"

/*************************************************/
Device::Device( const std::string & dev_name,
        const std::string & drv_name,
        const Opt & drv_args):
  std::shared_ptr<Driver>(Driver::create(drv_name, drv_args)),
  dev(dev_name) {
}

Device::Device(const Device & d){
  std::shared_ptr<Driver>::operator=(d);
  dev = d.dev; users = d.users;
}

void
Device::open(const uint64_t conn){
  if (users.count(conn)>0) return; // device is opened and used by this connection
  auto lk = get_lock();
  if (users.empty()) { // device needs to be opened
    (*this)->open();
    Log(2) << "#" << conn << "/" << dev << ": open device";
  }
  users.insert(conn);
}

void
Device::close(const uint64_t conn){
  if (users.count(conn)==0) return; // device is not used by this connection
  auto lk = get_lock();
  if (users.size()==1){
    (*this)->close();
    Log(2) << "#" << conn << "/" << dev << ": close device";
  }
  users.erase(conn);
}

std::string
Device::cmd(const std::string & cmd, const std::string & arg){
  return (*this)->cmd(cmd, arg);
}

