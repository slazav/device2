#include <iostream>
#include "locks.h"

#undef MHD_LOCKS_H
#define HAVE_PTHREAD_H
#define MHD_USE_POSIX_THREADS
#include "mhd_locks.h"


std::map<std::string, std::shared_ptr<void> >
Lock::mutexes;

Lock::Lock(const std::string & name): name(name){
  if (mutexes.count(name)==0){
    mutex.reset(new MHD_mutex_);
    MHD_mutex_init_((MHD_mutex_ *)mutex.get());
    mutexes.emplace(name, mutex);
  }
  else {
    mutex = mutexes.find(name)->second;
  }
}

Lock::~Lock(){
  if (mutex.use_count() != 2) return;
  MHD_mutex_destroy_((MHD_mutex_ *)mutex.get());
  mutexes.erase(name);
}

void
Lock::lock()   { MHD_mutex_lock_((MHD_mutex_ *)mutex.get());}
void
Lock::unlock() { MHD_mutex_unlock_((MHD_mutex_ *)mutex.get());}
