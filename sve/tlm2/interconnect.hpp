#ifndef INT_CON_H
#define INT_CON_H

#include <systemc>
#include <iostream>
#include <string>
#include "types.hpp"
#include "tlm_utils/tlm_quantumkeeper.h"

using namespace std;
using namespace sc_core;

SC_MODULE(InterCon)
{
public:
    InterCon(sc_module_name name);
    tlm_utils::simple_target_socket<InterCon> cpu_socket;
    tlm_utils::simple_initiator_socket<InterCon> ip_socket;
    tlm_utils::simple_initiator_socket<InterCon> mem_socket;

protected:
    void b_transport(pl_t&, sc_core::sc_time&);
};

#endif // INT_CON_H

