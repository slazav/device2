#include "drv_spp.h"
#include "err/err.h"
#include <cstring> // strcasecmp

std::string
Driver_spp::read_spp(double timeout){
  if (!flt) throw Err() << "SPP: read from closed device";
  std::string ret;
  while (1){
    std::string l;

    // Read from SPP program with timeout.
    // Err is thrown if error happens.
    // Return -1 on EOF.
    int res = flt->getline(l, timeout);
    // line starts with the special character
    if (l.size()>0 && l[0] == ch){
      if (l.substr(1,7) == "Error: ") throw Err() << l.substr(8);
      if (l.substr(1,7) == "Fatal: ") throw Err() << l.substr(8);
      if (l.substr(1) == "OK") return ret;

      if (ret.size()>0) ret += '\n';
      if (l.size()>1 && l[1] == ch) ret += l.substr(1);
      else throw Err() << "SPP: symbol " << ch <<
        " in the beginning of a line is not protected: " << prog;
    }
    else {
      if (ret.size()>0) ret += '\n';
      ret += l;
    }
    if (res<0) break;
  }
  throw Err() << "SPP: no #OK or #Error message: " << prog;
}


Driver_spp::Driver_spp(const Opt & opts) {
  opts.check_unknown({"prog", "open_timeout", "read_timeout", "errpref", "idn"});

  //prefix for error messages
  errpref = opts.get("errpref", "spp: ");

  prog = opts.get("prog");
  if (prog == "") throw Err() << errpref
    << "parameter -prog is empty or missing";

  errpref += prog + ": ";

  open_timeout = opts.get<double>("open_timeout", 20.0);
  read_timeout = opts.get<double>("read_timeout", 10.0);

  flt.reset(new IOFilter(prog));
  try {

    // first line: <symbol>SPP<version>
    std::string l;
    flt->getline(l, open_timeout);
    if (l.size()<5 || l[1]!='S' || l[2]!='P' || l[3]!='P')
      throw Err() << errpref
        << "not an SPP program, header expected";
    ch  = l[0];
    int ver = str_to_type<int>(l.substr(4));
    if (ver!=1 && ver!=2) throw Err() << errpref
      <<"unsupported SPP version";
    read_spp(open_timeout); // ignore message, throw errors
  }
  catch (Err e) {
    if (flt){
      flt->close_input();
      flt->kill();
      flt.reset();
    }
    throw;
  }
  idn = opts.get("idn", "");

}


Driver_spp::~Driver_spp() {
  if (!flt) return;
  flt->close_input();
  flt->kill();
  flt.reset();
}

std::string
Driver_spp::read() {
  if (!flt) throw Err() << errpref
    << "device is closed";
  return read_spp(read_timeout);
}

void
Driver_spp::write(const std::string & msg) {
  if (!flt) throw Err() << errpref
    << "device is closed";
  flt->ostream() << msg << "\n";
  flt->ostream().flush();
}


std::string
Driver_spp::ask(const std::string & msg) {
  if (idn.size() && strcasecmp(msg.c_str(),"*idn?")) return idn;
  write(msg);
  return read();
}
