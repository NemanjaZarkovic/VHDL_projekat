#ifndef _ROM_HPP_
#define _ROM_HPP_

#include <systemc>
#include <tlm>
#include <tlm_utils/simple_target_socket.h>
#include <vector>
#include "addr.hpp"
#include "types.hpp"
#include "utils.hpp"
#include <iostream>

using namespace std;
using namespace sc_core;

SC_MODULE(Rom)
{
    public:
        Rom(sc_core::sc_module_name name);
        tlm_utils::simple_target_socket<Rom> rom_socket;
        void b_transport(pl_t&, sc_core::sc_time&);
        
    protected:
        vector <num_f> rom;
};

#endif

