#ifndef DMA_C 
#define DMA_C
#include "dma.hpp"

DMA::DMA(sc_module_name name) : sc_module(name)
{
    s_dma_t.register_b_transport(this, &DMA::b_transport);
    SC_THREAD(send_to_fifo);
    SC_THREAD(read_from_fifo);
    sig_send = sc_dt::SC_LOGIC_0;
    sig_read = sc_dt::SC_LOGIC_0;

    cout << "DMA constructed" << endl;
}

void DMA::b_transport(pl_t& pl, sc_time& offset)
{
    cmd = pl.get_command();
    adr = pl.get_address();
    length = pl.get_data_length();
    buf = pl.get_data_ptr();
    begin = adr - 0x81000000;

    switch (cmd)
    {
        case TLM_WRITE_COMMAND:
            // Podiži signal za slanje ka FIFO
            sig_send = sc_dt::SC_LOGIC_1;
            
            // Očisti memoriju pre nego što napuniš FIFO
            pom_mem.clear();

            // Učitaj podatke iz transakcije u lokalni bafer kao num_f
            for (unsigned int i = 0; i < length; i++) {
                // Pretpostavljam da imaš funkciju koja konvertuje buffer u num_f
                num_f val = toNum_f(buf + i * sizeof(num_f));
                pom_mem.push_back(val);
            }

            // Start sending to FIFO (pokreće send_to_fifo)
            sig_send = sc_dt::SC_LOGIC_1;
            wait(sc_time(20, SC_NS));  // Dodaj kašnjenje
            sig_send = sc_dt::SC_LOGIC_0;

            pl.set_response_status(TLM_OK_RESPONSE);
            offset += sc_time(20, SC_NS);  // Dodaj dodatno kašnjenje ako je potrebno
            break;

        case TLM_READ_COMMAND:
            // Podiži signal za čitanje iz FIFO bafera
            sig_read = sc_dt::SC_LOGIC_1;

            // Sačekaj dok se FIFO ne pročita i podaci budu spremni
            wait(sc_time(20, SC_NS));  // Dodaj kašnjenje
            sig_read = sc_dt::SC_LOGIC_0;

            // Prenesi podatke iz FIFO u izlazni bafer kao num_f
            for (unsigned int i = 0; i < length; i++) {
                num_f val = pom_mem[i];
                doubleToUchar(buf + i * sizeof(num_f), val);
            }

            pl.set_response_status(TLM_OK_RESPONSE);
            offset += sc_time(20, SC_NS);  // Dodaj dodatno kašnjenje
            break;

        default:
            pl.set_response_status(TLM_COMMAND_ERROR_RESPONSE);
    }
}

void DMA::send_to_fifo()
{
    while(1) {
        // Čekaj dok signal za slanje ne postane aktivan
        while(sig_send == sc_dt::SC_LOGIC_0) {
            wait(sc_time(10, SC_NS));
        }

        // Kada je signal aktivan, šalji podatke iz pom_mem u FIFO
        for(unsigned int i = 0; i < length; i++) {
            while(!p_fifo_out->nb_write(pom_mem[i])) {
                wait(sc_time(10, SC_NS));  // Čekaj dok FIFO ne bude spreman
            }
        }

        // Nakon slanja svih podataka, postavi signal za slanje na nulu
        sig_send = sc_dt::SC_LOGIC_0;
    }
}

void DMA::read_from_fifo()
{
    num_f fifo_read;  // Koristimo num_f umesto sc_uint<16>
    while(1) {
        // Čekaj dok signal za čitanje ne postane aktivan
        while(sig_read == sc_dt::SC_LOGIC_0) {
            wait(sc_time(10, SC_NS));
        }

        pom_mem.clear();  // Očisti bafer pre čitanja novih podataka

        // Čitaj podatke iz FIFO i upisuj ih u pom_mem
        for (unsigned int i = 0; i < length; i++) {
            while(!p_fifo_in->nb_read(fifo_read)) {
                wait(sc_time(10, SC_NS));  // Čekaj dok FIFO ne bude spreman
            }
            pom_mem.push_back(fifo_read);  // Dodaj podatak u lokalni bafer
        }

        // Nakon što su svi podaci pročitajni, postavi signal za čitanje na nulu
        sig_read = sc_dt::SC_LOGIC_0;
    }
}

#endif // DMA_C

