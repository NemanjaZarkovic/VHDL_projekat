#ifndef MEM_H
#define MEM_H
#define SC_INCLUDE_FX

#include "types.hpp"
#include <iostream>
#include <systemc>
#include <string>
#include <fstream>
#include <vector>
#include "addr.hpp"
#include "utils.hpp"
#include "tlm_utils/tlm_quantumkeeper.h"

using namespace std;
using namespace sc_core;

SC_MODULE(Mem)
{
public:
    Mem(sc_module_name name);

    tlm_utils::simple_target_socket<Mem> mem_socket_1; 
    tlm_utils::simple_target_socket<Mem> mem_socket_2;
    void b_transport(pl_t&, sc_time&);

protected:
    vector <num_f> mem;
};

#endif // MEM_H
