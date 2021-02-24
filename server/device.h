#ifndef DEVICE_H
#define DEVICE_H

#include <set>
#include <map>
#include <queue>
#include <string>
#include <memory>

#include "err/err.h"
#include "opt/opt.h"
#include "drv.h"
#include <mutex>

/*************************************************/

// A device object represents a device in the configuration
// file.

class Device {

  // Device driver (non-null if device is in use)
  std::shared_ptr<Driver> drv;

  // Connections which use the device
  std::set<uint64_t> users;

  // Is the device locked by a single user?
  bool locked;

  // Device name
  std::string dev_name;

  // Driver name and args (for printing device information)
  std::string drv_name;
  Opt drv_args;

  // Mutex for locking device data
  std::mutex data_mutex;

  // Get lock for the mutex
  std::unique_lock<std::mutex> get_data_lock() {
    return std::unique_lock<std::mutex>(data_mutex);}

  // Mutex for locking write+read commands
  std::mutex cmd_mutex;

  // Get lock for the mutex
  std::unique_lock<std::mutex> get_cmd_lock() {
    return std::unique_lock<std::mutex>(cmd_mutex);}

  // Log buffers: conn -> list(shared_ptr(strings))
  // Each connection can start its own log buffer and
  // get data from it independently.
  // shared_ptr is used to share messages between log buffers.
  typedef std::queue<std::shared_ptr<std::string> > log_buf_t;
  std::map<uint64_t, log_buf_t> log_bufs;

  // Max number of lines in the log
  size_t max_log_size;

  // log a message with a prefix
  void log_message(const std::string & pref, const std::string & msg);

public:
  // Constructor
  Device( const std::string & dev_name,
          const std::string & drv_name,
          const Opt & drv_args);

  // Copy constructor
  Device(const Device & d);

  // Start using the device by a connection.
  // Open it if nobody else use it.
  void use(const uint64_t conn);

  // Stop using the device by a connection.
  // Close it if nobody else use it.
  void release(const uint64_t conn);

  // Lock device by a connection.
  // It can be done only if the connection is the only user of the device.
  // If device is locked other connections can not use it.
  void lock(const uint64_t conn);

  // Unlock device if it is locked by the connection.
  void unlock(const uint64_t conn);

  // Create a logging buffer for the connection,
  // start logging all communication of the device
  // to this buffer. If buffer exists reset it.
  void log_start(const uint64_t conn);

  // Delete the log buffer for this connection
  void log_finish(const uint64_t conn);

  // Get contents of the log buffer and clear it.
  std::string log_get(const uint64_t conn);

  // Send message to the device, get answer
  std::string ask(const uint64_t conn, const std::string & msg);

  // Print device information: name, users, driver, driver arguments.
  std::string print(const uint64_t conn=0) const;

};

#endif
