#include <cstring>
#include "drv_gpib.h"

const char*
error_text(int ecode) {
  switch(ecode) {
    case EDVR: return strerror(ibcnt); // system error
    case ECIC: return "board is not a CIC";
    case ENOL: return "no listeners";
    case EADR: return "board is not addressed correctly";
    case EARG: return "invalid argument";
    case ESAC: return "board is not a system controller";
    case EABO: return "I/O operation aborted (timeout)";
    case ENEB: return "board does not exists";
    case EDMA: return "DMA error"; // not used
    case EOIP: return "asynchronous I/O in progress";
    case ECAP: return "no capability for operation";
    case EFSO: return strerror(ibcnt); // file system error
    case EBUS: return "bus error";
    case ESTB: return "serial poll queue overflow";
    case ESRQ: return "SRQ stuck in ON position";
    case ETAB: return "table problem";
  }
  return "unknown error";
}

// Convert user-readable timeouts to libgpib constants.
// return -1 on error.
int
Driver_gpib::get_timeout(const std::string & s){
  if (s=="none")  return TNONE;
  if (s=="10us")  return T10us;
  if (s=="30us")  return T30us;
  if (s=="100us") return T100us;
  if (s=="300us") return T300us;
  if (s=="1ms")   return T1ms;
  if (s=="3ms")   return T3ms;
  if (s=="10ms")  return T10ms;
  if (s=="30ms")  return T30ms;
  if (s=="100ms") return T100ms;
  if (s=="300ms") return T300ms;
  if (s=="1s")    return T1s;
  if (s=="3s")    return T3s;
  if (s=="10s")   return T10s;
  if (s=="30s")   return T30s;
  if (s=="100s")  return T100s;
  if (s=="300s")  return T300s;
  if (s=="1000s") return T1000s;
  throw Err() << errpref << "bad gpib timeout: " << s;
}


/************************************************/

Driver_gpib::Driver_gpib(const Opt & opts) {

  opts.check_unknown({"addr","board","timeout","open_timeout",
    "eot", "eos", "eos_mode", "secondary", "bufsize",
    "errpref","idn", "add_str", "trim_str"});

  //prefix for error messages
  errpref = opts.get("errpref", "gpib: ");

  // address (mandatory setting) and board
  int addr = opts.get("addr", -1);
  if (addr <0) throw Err() << errpref
    << "Parameter -addr is empty or missing";
  int board = opts.get("board", 0);

  errpref += type_to_str(board) + ":" + type_to_str(addr) + ": ";

  int open_timeout = get_timeout(opts.get("open_timeout", "3s"));
  int timeout      = get_timeout(opts.get("timeout", "1s"));
  bool eot         = opts.get("eot", false);
  int  eos         = opts.get("eos", -1);
  bool secondary   = opts.get("secondary", false);

  int res;

  // open device
  dh = ibdev(board, addr, 0, open_timeout, 1, 0);
  if (ibsta & ERR) throw Err() << errpref << error_text(iberr);

  try {
    // set timeout
    res = ibtmo(dh, timeout);
    if (ibsta & ERR) throw Err() << errpref << error_text(iberr);

    // set EOT
    if (opts.exists("eot")){
      res = ibeot(dh, opts.get("eot", false));
      if (ibsta & ERR) throw Err() << errpref << error_text(iberr);
    }

    // set EOS
    if (opts.exists("eos")){
      int ch    = opts.get("eos", 0) & 0xff;
      auto mode = opts.get("eos_mode", "none");
      if      (mode == "bin")   ch |= BIN;
      else if (mode == "write") ch |= XEOS;
      else if (mode == "read")  ch |= REOS;
      else if (mode != "none")  throw Err() << errpref
        << "unknown -eos_mode value: " << mode;
      res = ibeos(dh, ch);
      if (ibsta & ERR) throw Err() << errpref << error_text(iberr);
    }

    // set secondary GPIB address
    if (opts.exists("secondary")){
      res = ibsad(dh, opts.get("secondary", 0));
      if (ibsta & ERR) throw Err() << errpref << error_text(iberr);
    }

    bufsize = opts.get("bufsize", 4096);
    add     = opts.get("add_str",  "");
    trim    = opts.get("trim_str", "");
    idn     = opts.get("idn", "");
  }
  catch (Err & e){
    ibonl(dh, 0);
    throw;
  }
}

Driver_gpib::~Driver_gpib() {
  ibonl(dh, 0);
}

std::string
Driver_gpib::read() {
  char buf[bufsize];
  auto res = ibrd(dh, buf, sizeof(buf));
  if (ibsta & ERR) throw Err() << errpref << error_text(iberr);

  auto ret = std::string(buf, buf+res);

  // -trim option
  if (trim.size()>0 &&
      ret.size() >= trim.size() &&
      ret.substr(ret.size()-trim.size()-1) == trim){
      ret.resize(ret.size()-trim.size());
  }
  return ret;
}

void
Driver_gpib::write(const std::string & msg) {
  std::string m = msg;
  if (add.size()>0) m+=add;

  auto ret = ibcmd(dh, m.data(), m.size());
  if (ibsta & ERR) throw Err() << errpref << error_text(iberr);
}

std::string
Driver_gpib::ask(const std::string & msg) {

  if (idn.size() && strcasecmp(msg.c_str(),"*idn?")) return idn;

  write(msg);

  // if we do not have '?' in the message
  // no answer is needed.
  if (msg.find('?') == std::string::npos)
    return std::string();

  return read();
}
