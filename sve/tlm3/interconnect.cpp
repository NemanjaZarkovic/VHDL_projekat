#ifndef INT_CON_C
#define INT_CON_C
#include "interconnect.hpp"

InterCon::InterCon(sc_module_name name):sc_module(name)
{
    cpu_socket.register_b_transport(this, &InterCon::b_transport);
    cout<< "Interconnect constructed" << endl;
}

void InterCon::b_transport(pl_t& pl, sc_time& offset)
{
    sc_dt::uint64 address = pl.get_address();
    string msg;
    offset += sc_time(2, SC_NS);
        
    if (address >= 0 && address < 100230)
    mem_socket -> b_transport(pl, offset);  //memorija
    
    else if (address >= 100300)
    ip_socket -> b_transport(pl, offset); //IP    
}

#endif //INT_CON_C
