///\cond HIDDEN (do not show this in Doxyden)

#include "dev_manager.h"
#include "err/assert_err.h"
#include <cassert>

using namespace std;

int
main(){
  try{

    /********************************************/
    // parsing URL
    assert(DevManager::parse_url("") ==
      vector<string>({"", "", ""}));

    assert(DevManager::parse_url("a") ==
      vector<string>({"a", "", ""}));

    assert(DevManager::parse_url("/a") ==
      vector<string>({"a", "", ""}));

    assert(DevManager::parse_url("//a") ==
      vector<string>({"", "a", ""}));

    assert(DevManager::parse_url("a/b") ==
      vector<string>({"a", "b", ""}));

    assert(DevManager::parse_url("/a/b") ==
      vector<string>({"a", "b", ""}));

    assert(DevManager::parse_url("//a/b") ==
      vector<string>({"", "a", "b"}));

    assert(DevManager::parse_url("a/b/c") ==
      vector<string>({"a", "b", "c"}));

    assert(DevManager::parse_url("/a/b/c") ==
      vector<string>({"a", "b", "c"}));

    assert(DevManager::parse_url("a//c") ==
      vector<string>({"a", "", "c"}));

    assert(DevManager::parse_url("/a//c") ==
      vector<string>({"a", "", "c"}));

    assert(DevManager::parse_url("a/b/c/d") ==
      vector<string>({"a", "b", "c/d"}));

    assert(DevManager::parse_url("/a/b/c/d") ==
      vector<string>({"a", "b", "c/d"}));

    /********************************************/
    // reading configuration file

    DevManager dm(std::cerr, 0);

    assert_err(dm.read_conf("test_data/e0.txt"), // missing file
      "can't open configuration: test_data/e0.txt");

    assert_eq(dm.devices.size(), 0);
    dm.read_conf("test_data/n1.txt"); // empty file
    assert_eq(dm.devices.size(), 0);

    assert_err(dm.read_conf("test_data/e1.txt"),
      "bad configuration file test_data/e1.txt at line 1: "
      "expected: <device name> <driver name> [-<parameter> <value>]");

    assert_err(dm.read_conf("test_data/e2.txt"),
      "bad configuration file test_data/e2.txt at line 1: "
      "unknown driver: b");

    assert_err(dm.read_conf("test_data/e3.txt"),
      "bad configuration file test_data/e3.txt at line 2: "
      "even-size list of [-<parameter> <value>] pairs expected");

    assert_err(dm.read_conf("test_data/e4.txt"),
      "bad configuration file test_data/e4.txt at line 2: "
      "parameter name should be prefixed with \"-\" and contain at least one character: c");

    assert_err(dm.read_conf("test_data/e5.txt"),
      "bad configuration file test_data/e5.txt at line 2: "
      "parameter name should be prefixed with \"-\" and contain at least one character: -");

    assert_err(dm.read_conf("test_data/e6.txt"),
      "bad configuration file test_data/e6.txt at line 2: "
      "duplicated device name: a");

    assert_eq(dm.devices.size(), 0);
    dm.read_conf("test_data/n2.txt");
    assert_eq(dm.devices.size(), 2);

    assert_err(dm.read_conf("test_data/e7.txt"), // missing file
      "bad configuration file test_data/e7.txt at line 3: "
      "duplicated device name: a");

    // error does not change configuration
    assert_eq(dm.devices.size(), 2);

  }
  catch (Err e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond