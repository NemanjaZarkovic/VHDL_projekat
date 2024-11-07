#include "dma.hpp"

DMA::DMA(sc_module_name name) : sc_module(name), dma_status(1), dma_result(0), dma_start(false)
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

    switch (cmd)
    {
        case TLM_WRITE_COMMAND:
            // Check if this is the DMA start command
            if (adr == ADDR_DMA_START) {
                dma_start = true;
                dma_status = 1;  // Set status to busy
                sig_send = sc_dt::SC_LOGIC_1;  // Trigger sending to FIFO
            }
            break;

        case TLM_READ_COMMAND:
            
            if (adr == ADDR_DMA_STATUS) {
                
                buf[0] = dma_status ? 1 : 0;
            } else if (adr == ADDR_DMA_RESULT) {
                *((int*)buf) = dma_result;  
            }
            break;

        default:
            pl.set_response_status(TLM_COMMAND_ERROR_RESPONSE);
    }

    pl.set_response_status(TLM_OK_RESPONSE);
    offset += sc_time(10, SC_NS);
}


void DMA::send_to_fifo()
{
    while(1)
    {
        while (sig_send == sc_dt::SC_LOGIC_0)
        {
            wait(sc_time(10, SC_NS));  
        }

     
        sig_send = sc_dt::SC_LOGIC_0;

        // Send data to FIFO1 (send to IP)
        for (unsigned int i = 0; i < length; i++)
        {
            while (!p_fifo_out->nb_write(pom_mem[i]))  // Non-blocking write to FIFO
            {
                wait(sc_time(10, SC_NS));  
            }
        }

        // Set status to done once data transfer is complete
        dma_status = 0;
        sig_read = sc_dt::SC_LOGIC_1;  // Start reading from IP (FIFO2)
    }
}

void DMA::read_from_fifo()
{
    while(1)
    {
        // Wait until sig_read is triggered
        while (sig_read == sc_dt::SC_LOGIC_0)
        {
            wait(sc_time(10, SC_NS));  // Wait 10 ns
        }

        // Reset the signal
        sig_read = sc_dt::SC_LOGIC_0;

        // Receive data from FIFO2 (IP to DMA)
        pom_mem.clear();
        for (unsigned int i = 0; i < length; i++)
        {
            sc_dt::sc_uint<16> fifo_read;
            while (!p_fifo_in->nb_read(fifo_read))  // Non-blocking read from FIFO
            {
                wait(sc_time(10, SC_NS));  // Retry every 10 ns
            }
            pom_mem.push_back(fifo_read);
        }

        dma_result = pom_mem[0];  
    }
}
