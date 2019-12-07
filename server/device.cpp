#include <iostream>
#include <fstream>
#include <unistd.h>

#include "err/err.h"
#include "log/log.h"
#include "read_words/read_words.h"
#include "locks.h"
#include "drivers.h"
#include "device.h"

/*************************************************/
Device::Device( const std::string & dev_name,
        const std::string & drv_name,
        const Opt & drv_args):
  std::shared_ptr<Driver>(Driver::create(drv_name, drv_args)),
  Lock("manager:" + dev_name),
  dev(dev_name) {
}

void
Device::open(const uint64_t conn){
  if (users.count(conn)>0) return; // device is opened and used by this connection
  lock();
  try {
    bool do_open = users.empty(); // device need to be opened
    if (do_open) {
      (*this)->open();
      Log(2) << "#" << conn << "/" << dev << ": open device";
    }
    users.insert(conn);
  }
  catch(Err & e){
    unlock();
    throw e;
  }
  unlock();
}

void
Device::close(const uint64_t conn){
  if (users.count(conn)==0) return; // device is not used by this connection
  lock();
  try {
    bool do_close = users.size()==1;
    if (do_close){
      (*this)->close();
      Log(2) << "#" << conn << "/" << dev << ": close device";
    }
    users.erase(conn);
  }
  catch(Err & e){
    unlock();
    throw e;
  }
  unlock();
}

std::string
Device::cmd(const std::string & cmd, const std::string & arg){
  lock();
  try {
    auto ret = (*this)->cmd(cmd, arg);
    unlock();
    return ret;
  }
  catch(Err & e){
    unlock();
    throw e;
  }
}

