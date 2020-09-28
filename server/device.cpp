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
  drv(Driver::create(drv_name, drv_args)),
  dev_name(dev_name),
  drv_name(drv_name),
  drv_args(drv_args),
  locked(false) {
}

Device::Device(const Device & d){
  drv = d.drv;
  dev_name = d.dev_name;
  drv_name = d.drv_name;
  drv_args = d.drv_args;
  locked = d.locked;
  users = d.users;
}

void
Device::use(const uint64_t conn){
  if (users.count(conn)>0) return; // device is opened and used by this connection
  if (locked) throw Err() << "device is locked";
  auto lk = get_lock();
  if (users.empty()) { // device needs to be opened
    drv->open();
    Log(2) << "conn:" << conn << " open device: " << dev_name;
  }
  users.insert(conn);
}

void
Device::lock(const uint64_t conn){
  // to lock the device we should be its only user.
  if (users.count(conn)==0) use(conn);
  auto lk = get_lock();
  if (users.size()!=1)
    throw Err() << "Can't lock the device: it is in use";
  locked = true;
}

void
Device::unlock(const uint64_t conn){
  auto lk = get_lock();
  if (!locked)
    throw Err() << "device is not locked";
  if (users.count(conn)==0)
    throw Err() << "device is locked by another connection";
  locked = false;
}

void
Device::release(const uint64_t conn){
  if (users.count(conn)==0) return; // device is not used by this connection
  auto lk = get_lock();
  if (users.size()==1){
    drv->close();
    Log(2) << "conn:" << conn << " close device: " << dev_name;
  }
  if (locked) locked = false;
  users.erase(conn);
}

// Send message to the device, get answer
std::string
Device::ask(const std::string & msg){
  if (locked) throw Err() << "device is locked";
  return drv->ask(msg);
}

std::string
Device::print(const uint64_t conn) const {
  std::ostringstream s;
  s << "Device: " << dev_name << "\n"
    << "Driver: " << drv_name << "\n";
  if (drv_args.size())
    s << "Driver arguments:\n";
  for (auto const & o:drv_args)
    s << "  -" << o.first << ": " << o.second << "\n";
  s << "Device is " << (users.size()>0 ? "open":"closed") << "\n";
  s << "Number of users: " << users.size() << "\n";
  if (conn && users.count(conn))
    s << "You are currently using the device\n";
  if (locked)
    s << "Device is locked\n";
  return s.str();
}
