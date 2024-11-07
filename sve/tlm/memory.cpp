#ifndef MEM_C
#define MEM_C
#include "memory.hpp"

Mem::Mem(sc_module_name name):sc_module(name)
{
    mem_socket_1.register_b_transport(this, &Mem::b_transport);
    mem_socket_2.register_b_transport(this, &Mem::b_transport);
	
    cout<<"Memory constructed"<<endl;
    mem.reserve(240000);
} 

void Mem::b_transport(pl_t& pl, sc_time& offset)
{
    tlm_command cmd    = pl.get_command();
    sc_dt::uint64 addr  = pl.get_address();
    unsigned char *buf = pl.get_data_ptr();
    unsigned int len   = pl.get_data_length();
    
    uint64_t value;
        
    switch(cmd)
    {
        case TLM_WRITE_COMMAND:
            if(addr+len != mem.size())
                mem.resize(mem.size() - (mem.size()-addr)+len,0);
                
            for(unsigned int i=0; i<len; i++)
            {       
                unsigned int m = addr + i;
                mem[m]=((num_f*)buf)[i];
                //mem[m] = buf[i];
                
                //cout << "Upisano u mem:" ;
                //cout << "Adresa: " << static_cast<int>(addr-1) << ", Upisan broj: " << static_cast<int>(mem[addr-1]) << endl;
                //cout<< "mem[" << m << "] = " << mem[m] << endl;


            }
            pl.set_response_status(TLM_OK_RESPONSE);
            
            offset += sc_core::sc_time(DELAY, sc_core::SC_NS);
            break;


            
        case tlm::TLM_READ_COMMAND:
            for (unsigned int i = 0; i < len; i+=6)
            {
                value = (uint64_t)(mem[addr++] << 30);
                buf[i] = (unsigned char)(value >> 8);
                buf[i+1] = (unsigned char)(value >> 16);
                buf[i+2] = (unsigned char)(value >> 24);
                buf[i+3] = (unsigned char)(value >> 32);
                buf[i+4] = (unsigned char)(value >> 40);
                buf[i+5] = (unsigned char)(value & 0xFF);
                
                //if (addr > 17000) {
                //std::cout << "citanje iz mem: ";
                //std::cout << "adresa: " << (int)(addr-1) << " " << mem[addr-1] <<", citam: " << value << " " << (num_f)buf[i] << (num_f)buf[i+1] << std::endl << (num_f)buf[i+2] << std::endl << (num_f)buf[i+3] << std::endl << (num_f)buf[i+4] << std::endl << (num_f)buf[i+5] << std::endl;
                //std::cout << "/////////////////////////////////////////////////" << endl;
                //std::cout << "IZ MEMORIJE " <<  mem[addr-1] << endl;
               // }
            }
            pl.set_response_status(tlm::TLM_OK_RESPONSE);
            
            offset += sc_core::sc_time(DELAY, sc_core::SC_NS);
            break;
			

        default:
            pl.set_response_status( TLM_COMMAND_ERROR_RESPONSE );
            offset += sc_core::sc_time(DELAY, sc_core::SC_NS);
    }
}

#endif // MEM_C
