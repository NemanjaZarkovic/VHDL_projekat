#include <systemc>
#include "vp.hpp"

using namespace sc_core;
using namespace std;

int sc_main(int argc, char* argv[])
{   
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <image_name>" << endl;
        return 1;
    }

    string image_name = argv[1]; // Prvi argument je ime slike
    sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", SC_DO_NOTHING);

    // Kreiranje instance klase Vp
    Vp vp("Virtual Platform", image_name, argc, argv);

    // Pokretanje SystemC simulacije
    sc_start(1000, SC_NS);

    return 0;
}

