#include "dma.hpp"

DMA::DMA(sc_module_name name) : sc_module(name)
{
    // Register the TLM target socket b_transport function
    dma_rec.register_b_transport(this, &DMA::b_transport);

    // Launch threads for FIFO data transfer
    SC_THREAD(send_to_fifo);
    SC_THREAD(read_from_fifo);

    // Initialize signal logic
    sig_send = sc_dt::SC_LOGIC_0;
    sig_read = sc_dt::SC_LOGIC_0;

    cout << "DMA constructed" << endl;
}

void DMA::b_transport(pl_t& pl, sc_time& offset)
{
    // Extract TLM command, address, and data length
    cmd = pl.get_command();
    adr = pl.get_address();
    length = pl.get_data_length();
    buf = pl.get_data_ptr();
    begin = adr - 0x81000000;  // Adjust address based on base address

    // Handle TLM write/read commands
    switch (cmd)
    {
        case TLM_WRITE_COMMAND:
            // Trigger reading data from memory to send via FIFO
            pl.set_command(TLM_READ_COMMAND);
            pl.set_address(begin);
            dma_send->b_transport(pl, offset);  // Request data from memory
            assert(pl.get_response_status() == TLM_OK_RESPONSE);

            // Load the data from memory into local buffer
            buf = pl.get_data_ptr();
            pom_mem.clear();
            for (unsigned int i = 0; i < length; i++)
            {
                pom_mem.push_back(((sc_dt::sc_uint<16>*)buf)[i]);
            }

            // Start sending the data to the IP block via FIFO
            sig_send = sc_dt::SC_LOGIC_1;

            // Set response status and time delay
            pl.set_response_status(TLM_OK_RESPONSE);
            offset += sc_time(20, SC_NS);
            break;

        case TLM_READ_COMMAND:
            // Trigger reading data from the IP block FIFO
            sig_read = sc_dt::SC_LOGIC_1;

            // Set time delay
            offset += sc_time(20, SC_NS);
            break;

        default:
            pl.set_response_status(TLM_COMMAND_ERROR_RESPONSE);
    }
}

void DMA::send_to_fifo()
{
    sc_time offset = SC_ZERO_TIME;

    #ifdef QUANTUM
    tlm_utils::tlm_quantumkeeper qk;
    qk.reset();
    #endif

    while(1)
    {
        // Wait until sig_send is activated
        while (sig_send == sc_dt::SC_LOGIC_0)
        {
            #ifdef QUANTUM
            qk.inc(sc_time(10, SC_NS));
            offset = qk.get_local_time();
            qk.set_and_sync(offset);
            #else
            offset += sc_time(10, SC_NS);
            #endif
        }

        // Reset the send signal and write to the FIFO
        sig_send = sc_dt::SC_LOGIC_0;
        for (unsigned int i = 0; i < length; i++)
        {
            while (!p_fifo_out->nb_write(pom_mem[i]))  // Write to FIFO until successful
            {
                #ifdef QUANTUM
                qk.inc(sc_time(10, SC_NS));
                offset = qk.get_local_time();
                qk.set_and_sync(offset);
                #else
                offset += sc_time(10, SC_NS);
                #endif
            }
        }
    }
}

void DMA::read_from_fifo()
{
    pl_t pl;
    sc_time offset = SC_ZERO_TIME;
    sc_dt::sc_uint<16> fifo_read;

    #ifdef QUANTUM
    tlm_utils::tlm_quantumkeeper qk;
    qk.reset();
    #endif

    while (1)
    {
        // Wait until sig_read is activated
        while (sig_read == sc_dt::SC_LOGIC_0)
        {
            #ifdef QUANTUM
            qk.inc(sc_time(10, SC_NS));
            offset = qk.get_local_time();
            qk.set_and_sync(offset);
            #else
            offset += sc_time(10, SC_NS);
            #endif
        }

        // Reset the read signal and read from the FIFO
        pom_mem.clear();
        sig_read = sc_dt::SC_LOGIC_0;

        for (unsigned int i = 0; i < length; i++)
        {
            while (!p_fifo_in->nb_read(fifo_read))  // Read from FIFO until successful
            {
                #ifdef QUANTUM
                qk.inc(sc_time(10, SC_NS));
                offset = qk.get_local_time();
                qk.set_and_sync(offset);
                #else
                offset += sc_time(10, SC_NS);
                #endif
            }
            pom_mem.push_back(fifo_read);
        }

        // Write the received data back to memory via the TLM socket
        buf = (unsigned char*)&pom_mem[0];
        pl.set_address(begin);
        pl.set_data_length(length);
        pl.set_command(TLM_WRITE_COMMAND);
        pl.set_data_ptr(buf);
        dma_send->b_transport(pl, offset);
         #ifdef QUANTUM
        qk.set_and_sync(offset);  // Synchronize after TLM transaction
        #endif
    }
}

