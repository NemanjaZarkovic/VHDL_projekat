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
#define ADDR_IP_INPUT 0x00000016  
#define ADDR_IP_OUTPUT  0x00000020
using namespace std;
using namespace sc_core;

SC_MODULE(Ip)
{   
    public:
        Ip(sc_module_name name);
        ~Ip();

        tlm_utils::simple_target_socket<Ip> interconnect_socket;
        tlm_utils::simple_initiator_socket<Ip> mem_socket;
        tlm_utils::simple_initiator_socket<Ip> rom_socket;
        tlm_utils::simple_initiator_socket<Ip> dma_socket;
        
        sc_port<sc_fifo_in_if<sc_dt::sc_uint<16>>> p_fifo_in;
    				sc_port<sc_fifo_out_if<sc_dt::sc_uint<16>>> p_fifo_out;

        


    protected:
        void b_transport(pl_t&, sc_time&);
        void proc();
        
        void write_mem(sc_uint<64> addr, num_f val);
        num_f read_mem(sc_uint<64> addr);
        num_f read_rom(sc_uint<64> addr);
        
        
        vector<num_f> mem;

        sc_core::sc_time offset;
        
        sc_uint<1> ready;
        sc_uint<1> start;

        std::vector<num_f> pixels1D;
        std::vector<num_f> _lookup2;  
        vector<num_f> _lookup2_pom;   
        vector<num_f> _pixels1D;      
        vector<num_f> index1D;    
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
        num_f** _Pixels;
        num_i broj1;
        num_i broj2;
        std::vector<std::vector<std::vector<num_f>>> _index;    
};

        
#endif
