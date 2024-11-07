#include "lookuprom.hpp"

Rom::Rom(sc_core::sc_module_name name) : sc_module(name)
{
    rom_socket.register_b_transport(this, &Rom::b_transport);
    rom.reserve(360);
    rom = {.939411163330078125, .829029083251953125, .7316131591796875, .6456451416015625, .569782257080078125, .50283050537109375, .443744659423828125, .391605377197265625, .34558868408203125, .304981231689453125, .269145965576171875, .237518310546875, .2096099853515625, .184978485107421875, .163242340087890625, .144062042236328125, .127132415771484375, .112194061279296875, .099010467529296875, .087375640869140625, .07711029052734375, .068050384521484375, .06005096435546875, .052997589111328125, .0467681884765625, .041271209716796875, .0364227294921875, .03214263916015625, .0283660888671875, .02503204345703125, .022090911865234375, .01949310302734375, .01720428466796875, .0151824951171875, .013397216796875, .011821746826171875, .010433197021484375, .00920867919921875, .00812530517578125, .007171630859375};
    
    SC_REPORT_INFO("ROM", "Constructed.");
}

void Rom::b_transport(pl_t &pl, sc_core::sc_time &offset)
{
    tlm::tlm_command cmd = pl.get_command();
    sc_dt::uint64 addr = pl.get_address();
    unsigned int len = pl.get_data_length();
    unsigned char *buf = pl.get_data_ptr();
    
    uint64_t value;
    
    switch(cmd)
    {
        case tlm::TLM_READ_COMMAND:
            for (int i = 0; i < len; i+=6)
            {   
                int m = addr - addr_rom;
                value = (uint64_t)(rom[m++] << 30);
                buf[i] = (unsigned char)(value >> 8);
                buf[i+1] = (unsigned char)(value >> 16);
                buf[i+2] = (unsigned char)(value >> 24);
                buf[i+3] = (unsigned char)(value >> 32);
                buf[i+4] = (unsigned char)(value >> 40);
                buf[i+5] = (unsigned char)(value & 0xFF);
            }
            pl.set_response_status(tlm::TLM_OK_RESPONSE);
            
            offset += sc_core::sc_time(DELAY, sc_core::SC_NS);
            break;
            
        default:
            pl.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
            offset += sc_core::sc_time(DELAY, sc_core::SC_NS);
    }
}
