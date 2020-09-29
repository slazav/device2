#include <iostream>
#include <fstream>
#include <unistd.h>

#include "err/err.h"
#include "log/log.h"
#include "read_words/read_words.h"
#include "device.h"

/*************************************************/
Device::Device( const std::string & dev_name,
        const std::string & drv_name,
        const Opt & drv_args):
  dev_name(dev_name),
  drv_name(drv_name),
  drv_args(drv_args),
  locked(false),
  max_log_size(1024) {
}

Device::Device(const Device & d){
  users = d.users;
  drv = d.drv;
  dev_name = d.dev_name;
  drv_name = d.drv_name;
  drv_args = d.drv_args;
  locked = d.locked;
  max_log_size = d.max_log_size;
}

void
Device::use(const uint64_t conn){
  if (users.count(conn)>0) return; // device is opened and used by this connection
  if (locked) throw Err() << "device is locked";
  auto lk = get_lock();
  if (users.empty()) { // device needs to be opened
    drv = Driver::create(drv_name, drv_args);
    Log(2) << "conn:" << conn << " open device: " << dev_name;
  }
  users.insert(conn);
}

void
Device::release(const uint64_t conn){
  auto lk = get_lock();

  // remove log buffer
  if (log_bufs.count(conn)>0)
    log_bufs.erase(conn);

  // device is not used by this connection
  if (users.count(conn)==0) return;

  // if device is used by only this connection close it
  if (users.size()==1){
    drv.reset();
    Log(2) << "conn:" << conn << " close device: " << dev_name;
  }
  if (locked) locked = false;
  users.erase(conn);
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
Device::log_start(const uint64_t conn){
  auto lk = get_lock();
  log_bufs[conn] = log_buf_t();
}

void
Device::log_finish(const uint64_t conn){
  auto lk = get_lock();
  log_bufs.erase(conn);
}

std::string
Device::log_get(const uint64_t conn){
  auto lk = get_lock();
  if (log_bufs.count(conn)==0)
    throw Err() << "Logging is off";
  std::string ret;
  while (log_bufs[conn].size()>0){
    ret += *log_bufs[conn].front();
    log_bufs[conn].pop();
  }
  return ret;
}

void
Device::log_message(const std::string & pref, const std::string & msg){
  if (log_bufs.size()==0) return;
  // make shared_ptr to share it between log buffers
  std::shared_ptr<std::string> s(new std::string);
  *s += pref + msg + "\n";
  for (auto & b: log_bufs){
    b.second.push(s);
    // limit log buffer size to max_log_size
    while (b.second.size()>max_log_size) b.second.pop();
  }
}

// Send message to the device, get answer
std::string
Device::ask(const uint64_t conn, const std::string & msg){

  // open device if needed
  if (users.count(conn)==0) use(conn);

  if (locked) throw Err() << "device is locked";

  // if no logging is needed just return answer
  if (log_bufs.size()==0) return drv->ask(msg);

  // do all logging (message, answer, errors)
  log_message(">> ", msg);
  try {
    auto ret = drv->ask(msg);
    log_message("<< ", ret);
    return ret;
  }
  catch (Err & e) {
    log_message("EE ", e.str());
    throw;
  }
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
