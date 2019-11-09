///\cond HIDDEN (do not show this in Doxyden)

#include "locks.h"
#include "err/assert_err.h"
#include <cassert>
#include <iostream>

using namespace std;

int
main(){
  try{
    Lock l1("a"), l2("a"), l3("b"), l4("b");
  }
  catch (Err e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond