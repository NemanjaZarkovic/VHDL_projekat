#ifndef IP_H
#define IP_H
#define SC_INCLUDE_FX

#include <iostream>
#include <systemc>
#include <string>
#include <fstream>
#include <deque>
#include <vector>
#include <array>
#include <algorithm>
#include "utils.hpp"
#include "types.hpp"
#include "addr.hpp"
#include "tlm_utils/tlm_quantumkeeper.h"
#include "dma.hpp" 
#define addr_data 0x81000000 

using namespace std;
using namespace sc_core;

SC_MODULE(Ip)
{   
public:
    Ip(sc_module_name name, DMA* dma_ptr);  // Dodajemo parametar za DMA pointer
    ~Ip();

    tlm_utils::simple_target_socket<Ip> interconnect_socket;
    tlm_utils::simple_initiator_socket<Ip> mem_socket;
    tlm_utils::simple_initiator_socket<Ip> rom_socket;

    // DMA povezivanje
    DMA* dma;

protected:
    void b_transport(pl_t&, sc_time&);
    void proc();

    void write_mem(sc_uint<64> addr, num_f val);
    num_f read_mem(sc_uint<64> addr);
    num_f read_rom(sc_uint<64> addr);
    
    // Funkcije za rad sa DMA (preko FIFO bafera)
    void send_data_to_dma();
    num_f receive_data_from_dma();
    
    vector<num_f> mem;
    sc_core::sc_time offset;

    sc_uint<1> ready;
    sc_uint<1> start;

    std::vector<num_f> pixels1D;
    std::vector<num_f> _lookup2;     
    num_i iradius;
    num_f fracr;
    num_f fracc;
    num_f inv_spacing;
    num_f rpos;
    num_f cpos;
    num_i step;
    num_f _cose;
    num_f _sine;
    num_i iy;
    num_i ix;
    num_f rx;
    num_f cx;
    num_i scale;

    std::vector<std::vector<std::vector<num_f>>> _index; 

    // Dodajemo metode za interakciju sa FIFO baferima
    void read_from_fifo();
    void send_to_fifo(num_f data);

    // DMA FIFO interfejsi
    sc_port<sc_fifo_in_if<num_f>> p_fifo_in;  // FIFO za ulaz iz DMA
    sc_port<sc_fifo_out_if<num_f>> p_fifo_out; // FIFO za izlaz ka DMA
};

#endif // IP_H

