#include <string>
#include "device.h"
#include "err/err.h"
#include "read_words/read_words.h"


/*************************************************/
// print help message
void usage(){
  std::cout << "device_ping -- "
    "directly open a device (bypassing server), send message and get answer.\n"
    "Usage: device_ping <driver> [<driver options>] -- <msg>\n\n"
    "This program should be used for test/autodetect scripts (e.g. in udev rules).\n"
    "Homepage, documentation: https://github.com/slazav/device2\n"
    "V.Zavjalov, 2025, vl dot zavjalov at gmail dot com\n"
  ;
  throw Err();
}

/*************************************************/
// main function.

int
main(int argc, char ** argv) {

  try {

    // parse arguments
    std::vector<std::string> args(argv+1, argv+argc);
    if (args.size() < 1) usage();

    auto drv = args[0];
    Opt opts;
    size_t i = 1;
    while (i+2 < args.size() && args[i]!="--"){
      if (args[i].size()<2 || args[i][0]!='-') throw Err()
        << "parameter name should be prefixed with \"-\" "
        << "and contain at least one character: " << args[i];
      opts.put(args[i].substr(1), unquote_words(args[i+1]));
      i+=2;
    }
    if (i>=args.size() || args[i]!="--") throw Err()
      << "driver options should contain -<name> <value> pairs followed by -- and ";

    if (i+1>=args.size()) throw Err() << "empty message";
    auto msg = args[i+1];
    for (size_t j = i+2; j<args.size(); j++) msg += " " + args[j];

    // create Device object, send message, get answer
    Device dev("", drv, opts);
    //std::cout << dev.print();
    std::cout << dev.ask(0, msg) << "\n";;

  }
  catch (Err e){
    if (e.str()!="") std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}
