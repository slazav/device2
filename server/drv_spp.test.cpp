///\cond HIDDEN (do not show this in Doxyden)

#include "drv_spp.h"
#include "err/assert_err.h"

using namespace std;

int
main(){
  try{

    Opt o;
    o.put("prog", "echo a");
    assert_err(Driver_spp d(o), "spp: echo a: not an SPP program, header expected");

    o.put("prog", "echo '#SAA1'");
    assert_err(Driver_spp d(o), "spp: echo '#SAA1': not an SPP program, header expected");

    o.put("prog", "echo '#SPP1a'");
    assert_err(Driver_spp d(o), "can't parse value: \"1a\"");

    o.put("prog", "echo '#SPP1'");
    assert_err(Driver_spp d(o), "SPP: no #OK or #Error message: echo '#SPP1'");

    o.put("prog", "echo '#SPP1\naaa\nbbb'");
    assert_err(Driver_spp d(o), "SPP: no #OK or #Error message: echo '#SPP1\naaa\nbbb'");

    o.put("prog", "echo '#SPP1\n#\n'");
    assert_err(Driver_spp d(o), "SPP: symbol # in the beginning of a line is not protected: echo '#SPP1\n#\n'");

    o.put("prog", "echo '#SPP1\n#xxx\n'");
    assert_err(Driver_spp d(o), "SPP: symbol # in the beginning of a line is not protected: echo '#SPP1\n#xxx\n'");

    o.put("prog", "echo '#SPP1\naaa\nbbb\n#Error: something wrong'");
    assert_err(Driver_spp d(o), "something wrong");

    o.put("prog", "echo '#SPP1\naaa\nbbb\n#Fatal: something wrong'");
    assert_err(Driver_spp d(o), "something wrong");

    {
      o.put("prog", "echo '#SPP1\naaa\nbbb\n#OK'");
      Driver_spp d(o);
    }

  }
  catch (Err e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond