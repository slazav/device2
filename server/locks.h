#ifndef LOCKS_H
#define LOCKS_H

#include <memory>
#include <string>
#include <map>

/************************************************/
// Lock class: get a lock for a given resource
// name and use it to lock/unlock the resource.

class Lock {

  // static storage for all mutexes
  static std::map<std::string, std::shared_ptr<void> > mutexes;

  // resource name
  std::string name;

  // pointer to the mutex for requested resource
  std::shared_ptr<void> mutex;

public:

  // Get a lock for a given resource name.
  Lock(const std::string & name);
  ~Lock();

  void lock();

  void unlock();

};

#endif