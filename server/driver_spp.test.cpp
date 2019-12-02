///\cond HIDDEN (do not show this in Doxyden)

#include "drivers.h"
#include "err/assert_err.h"

using namespace std;

int
main(){
  try{

    Opt o;
    {
      o.put("prog", "echo a");
      Driver_spp d(o);
      assert_err(d.open(), "SPP: unknown protocol, header expected: echo a");
    }

    {
      o.put("prog", "echo '#SAA1'");
      Driver_spp d(o);
      assert_err(d.open(), "SPP: unknown protocol, header expected: echo '#SAA1'");
    }

    {
      o.put("prog", "echo '#SPP1a'");
      Driver_spp d(o);
      assert_err(d.open(), "can't parse value: \"1a\"");
    }

    {
      o.put("prog", "echo '#SPP1'");
      Driver_spp d(o);
      assert_err(d.open(), "SPP: no #OK or #Error message: echo '#SPP1'");
    }

    {
      o.put("prog", "echo '#SPP1\naaa\nbbb'");
      Driver_spp d(o);
      assert_err(d.open(), "SPP: no #OK or #Error message: echo '#SPP1\naaa\nbbb'");
    }

    {
      o.put("prog", "echo '#SPP1\n#'");
      Driver_spp d(o);
      assert_err(d.open(), "SPP: symbol # in the beginning of a line is not protected: echo '#SPP1\n#'");
    }

    {
      o.put("prog", "echo '#SPP1\n#xxx'");
      Driver_spp d(o);
      assert_err(d.open(), "SPP: symbol # in the beginning of a line is not protected: echo '#SPP1\n#xxx'");
    }

    {
      o.put("prog", "echo '#SPP1\naaa\nbbb\n#Error: something wrong'");
      Driver_spp d(o);
      assert_err(d.open(), "something wrong");
    }

    {
      o.put("prog", "echo '#SPP1\naaa\nbbb\n#Fatal: something wrong'");
      Driver_spp d(o);
      assert_err(d.open(), "something wrong");
    }

    {
      o.put("prog", "echo '#SPP1\naaa\nbbb\n#OK'");
      Driver_spp d(o);
      d.open();
      d.close();
    }

  }
  catch (Err e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond