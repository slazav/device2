#include <cstring>
#include "drv_gpib.h"
#include "drv_utils.h"
#include <unistd.h>

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
    "errpref", "idn", "read_cond", "add_str", "trim_str", "delay"});

  //prefix for error messages
  errpref = opts.get("errpref", "gpib: ");

  // address (mandatory setting) and board
  int addr = opts.get("addr", -1);
  if (addr <0) throw Err() << errpref
    << "Parameter -addr is empty or missing";
  int board = opts.get("board", 0);

  errpref += std::string("dev:") + type_to_str(board) + "." + type_to_str(addr) + ": ";

  int open_timeout = get_timeout(opts.get("open_timeout", "10s"));
  int timeout      = get_timeout(opts.get("timeout", "3s"));
  bool eot         = opts.get("eot", false);
  int  eos         = opts.get("eos", -1);
  bool secondary   = opts.get("secondary", false);

  int res;

  // open device
  dh = ibdev(board, addr, 0, open_timeout, 1, 0);
  if (ibsta & ERR) throw Err() << errpref
    << "opening the device: " << error_text(iberr);

  try {

    // device clear
    ibclr(dh);
    if (ibsta & ERR) throw Err() << errpref
      << "device clear: " << error_text(iberr);

    // set timeout
    ibtmo(dh, timeout);
    if (ibsta & ERR) throw Err() << errpref
      << "setting timeout: " << error_text(iberr);

    // set EOT
    if (opts.exists("eot")){
      ibeot(dh, opts.get("eot", false));
      if (ibsta & ERR) throw Err() << errpref
        << "setting EOT: " << error_text(iberr);
    }

    // set EOS
    if (opts.exists("eos")){
      int ch    = opts.get("eos", 0) & 0xff;
      auto mode = opts.get("eos_mode", "");
      for (const char c:mode){
        if      (c == 'B') ch |= BIN;
        else if (c == 'X') ch |= XEOS;
        else if (c == 'R') ch |= REOS;
        else throw Err() << errpref
          << "unknown -eos_mode value (only B X R characters are allowed): " << mode;
      }
      ibeos(dh, ch);
      if (ibsta & ERR) throw Err() << errpref
        << "setting EOS: " << error_text(iberr);
    }

    // set secondary GPIB address
    if (opts.exists("secondary")){
      ibsad(dh, opts.get("secondary", 0));
      if (ibsta & ERR) throw Err() << errpref
        << "setting secondary addess: " << error_text(iberr);
    }

    bufsize = opts.get("bufsize", 4096);
    add     = opts.get("add_str",  "\n");
    trim    = opts.get("trim_str", "\n");
    delay   = opts.get("delay",    0);
    idn     = opts.get("idn", "");
    read_cond = str_to_read_cond(opts.get("read_cond", "qmark1w"));
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
  ibrd(dh, buf, sizeof(buf));
  if (ibsta & ERR) throw Err() << errpref
    << "read error: " << error_text(iberr);

  auto ret = std::string(buf, buf+ibcntl);

  trim_str(ret,trim); // -trim option
  return ret;
}

void
Driver_gpib::write(const std::string & msg) {
  std::string m = msg;
  if (add.size()>0) m+=add;

  auto ret = ibwrt(dh, m.data(), m.size());
  if (ibsta & ERR) throw Err() << errpref
    << "write error: " << error_text(iberr);

  if (delay>0) usleep(delay*1e6);
}

std::string
Driver_gpib::ask(const std::string & msg) {

  if (idn.size() && strcasecmp(msg.c_str(),"*idn?")==0) return idn;

  write(msg);

  // if there is no '?' in the message no answer is needed.
  if (!check_read_cond(msg, read_cond)) return std::string();

  return read();
}
