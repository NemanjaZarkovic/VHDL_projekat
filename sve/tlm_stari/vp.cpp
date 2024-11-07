#include "vp.hpp"

Vp::Vp (sc_core::sc_module_name name,const string& image_name,int arg, char **argv): 
    sc_module (name),
    cpu("Cpu",image_name,arg,argv),
    interconnect("Interconnect"),
    ip("Hard"),
    memory("Memory"),
    rom("Rom")

{
    cpu.interconnect_socket.bind(interconnect.cpu_socket);
    interconnect.mem_socket.bind(memory.mem_socket_1);
    interconnect.ip_socket.bind(ip.interconnect_socket);
    ip.mem_socket.bind(memory.mem_socket_2);
    ip.rom_socket.bind(rom.rom_socket);

    SC_REPORT_INFO("Virtual Platform", "Constructed.");
}

Vp::~Vp()
{
     SC_REPORT_INFO("Virtual Platform", "Destroyed.");
}
