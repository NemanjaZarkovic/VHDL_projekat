#include "vp.hpp"

Vp::Vp (sc_core::sc_module_name name,const string& image_name,int arg, char **argv): 
    sc_module (name),
    cpu("Cpu",image_name,arg,argv),
    interconnect("Interconnect"),
    ip("Hard"),
    memory("Memory"),
    rom("Rom"),
    dma("DMA"),
    fifo1(16),
				fifo2(16)
{
    cpu.interconnect_socket.bind(interconnect.cpu_socket);
    interconnect.mem_socket.bind(memory.mem_socket_1);
    interconnect.ip_socket.bind(ip.interconnect_socket);
    ip.mem_socket.bind(memory.mem_socket_2);
    ip.rom_socket.bind(rom.rom_socket);
    
    cpu.dma_socket.bind(dma.s_dma_t);
    
    dma.p_fifo_out.bind(fifo1);  // Send data from DMA to IP (FIFO1)
    dma.p_fifo_in.bind(fifo2);   // Receive data from IP to DMA (FIFO2)
    
    ip.p_fifo_in.bind(fifo1);    // IP reads data from FIFO1
    ip.p_fifo_out.bind(fifo2);   // IP writes results to FIFO2
    SC_REPORT_INFO("Virtual Platform", "Constructed.");
}

Vp::~Vp()
{
     SC_REPORT_INFO("Virtual Platform", "Destroyed.");
}
