#ifndef DMA_H
#define DMA_H

#include <systemc>
#include "types.hpp"
#include "tlm_utils/tlm_quantumkeeper.h"
#include <string>
#include <iostream>
#include "addr.hpp"

using namespace std;
using namespace sc_core;

SC_MODULE(DMA)
{
public:
    SC_HAS_PROCESS(DMA);
    DMA(sc_module_name name);

    //TLm soket koji dobija instukcije od CPU
    tlm_utils::simple_target_socket<DMA> s_dma_t;

    //FIFO za slanje i dobijanje rezultata iz IP
    sc_port<sc_fifo_out_if<sc_dt::sc_uint<16>>> p_fifo_out;  // posalji u ip
    sc_port<sc_fifo_in_if<sc_dt::sc_uint<16>>> p_fifo_in;   // dobi podatke iz ip
protected:
    void b_transport(pl_t& pl, sc_time& offset);  
    void send_to_fifo();
    void read_from_fifo();

    int dma_status;    
    int dma_result;    
    bool dma_start;    

    
    vector<int> pom_mem;  
    tlm_command cmd;
    sc_dt::uint64 adr;
    unsigned int length;
    unsigned char *buf;
    unsigned int begin;

  
    sc_dt::sc_logic sig_send;
    sc_dt::sc_logic sig_read;
};

#endif // DMA_H

