#include <systemc.h>
#include <iostream>

// Constants
const int ARRAY_SIZE = 100;
const int FIFO_SIZE = 33;

// DMA Module
SC_MODULE(DMA) {
    sc_in<bool> clk;
    sc_fifo_out<int> fifo_out; // Send data to IP
    sc_fifo_in<int> fifo_in;   // Receive result from IP
    int data[ARRAY_SIZE];
    
    SC_CTOR(DMA) {
        SC_THREAD(transfer);
        sensitive << clk.pos();
    }
    
    void transfer() {
    // Initialize the data
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data[i] = i;
    }

    // Send numbers to IP block
    for (int i = 0; i < ARRAY_SIZE - 1; i += 2) {
        fifo_out.write(data[i]);
        fifo_out.write(data[i + 1]);
				}    

    // Receive and print results
    for (int i = 0; i < (ARRAY_SIZE / 2); ++i) {
        int result = fifo_in.read();
        std::cout << "Result " << i << ": " << result << std::endl;    
     }
				
    sc_stop();  
    }
};


// IP Block Module
SC_MODULE(IP_Block) {
    sc_in<bool> clk;
    sc_fifo_in<int> fifo_in;
    sc_fifo_out<int> fifo_out;
    
    SC_CTOR(IP_Block) {
        SC_THREAD(process);
        sensitive << clk.pos();
    }

    void process() {
        while (true) {
            int a = fifo_in.read();
            int b = fifo_in.read();
            int result = a + b;
            fifo_out.write(result);
        }
    }
};

// Top Module
SC_MODULE(Top) {
    sc_clock clk;
    DMA dma;
    IP_Block ip_block;
    sc_fifo<int> fifo1;
    sc_fifo<int> fifo2;

    SC_CTOR(Top) 
    : clk("clk", 1, SC_NS), dma("dma"), ip_block("ip_block"), fifo1(FIFO_SIZE), fifo2(FIFO_SIZE) 
    {
        // Connect modules
        dma.clk(clk);
        dma.fifo_out(fifo1);
        dma.fifo_in(fifo2);
        
        ip_block.clk(clk);
        ip_block.fifo_in(fifo1);
        ip_block.fifo_out(fifo2);
    }
};

int sc_main(int argc, char* argv[]) {
    Top top("top");
    sc_start();
    return 0;
}

