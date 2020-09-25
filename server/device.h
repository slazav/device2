#ifndef DEVICE_H
#define DEVICE_H

#include <set>
#include <string>
#include <memory>

#include "err/err.h"
#include "opt/opt.h"
#include "drivers.h"
#include <mutex>

/*************************************************/

// A device object represents a device in the configuration
// file.

class Device {

  // Device driver
  std::shared_ptr<Driver> drv;

  // Connections which use the device
  std::set<uint64_t> users;

  // Device name
  std::string dev_name;

  // Driver name and args (for printing device information)
  std::string drv_name;
  Opt drv_args;

  // Mutex for locking device data
  std::mutex data_mutex;

  // Get lock for the mutex
  std::unique_lock<std::mutex> get_lock() {
    return std::unique_lock<std::mutex>(data_mutex);}

public:
  // Constructor
  Device( const std::string & dev_name,
          const std::string & drv_name,
          const Opt & drv_args);

  // Copy constructor
  Device(const Device & d);

  // Reserve device for a connection.
  // Open it if there were no reservations.
  void open(const uint64_t conn);

  // Remove reservation for a connection.
  // Close the device if no reservations left.
  void close(const uint64_t conn);

  // Send command to the device
  std::string cmd(const std::string & cmd, const std::string & arg);

  // print device information: name, users, driver, driver arguments
  std::string print(const uint64_t conn=0) const;

};

#endif
