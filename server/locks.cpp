#include <iostream>
#include "locks.h"

#undef MHD_LOCKS_H
#define HAVE_PTHREAD_H
#define MHD_USE_POSIX_THREADS
#include "mhd_locks.h"

MHD_mutex_ lock_for_locks;

std::map<std::string, std::shared_ptr<void> >
Lock::mutexes;

Lock::Lock(const std::string & name): name(name){
  if (mutexes.count(name)==0){
    mutex.reset(new MHD_mutex_);
    MHD_mutex_init_((MHD_mutex_ *)mutex.get());
    MHD_mutex_lock_(&lock_for_locks);
    mutexes.emplace(name, mutex);
    MHD_mutex_unlock_(&lock_for_locks);
  }
  else {
    mutex = mutexes.find(name)->second;
  }
}

Lock::~Lock(){
  if (mutex.use_count() != 2) return;
  MHD_mutex_destroy_((MHD_mutex_ *)mutex.get());
  MHD_mutex_lock_(&lock_for_locks);
  mutexes.erase(name);
  MHD_mutex_unlock_(&lock_for_locks);
}

void
Lock::lock()   { MHD_mutex_lock_((MHD_mutex_ *)mutex.get());}
void
Lock::unlock() { MHD_mutex_unlock_((MHD_mutex_ *)mutex.get());}
