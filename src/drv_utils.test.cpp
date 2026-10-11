///\cond HIDDEN (do not show this in Doxyden)

#include "drv_utils.h"
#include "err/assert_err.h"

using namespace std;

int
main(){
  try{

    std::string s("abcdefghij");
    assert_eq(trim_str(s, "ij"), true);
    assert_eq(s, "abcdefgh");
    assert_eq(trim_str(s, "gj"), false);
    assert_eq(s, "abcdefgh");

    assert_eq(str_to_read_cond("always"),  READCOND_ALWAYS);
    assert_eq(str_to_read_cond("never"),   READCOND_NEVER);
    assert_eq(str_to_read_cond("qmark"),   READCOND_QMARK);
    assert_eq(str_to_read_cond("qmark1w"), READCOND_QMARK1W);
    assert_err(str_to_read_cond("xxx"), "unknown -read_cond value: xxx");

    assert_eq(check_read_cond("ABC",  READCOND_ALWAYS), true);
    assert_eq(check_read_cond("ABC",  READCOND_NEVER),  false);
    assert_eq(check_read_cond("ABC",  READCOND_QMARK),  false);
    assert_eq(check_read_cond("ABC?", READCOND_QMARK),  true);

    assert_eq(check_read_cond("DISP:TEXT WHAT?!", READCOND_QMARK1W),  false);
    assert_eq(check_read_cond("DISP:TEXT? (1)", READCOND_QMARK1W),  true);

  }
  catch (Err e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond