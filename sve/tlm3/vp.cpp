#include "vp.hpp"

Vp::Vp (sc_core::sc_module_name name,const string& image_name,int arg, char **argv): 
    sc_module (name),
    cpu("Cpu",image_name,arg,argv),
    interconnect("Interconnect"),
    ip("Hard"),
    memory("Memory"),
    rom("Rom"),
    dma("DMA")
				
{
    cpu.interconnect_socket.bind(interconnect.cpu_socket);
    interconnect.mem_socket.bind(memory.mem_socket_1);
    interconnect.ip_socket.bind(ip.interconnect_socket);
    ip.mem_socket.bind(memory.mem_socket_2);
    ip.rom_socket.bind(rom.rom_socket);
    dma.dma_send.bind(interconnect.cpu_socket);
    sc_fifo<sc_dt::sc_uint<16>> fifo1(2);
    sc_fifo<sc_dt::sc_uint<16>> fifo2(2);
    
    dma.p_fifo_out.bind(fifo1);
    ip.p_fifo_in.bind(fifo1);

    dma.p_fifo_in.bind(fifo2);
    ip.p_fifo_out.bind(fifo2);


    SC_REPORT_INFO("Virtual Platform", "Constructed.");
}

Vp::~Vp()
{
     SC_REPORT_INFO("Virtual Platform", "Destroyed.");
}
