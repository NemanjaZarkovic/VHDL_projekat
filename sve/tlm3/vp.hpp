#ifndef VP_HPP_
#define VP_HPP_

#include <systemc>
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/simple_target_socket.h>
#include "cpu.hpp"
#include "interconnect.hpp"
#include "ip.hpp"
#include "memory.hpp"
#include "lookuprom.hpp"
#include "dma.hpp"

class Vp :  public sc_core::sc_module
{
    public:
        Vp(sc_core::sc_module_name name,const string& image_name, int arg, char **argv);
        ~Vp();

    protected:
        Cpu cpu;
        InterCon interconnect;
        Ip ip;
        Mem memory;
        Rom rom;		
        DMA dma;
};

#endif // VP_HPP_
